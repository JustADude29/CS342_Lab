#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string>
#include <iostream>

static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char* base64_encode(unsigned char *src, size_t len) {
    unsigned char *out, *pos;
    const unsigned char *end, *in;

    size_t olen;

    olen = 4*((len + 2) / 3);

    char* outStr=new char[0];

    if (olen < len)
        return NULL;

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

#define MSG_LEN 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <Server_IP_Address> <Server_Port_Number>\n", argv[0]);
        return 1;
    }

    const char *serverIP = argv[1];
    int serverPort = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);

    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connect");
        return 1;
    }

    printf("Connected to server\n");
    char buffer[MSG_LEN];
    int bytesRead = recv(sock, buffer, sizeof(buffer), 0);
    std::cout<<buffer<<std::endl;
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("Server Acknowledgment: %s\n", buffer);
    }

    bool first=true;
    while (1) {
        char message[MSG_LEN];
        printf("Enter an expression: ");
        fgets(message, MSG_LEN, stdin);

        if(strcmp(message, "/exit\n")==0){
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
    printf("Connection closed\n");

    return 0;
}
