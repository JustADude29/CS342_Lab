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

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(unsigned char *src, size_t len)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olen;

    olen = 4*((len + 2) / 3); /* 3-byte blocks to 4-byte */

    char* outStr=new char[0];

    if (olen < len)
        return NULL; /* integer overflow */

    outStr = new char[olen];
    out = (unsigned char*)&outStr[0];

    end = src + len;
    in = src;
    pos = out;
    while (end - in >= 3) {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
    }

    if (end - in) {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1) {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
    }

    return outStr;
}

void errorPrinter(std::string s){
	std::cerr<<COLOR_RED<<s<<COLOR_RESET<<std::endl;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        errorPrinter("Usage: ./client <ip> <PORT>");
        return 1;
    }

    uint MSG_LEN = 1024;
    const char *serverIP = argv[1];
    int serverPort = atoi(argv[2]);

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
    std::cout<<buffer<<std::endl;
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        std::cout<<COLOR_YELLOW<<"Acknowledgement recieved: "<<buffer<<COLOR_RESET<<std::endl;
    }
    std::cout<<COLOR_BLUE<<"Connected to server"<<COLOR_RESET<<std::endl;

    while (1) {
        char message[MSG_LEN];
        std::cout<<COLOR_YELLOW<<"Enter a message, format:<msg_type> <msg>: "<<COLOR_RESET<<std::endl;
        fgets(message, MSG_LEN, stdin);

        char *formattedMessage = base64_encode((unsigned char *)message, strlen(message));

        send(sock, formattedMessage, strlen(formattedMessage), 0);
        if(message[0]=='3') break;

        char buffer[MSG_LEN];
        int siz = recv(sock, buffer, MSG_LEN, 0);
        if(siz>0){
            std::cout<<COLOR_BLUE<<"Acknowledgement recieved: "<<buffer<<COLOR_RESET<<std::endl;
        }
    }

    close(sock);
    std::cout<<"Connection closed"<<std::endl;

    return 0;
}
