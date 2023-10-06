#include <iostream>
#include <set>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

const int MAX_CLIENTS = 10;
const int PORT = 8080;

std::set<int> clients;
std::mutex clientMutex;
std::condition_variable cv;

void broadcastMessage(const std::string& message, int sender) {
    std::lock_guard<std::mutex> lock(clientMutex);
    for (auto client : clients) {
        if (client != sender) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

void* handleClient(void* arg) {
    int clientSocket = *((int*)arg);
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            std::lock_guard<std::mutex> lock(clientMutex);
            clients.erase(clientSocket);
            cv.notify_all();
            close(clientSocket);
            return nullptr;
        }
        std::string message(buffer);
        broadcastMessage("Client #" + std::to_string(clientSocket) + ": " + message, clientSocket);
    }
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrSize = sizeof(clientAddr);
    pthread_t tid;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error binding" << std::endl;
        return -1;
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        std::cerr << "Error listening" << std::endl;
        return -1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
        if (clientSocket == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }

        std::lock_guard<std::mutex> lock(clientMutex);
        if (clients.size() >= MAX_CLIENTS) {
            std::cout << "Maximum clients reached. Connection rejected." << std::endl;
            close(clientSocket);
            continue;
        }

        clients.insert(clientSocket);
        pthread_create(&tid, nullptr, handleClient, &clientSocket);
        pthread_detach(tid);
    }

    close(serverSocket);

    return 0;
}