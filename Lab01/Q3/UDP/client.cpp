#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

void errorPrinter(std::string s){
	std::cerr<<s<<std::endl;
}

int main(int argc, char* argv[]){
    if(argc!=3){
        errorPrinter("Usage: ./client <ip> <PORT>");
        return -1;
    }

    const char *serverIP = argv[1];
    int serverPort = atoi(argv[2]);
    const uint MSG_LEN = 1024;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock<0){
        errorPrinter("socket error");
        return -1;
    }

    struct sockaddr_in serverAddr;
    socklen_t addrlen;
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    char buffer[MSG_LEN];
    while(1){
        std::cout<<"Type an expression(-1 to exit): ";
        fgets(buffer, MSG_LEN, stdin);

        if(strcmp(buffer, "-1\n")==0){
            break;
        }

        sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

        addrlen = sizeof(serverAddr);
        int n = recvfrom(sock, buffer, MSG_LEN, 0, (struct sockaddr*)&serverAddr, &addrlen);
        if(n>0){
            buffer[n]='\0';
            std::cout<<"Result: "<<buffer<<std::endl;
        }
    }

    close(sock);
    return 0;
}