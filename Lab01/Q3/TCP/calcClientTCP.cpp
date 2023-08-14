#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_RESET "\033[0m"

void errorPrinter(std::string s){
	std::cerr<<COLOR_RED<<s<<COLOR_RESET<<std::endl;
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

    char buffer[MSG_LEN];
    int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout<<COLOR_YELLOW<<"Server acknowledgement: "<<buffer<<COLOR_RESET<<std::endl;
    }
    std::cout<<COLOR_BLUE<<"Connected to server"<<COLOR_RESET<<std::endl;

    bool first=true;
    while (1) {
        char message[MSG_LEN];
        std::cout<<COLOR_YELLOW<<"Enter an expression(usage: <num1> <op> <num2>, -1 to exit): "<<COLOR_GREEN;
        fgets(message, MSG_LEN, stdin);
        std::cout<<COLOR_RESET<<std::endl;

        if(strcmp(message, "-1\n")==0){
            break;
        }

        send(sock, message, strlen(message), 0);
        
        std::cout<<COLOR_BLUE<<"Result: ";
        char buff[MSG_LEN];
        int n = recv(sock, buff, MSG_LEN, 0);
        if(n>0){
            buff[n]='\0';
            std::cout<<COLOR_GREEN<<buff<<COLOR_RESET<<std::endl;
            std::cout<<std::endl;
        }
    }

    close(sock);
    std::cout<<COLOR_RED<<"Connection closed"<<COLOR_RESET<<std::endl;

    return 0;
}
