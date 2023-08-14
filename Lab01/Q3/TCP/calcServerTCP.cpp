#include <cmath>
#include <ostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>

#define COLOR_RED "\033[1;31m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_RESET "\033[0m"

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

void errorPrinter(std::string s){
	std::cerr<<COLOR_RED<<s<<COLOR_RESET<<std::endl;
}

int main(int argc , char *argv[])
{
	if(argc!=2){
		errorPrinter("Usage: ./server <PORT>");
		return -1;
	}
    uint PORT = std::stoi(argv[1]);
    uint MSG_LEN=1024;
    
    int max_clients = 30;
	
	int master_socket , addrlen , new_socket , client_socket[max_clients] ,
		activity, i , valread , sd;
	int max_sd;
	struct sockaddr_in address;
		
	char buffer[MSG_LEN];

	fd_set readfds;

	char *message = "Welcome to socketCHATter 3\n";

	for (i = 0; i < max_clients; i++){
		client_socket[i] = 0;
	}

	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) < 0){
		errorPrinter("socket error");
		return -1;
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){
		errorPrinter("bind error");
		return -1;
	}
	std::cout<<COLOR_YELLOW"Server on Port: "<<PORT<<COLOR_RESET<<std::endl;
		
	if (listen(master_socket, 20) < 0){
		errorPrinter("listen error");
		return -1;
	}

	addrlen = sizeof(address);
	std::cout<<COLOR_BLUE<<"Waiting for connections"<<COLOR_RESET<<std::endl;
		
	bool first = true;
	while(1){
		FD_ZERO(&readfds);

		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		
		for ( i = 0 ; i < max_clients ; i++){
			sd = client_socket[i];

			if(sd > 0)
				FD_SET( sd , &readfds);

			if(sd > max_sd)
				max_sd = sd;
		}

		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
	
		if ((activity < 0) && (errno!=EINTR)){
			errorPrinter("Select error");
		}

		if (FD_ISSET(master_socket, &readfds)){
			if((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
				errorPrinter("accept error");
				exit(EXIT_FAILURE);
			}
			
            std::cout<<COLOR_YELLOW<<"New connection: "<<new_socket<<" on ip: "<<inet_ntoa(address.sin_addr)<<" port: "<<ntohs(address.sin_port)<<COLOR_RESET<<std::endl;
		
			if(send(new_socket, message, strlen(message), 0) != strlen(message) ){
				errorPrinter("send error");
			}

			for(i = 0; i < max_clients; i++){
				if( client_socket[i] == 0 ){
					client_socket[i] = new_socket;
					std::cout<<COLOR_BLUE<<"Adding socket to list at index: "<<i<<COLOR_RESET<<std::endl;
					break;
				}
			}
		}

		for (i = 0; i < max_clients; i++){
			sd = client_socket[i];

			if (FD_ISSET( sd , &readfds)){
				if ((valread = read( sd , buffer, 1024)) == 0){
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    std::cout<<COLOR_RED<<"Host disconnected: "<<i<<" on ip: "<<inet_ntoa(address.sin_addr)<<" port: "<<ntohs(address.sin_port)<<COLOR_RESET<<std::endl;
					close( sd );
					client_socket[i] = 0;
				}

				else{
                    buffer[valread] = '\0';
					std::cout<<COLOR_BLUE<<"Recieved expression: "<<COLOR_YELLOW<<buffer<<COLOR_BLUE"from client: "<<COLOR_YELLOW<<i<<COLOR_RESET<<std::endl;
					std::string exp = buffer;
					if(isValidExpression(exp)){
						char* res = resulter(buffer);
						send(sd, res, strlen(res), 0);
					}else{
						send(sd, "Invalid expression", 19, 0);
					}
				}
			}

		}
	}
		
	return 0;
}
