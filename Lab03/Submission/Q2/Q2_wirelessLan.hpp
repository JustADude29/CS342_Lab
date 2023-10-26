#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <signal.h>
#include <cstring>

using namespace std;

static int propogation_time=1, backoff_max=5,cnt=0;
static mutex outputMutex;
static mutex value_mutex;
static int num_of_collisions=0;
static int successful_transmissions=0;
static int num_of_backoffs=0;
static int num_of_collisions_detected=0;
static int num_of_fails=0;
static chrono::high_resolution_clock::time_point startTime;
static std::condition_variable cv;

static std::mutex cv_m;
static std::unique_lock<std::mutex> lk(cv_m);

static bool backoff_fail=false;
static int transmission_interval = 5;

class Channel {
public:
    bool busy = false;
    string transmitted_data = "";
    int owner = -1;
    bool collision = 0;
    int totalNodes;

public:
    bool is_busy() {
        return busy;
    }

    bool  transmit(int node_id, string data,double transmission_time) {
        if (busy) {
            std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
            std::chrono::milliseconds cnt = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            outputMutex.lock();
            cout << "Node " << node_id << " is waiting due to channel busy at " << (cnt.count())/1000 << " s "<< endl;
            num_of_collisions_detected++;
            outputMutex.unlock();
            return 0;
        }
        else{
            sleep(propogation_time);
            value_mutex.lock();
            totalNodes++;
            busy = true;
            value_mutex.unlock();
            sleep(transmission_time);
            
            value_mutex.lock();
            outputMutex.lock();

            std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
            std::chrono::milliseconds cnt = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            cout << "At " << (cnt.count())/1000 << " s "<< endl;
            cout << totalNodes << endl;
            outputMutex.unlock();
            value_mutex.unlock();


            if(totalNodes > 1){
                outputMutex.lock();
                cout << "Collision detected and resolved...\n";
                num_of_collisions++;
                outputMutex.unlock();
                value_mutex.lock();
                totalNodes--;
                outputMutex.lock();
                endTime = std::chrono::high_resolution_clock::now();
                cnt = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                cout << "At " << (cnt.count())/1000 << " s "<< endl;
                cout << totalNodes << endl;
                outputMutex.unlock();
                value_mutex.unlock();
                cv.notify_all();
                return 0;
            }

            
            transmitted_data = data;
            outputMutex.lock();
            endTime = std::chrono::high_resolution_clock::now();
            cnt = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

            cout << "Node " << node_id << " is propagating: " << data << " at "<< (cnt.count())/1000 << " s "<< endl;
            outputMutex.unlock();

            if(cv.wait_for(lk,std::chrono::seconds(propogation_time)) == std::cv_status::no_timeout ){
                // collision
                outputMutex.lock();
                cout << "Collision detected and resolved...\n";
                num_of_collisions++;
                outputMutex.unlock();
                value_mutex.lock();
                totalNodes--;
                outputMutex.lock();
                endTime = std::chrono::high_resolution_clock::now();
                cnt = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
                cout << "At " << (cnt.count())/1000 << " s "<< endl;
                cout << totalNodes << endl;
                outputMutex.unlock();
                value_mutex.unlock();
                busy = 0;
                return 0;
            }
            
            
            value_mutex.lock();
            successful_transmissions++;
            totalNodes--;
            outputMutex.lock();
            endTime = std::chrono::high_resolution_clock::now();
            cnt = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            cout << "At " << (cnt.count())/1000 << " s "<< endl;
            cout << totalNodes << endl;
            outputMutex.unlock();
            value_mutex.unlock();
            busy=0;
            return 1;
        }
    }

    void clear() {
        busy = false;
        owner = -1;
        transmitted_data = "";
    }

};


static Channel channel;


class Node {
public:
    int node_id;
    int backoff_max;
    int backoff_size;
    int col;
public:
    Node(int id, int backoff_max) : node_id(id), backoff_max(backoff_max), backoff_size(0) {}
    bool transmit_data(string data,double transmission_time) {
        while(1){
            if(channel.transmit(node_id, data,transmission_time)) {
                backoff_size = 0;
                cout << "Node " << node_id << " successfully transmited message: " << data << endl;
                return 1;
            }
            else {
                if(backoff_fail){
                    backoff_size++;
                    if(backoff_size > backoff_max) {
                        backoff_size = 0;
                        cout << "Node " << node_id << " failed to transmit message: " << data << endl;
                        num_of_fails++;
                        return 0;
                    }
                }
                num_of_backoffs++;
                perform_backoff();
            }
        }
    }

    void perform_backoff() {
        double backoff_time;
        if(backoff_fail)
            backoff_time = rand() % ((1<<backoff_size) - 1)+1;
        else
            backoff_time = rand() % (backoff_max*4 - 1)+1;
            
        outputMutex.lock();
        cout << "Node " << node_id << " is backing off for " << backoff_time << " seconds." << endl;
        outputMutex.unlock();
        sleep(backoff_time);
    }    
};

static void print_statistics(){
       outputMutex.lock();
        cout<<"\n.........Statistics........\n";
        cout<<"Number of successful transmissions = "<<successful_transmissions<<endl;
        cout<<"Number of Collisions Detected = "<<num_of_collisions<<endl;
        cout<<"Number of Backoffs =  "<<num_of_backoffs<<endl;
        cout<<"Number of Collisions Avoided = "<<num_of_collisions_detected<<endl;
        if(backoff_fail)
            cout<<"Number of failed transmissions = "<<num_of_fails<<endl;
        cout<<".............................\n";
        cout<<"Exiting....................\n";
        outputMutex.unlock();
}

static void sighandler(int sig){
    print_statistics();
      exit(0);
}



static void handleChannel(){
    while(1){
     int x;
     signal(SIGINT, &sighandler);
     cin>>x;
     if(x==-1){
       print_statistics();
        break;
     }
    }
}

static void handleNode(Node node,double trans_time){
   int count = 0;
   int num_msgs = 3;
   while(num_msgs--){
        count++;
        string data="Message from node: ";
        string num=to_string(node.node_id);
        data=data+num+": " + to_string(count);
        if(node.transmit_data(data,trans_time)) sleep(rand()%transmission_interval + 1);
   }
}