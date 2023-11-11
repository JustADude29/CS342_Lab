#include <cmath>
#include <chrono>
#include <thread>
#include <vector>
#include <climits>
#include <signal.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <cstring>

using namespace std;

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

union semun {
    int val;    /* used for SETVAL only */
    struct semid_ds *buf;   /* for IPC_STAT and IPC_SET */
    ushort *array;  /* used for GETALL and SETALL */
};

int msqid = -1;
char buffer[5000];
int NUM_SERVERS = 1;
int QUEUE_SIZE = -1;
double ARRIVAL_RATE = 1;
double SERVICE_RATE = 1;
double RATE = 1;
key_t sem_key;
union semun arg;
int semid = -1;
pthread_t averagesThread, arrivalsThread;
bool multipleQueues = false, printerBool = true;
int dropCount = 0;
vector<int> QueueLenthData, BusyServerData, Dummy;
vector<std::chrono::duration<double>> waitTimesData;

class ExpRandGenerator {
private:
    double lambda;
    int a, b, m;
    vector<double> randList;
public:
    ExpRandGenerator(double rate, int seed = 25): lambda(1/rate), a(1597), b(25), m(244944) {
        set_seed(seed);
    }
    void set_rate(double rate) {
        lambda = 1/rate;
    }
    void set_seed(int seed){
        randList.resize(17);
        for(int i = 0; i < 17; i++) {
            randList[i] = (double)seed / m;
            seed = (seed*a + b) % m;
        }
    } 
    double generate() {
        randList.push_back(randList[randList.size() - 17] - randList[randList.size() - 5]);
        if(randList.back() < 0) randList.back() += 1;
        return - log(randList.back()) * lambda;
    }  
};

void* handleAverages(void* args) {
    do{
        std::chrono::duration<double> duration(0.01 / RATE);
        std::this_thread::sleep_for(duration);
        if(multipleQueues) {
            for(int i = 0; i < NUM_SERVERS; i++) QueueLenthData.push_back(semctl(semid, 2*i + 1, GETVAL, NULL));
            BusyServerData.push_back(semctl(semid, 2*NUM_SERVERS, GETVAL, NULL));
        } else {
            QueueLenthData.push_back(semctl(semid, 1, GETVAL, NULL));
            BusyServerData.push_back(semctl(semid, 2, GETVAL, NULL));
        }
    } while(true);
    return NULL;
}

void sem_wait(int id, short unsigned int i = 0) {
    struct sembuf sb = {i, -1, 0};
    semop(id, &sb, 1);
    return;
}
void sem_signal(int id, short unsigned int i = 0) {
    struct sembuf sb = {i, 1, 0};
    semop(id, &sb, 1);
    return;
}

ExpRandGenerator rgen_arrivals(ARRIVAL_RATE);
ExpRandGenerator rgen_service(SERVICE_RATE, 69);

void* handleServer(void* args){
    double waitT = rgen_service.generate();
    std::chrono::duration<double> duration(waitT / RATE);
    std::this_thread::sleep_for(duration);
    return NULL;
}

void* handleQueue(void* args) {
    if(multipleQueues) {
        int index = *(int *)args;
        // cout << "val " << index << endl;
        sem_signal(semid, index*2 + 1);
        auto start = std::chrono::high_resolution_clock::now();
        sem_wait(semid, index*2);
        auto end = std::chrono::high_resolution_clock::now();
        sem_signal(semid, 2*NUM_SERVERS);
        pthread_t serverThread;
        pthread_create(&serverThread, NULL, handleServer, NULL);
        sem_wait(semid, index*2 + 1);
        pthread_join(serverThread, NULL);
        sem_signal(semid, index*2);
        sem_wait(semid, 2*NUM_SERVERS);
        sem_wait(semid, 2*NUM_SERVERS + 1);
        waitTimesData.push_back(end - start);
        sem_signal(semid, 2*NUM_SERVERS + 1);
    }
    else {
        sem_signal(semid, 1);
        auto start = std::chrono::high_resolution_clock::now();
        sem_wait(semid);
        auto end = std::chrono::high_resolution_clock::now();
        pthread_t serverThread;
        sem_signal(semid, 2);
        pthread_create(&serverThread, NULL, handleServer, NULL);
        sem_wait(semid, 1);
        pthread_join(serverThread, NULL);
        sem_wait(semid, 2);
        sem_signal(semid);
        sem_wait(semid, 3);
        waitTimesData.push_back(end - start);
        sem_signal(semid, 3);
    }
    return NULL;
}

void* handleArrivals(void* args) {
    while(true){
        double waitT = rgen_arrivals.generate();
        sprintf(buffer, "Next passenger arriving in %lfs", waitT);
        msgsnd(msqid, buffer, strlen(buffer), 0);
        // cout << "Waiting for " << waitT << "s\n";
        std::chrono::duration<double> duration(waitT / RATE);
        std::this_thread::sleep_for(duration);
        if(multipleQueues) {
            int m = INT_MAX, m_index = -1, x;
            for(int i = 0; i < NUM_SERVERS; i++) {
                x = semctl(semid, i*2 + 1, GETVAL, NULL);
                // cout << i << " " << x << endl;
                if(x < m) {
                    m = x;
                    m_index = i;
                }
            }
            // cout << "m: " << m << endl;
            if(QUEUE_SIZE != -1 && m >= QUEUE_SIZE) {
                // cout << "All Queues Full, rejecting passenger :(\n";
                sprintf(buffer, "All Queues full, rejecting passenger :(");
                msgsnd(msqid, buffer, strlen(buffer), 0);
                dropCount++;
                continue;
            } else {
                pthread_t queueThread;
                Dummy.push_back(m_index);
                pthread_create(&queueThread, NULL, handleQueue, &Dummy[Dummy.size() - 1]);
            }
        }
        else {
            if(QUEUE_SIZE != -1 && semctl(semid, 1, GETVAL, NULL) >= QUEUE_SIZE) {
                // cout << "Queue full, rejecting passenger :(\n";
                sprintf(buffer, "Queue full, rejecting passenger :(");
                msgsnd(msqid, buffer, strlen(buffer), 0);
                dropCount++;
                continue; 
            }
            pthread_t queueThread;
            pthread_create(&queueThread, NULL, handleQueue, NULL);
        }
    }
    return NULL;
}

void programTerminate() {
    // printerBool = false;
    pthread_cancel(averagesThread);
    pthread_cancel(arrivalsThread);
    
    double avgQLen = 0, avgUtil = 0, avgWaitTime = 0;
    for(int i = 0; i < QueueLenthData.size(); i++) {
        avgQLen += QueueLenthData[i];
    }
    avgQLen /= QueueLenthData.size();

    for(int i = 0; i < BusyServerData.size(); i++) {
        avgUtil += BusyServerData[i];
    }
    avgUtil /= (BusyServerData.size() * NUM_SERVERS);

    for(int i = 0; i < waitTimesData.size(); i++) {
        avgWaitTime += waitTimesData[i].count();
    }
    avgWaitTime = avgWaitTime * RATE / waitTimesData.size();

    cout << GREEN <<"Avg. Q Length = " << BLUE << avgQLen << GREEN << ", Avg. Utilisation = " << BLUE << avgUtil << GREEN << ", Avg. Wait Time = " << BLUE << avgWaitTime << GREEN << ", Drop Count = " << BLUE <<dropCount << RESET << endl;
    if (semctl(semid, 0, IPC_RMID, arg) == -1) {
        perror("semctl - IPC_RMID");
        exit(1);
    }
    return;
}

void signalHandler(int signum) {
    cout << "\nTerminating...\n";
    programTerminate();
    exit(signum);
}

int main(int argc, char* argv[]) {
    key_t key;
    if ((key = ftok("logger.cpp", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }
    
    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    double simDuration;
    cout << YELLOW <<"Simulation Duration: " << RESET;
    cin >> simDuration;
    cout << YELLOW << "Enter Rate of simulation: " << RESET;
    cin >> RATE;
    cout << YELLOW << "Enter the Queue size(-1 for infinite buffer): " << RESET;
    cin >> QUEUE_SIZE;
    cout << YELLOW << "Enter the number of servers: " << RESET;
    cin >> NUM_SERVERS;
    if(NUM_SERVERS > 1) {
        char inp;
        cout << YELLOW << "Have Multiple Queues? (Y/n) " << RESET;
        cin >> inp;
        if(inp == 'Y' || inp == 'y') multipleQueues = true;
    }
    cout << YELLOW << "Enter arrival rate: " << RESET;
    cin >> ARRIVAL_RATE;
    cout << YELLOW << "Enter service rate: " << RESET;
    cin >> SERVICE_RATE;
    rgen_arrivals.set_rate(ARRIVAL_RATE);
    rgen_service.set_rate(SERVICE_RATE);
    
    if ((sem_key = ftok("Q2.cpp", 'S')) == -1) {
        perror("ftok");
        exit(1);
    }
    if(multipleQueues) {
        if ((semid = semget(sem_key, 2*NUM_SERVERS + 2, 0666 | IPC_CREAT)) == -1) {
            perror("semget");
            exit(1);
        }
        arg.array = new ushort(2*NUM_SERVERS + 2);
        for(int i = 0; i < NUM_SERVERS; i++) {
            arg.array[2 * i] = 1;
            arg.array[2 * i + 1] = 0;
        }
        arg.array[2 * NUM_SERVERS] = 0;
        arg.array[2 * NUM_SERVERS + 1] = 1;
    }
    else {
        if ((semid = semget(sem_key, 4, 0666 | IPC_CREAT)) == -1) {
            perror("semget");
            exit(1);
        }
        arg.array = new ushort(4);
        arg.array[0] = NUM_SERVERS;
        arg.array[1] = arg.array[2] = 0; 
        arg.array[3] = 1;
    }
    if (semctl(semid, 0, SETALL, arg) == -1) {
        perror("semctl - SETALL");
        exit(1);
    }

    cout << BLUE <<"\nEntering Threading\n" << RESET;
    vector<int> queue_ind(NUM_SERVERS);
    for(int i = 0; i < NUM_SERVERS; i++) queue_ind[i] = i;
    pthread_create(&averagesThread, NULL, handleAverages, NULL);
    pthread_create(&arrivalsThread, NULL, handleArrivals, NULL);
    // pthread_t printerThread;
    // pthread_create(&printerThread, NULL, printer, NULL);
    signal(SIGINT, signalHandler);
    cout << BLUE <<"Main going to sleep...\n\n" << RESET;
    std::chrono::duration<double> duration(simDuration / RATE);
    std::this_thread::sleep_for(duration);
    programTerminate();
    return 0;
}