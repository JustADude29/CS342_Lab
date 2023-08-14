#include <cmath>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>

void errorPrinter(std::string s){
	std::cerr<<s<<std::endl;
}

char* resulter(char* s){
    std::string str = s;

    std::string s1, s2;
    char op;
    bool neg1=false, neg2=false;
    int i=0;
    if(str[0]=='-') {neg1=true;i++;}

    bool first=true;
    while(i<str.size()){
        if((str[i]=='*' || str[i]=='/' || str[i]=='+' || str[i]=='-' || str[i]=='^') && first){
            op = str[i];
            i++;
            first=false;
            continue;
        }
        if(first){
            s1.push_back(str[i]);
            i++; continue;
        }else{
            if(str[i]=='-') {neg2=true; i++; continue;}
            s2.push_back(str[i]);
            i++;
        }
    }
    float i1 = std::stof(s1);
    float i2 = std::stof(s2);
    if(neg1) i1=i1*-1;
    if(neg2) i2=i2*-1;
    float res=0;
    if(op=='*') res = i1*i2;
    if(op=='/') res = i1/i2;
    if(op=='+') res = i1+i2;
    if(op=='-') res = i1-i2;
    if(op=='^') res = std::pow(i1,i2);

    std::string result = std::to_string(res);
    char* r = new char(result.size());
    for(int i=0; i<result.size(); i++) r[i]=result[i];
    return r;
}

bool isValidExpression(std::string &str) {
    std::string s1, s2;
    char op='&';
    bool neg1=false, neg2=false;
    int i=0;
    while(str[i]==' ') i++;
    if(str[i]=='-'){
        neg1=true;
        i++;
    }

    bool first=true;
    while(i<str.size()){
        if((str[i]=='*' || str[i]=='/' || str[i]=='+' || str[i]=='-' || str[i]=='^') && first){
            op = str[i];
            i++;
            first=false;
            continue;
        }
        if(first){
            s1.push_back(str[i]);
            i++; continue;
        }else{
            if(str[i]=='-') {neg2=true; i++; continue;}
            s2.push_back(str[i]);
            i++;
        }
    }
    if(op=='&') return false;
    return true;
}

int main(int argc, char* argv[]){
    if(argc!=2){
        errorPrinter("Usage: ./server <PORT>");
        return -1;
    }

    uint PORT = std::stoi(argv[1]);
    uint MSG_LEN = 1024;

    int master_socket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrlen;

    char buffer[MSG_LEN];

    if((master_socket = socket(AF_INET, SOCK_DGRAM, 0))<0){
        std::cerr<<"socket failed"<<std::endl;
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(master_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr))<0){
        errorPrinter("bind error");
        exit(EXIT_FAILURE);
    }

    std::cout<<"Server on PORT: "<<PORT<<std::endl;

    addrlen = sizeof(clientAddr);

    while(1){
        int bytes = recvfrom(master_socket, buffer, MSG_LEN, 0, (struct sockaddr*)&clientAddr, &addrlen);
        if(bytes==-1){
            errorPrinter("receive error");
            continue;
        }

        buffer[bytes]='\0';
        std::cout<<"Received expression: "<<buffer<<std::endl;
        std::string exp = buffer;
        if(isValidExpression(exp)){
            char *res = resulter(buffer);
            sendto(master_socket, res, strlen(res), 0, (struct sockaddr*)&clientAddr, addrlen);
        }else{
            sendto(master_socket, "Invalid expression", 18, 0, (struct sockaddr*)&clientAddr, addrlen);
        }
    }

    close(master_socket);

    return 0;
}
