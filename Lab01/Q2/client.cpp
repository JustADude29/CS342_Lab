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

#define LOG(x) std::cout<<x<<std::endl;

#define MSG_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr<<"Usage: ./client <ip> <PORT>"<<std::endl;
        return -1;
    }

    const char *serverIP = argv[1];
    int serverPort = atoi(argv[2]);
    fd_set readfds;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr<<"socket error"<<std::endl;
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr<<"Connect error"<<std::endl;
        return 1;
    }

    printf("Connected to server\n");
    char buffer[MSG_LEN];
    int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
    std::cout<<buffer<<std::endl;
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout<<"Server acknowledgement: "<<buffer<<std::endl;
    }

    bool first=true;
    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        int act = select(sock+1, &readfds, NULL, NULL, NULL);

        if(act<0){
            std::cerr<<"select error"<<std::endl;
        }

        if(FD_ISSET(STDIN_FILENO, &readfds)){
            int siz = read(STDIN_FILENO,  buffer, MSG_LEN);
            if(strcmp(buffer, "/exit\n")==0) break;
            buffer[siz]='\0';
            send(sock, buffer, strlen(buffer), 0);
            LOG("sent");
        }

        else {
            int siz = read(sock, buffer, MSG_LEN);
            buffer[siz]='\0';
            if(siz>0){
                LOG(buffer);
            }else break;
        }
    }

    close(sock);
    std::cout<<"connection closed"<<std::endl;

    return 0;
}
