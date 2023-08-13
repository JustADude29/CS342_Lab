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

int main(int argc, char *argv[]) {
    if (argc != 3) {
        errorPrinter("Usage: ./client <ip> <PORT>");
        return 1;
    }

    const char *serverIP = argv[1];
    int serverPort = atoi(argv[2]);
    const uint MSG_LEN = 1024;

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        errorPrinter("socket error");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        errorPrinter("connect error");
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
        char message[MSG_LEN];
        printf("Enter an expression(-1 to exit): ");
        fgets(message, MSG_LEN, stdin);

        if(strcmp(message, "-1\n")==0){
            break;
        }

        send(sock, message, strlen(message), 0);
        
        std::cout<<"Result: ";
        char buff[MSG_LEN];
        int n = recv(sock, buff, MSG_LEN, 0);
        if(n>0){
            buff[n]='\0';
            std::cout<<buff<<std::endl;
        }
    }

    close(sock);
    std::cout<<"Connection closed"<<std::endl;

    return 0;
}
