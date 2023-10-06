#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

const int PORT = 8080;
const char* SERVER_IP = "127.0.0.1";

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return -1;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error connecting to server" << std::endl;
        return -1;
    }

    std::cout << "Connected to server at " << SERVER_IP << ":" << PORT << std::endl;

    char message[1024];
    while (true) {
        std::cout << "Enter a message (or type '/exit' to quit): ";
        std::cin.getline(message, sizeof(message));
        if (std::string(message) == "/exit") {
            break;
        }
        send(clientSocket, message, strlen(message), 0);
    }

    close(clientSocket);

    return 0;
}
