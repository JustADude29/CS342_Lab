#include <cstddef>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

#define LOG(x) std::cout << x << std::endl;
#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_RESET "\033[0m"

#define MSG_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << COLOR_RED << "Usage: ./client <ip> <PORT>" << COLOR_RESET << std::endl;
        return -1;
    }

    const char *serverIP = argv[1];
    int serverPort = atoi(argv[2]);
    fd_set readfds;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << COLOR_RED << "socket error" << COLOR_RESET << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << COLOR_RED << "Connect error" << COLOR_RESET << std::endl;
        return 1;
    }

    std::cout << COLOR_GREEN << "Connected to server" << COLOR_RESET << std::endl;
    char buffer[MSG_LEN];
    int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
    std::cout << COLOR_BLUE << buffer << COLOR_RESET << std::endl;
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout << COLOR_YELLOW << "Server acknowledgement: " << buffer << COLOR_RESET << std::endl;
    }

    bool first = true;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int act = select(sock + 1, &readfds, NULL, NULL, NULL);

        if (act < 0) {
            std::cerr << COLOR_RED << "select error" << COLOR_RESET << std::endl;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            int siz = read(STDIN_FILENO, buffer, MSG_LEN);
            if (strcmp(buffer, "/exit\n") == 0) break;
            buffer[siz] = '\0';
            send(sock, buffer, strlen(buffer), 0);
            LOG(COLOR_GREEN + std::string("Sent message") + COLOR_RESET);
            LOG("");
        } else {
            int siz = read(sock, buffer, MSG_LEN);
            buffer[siz] = '\0';
            std::string s;
            for(int j=2; j<strlen(buffer); j++) s.push_back(buffer[j]);
            if (siz > 0) {
                LOG(COLOR_GREEN + s + COLOR_RESET);
            } else break;
        }
    }

    close(sock);
    std::cout << COLOR_YELLOW << "Connection closed" << COLOR_RESET << std::endl;

    return 0;
}
