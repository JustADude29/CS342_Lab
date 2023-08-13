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
    int i1 = std::stoi(s1);
    int i2 = std::stoi(s2);
    if(neg1) i1=i1*-1;
    if(neg2) i2=i2*-1;
    int res=0;
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
	
int main(int argc , char *argv[])
{
    uint PORT = std::stoi(argv[2]);
    uint MSG_LEN=1024;
    
    int max_clients = 30;
	
    int opt=1;
	int master_socket , addrlen , new_socket , client_socket[max_clients] ,
		activity, i , valread , sd;
	int max_sd;
	struct sockaddr_in address;
		
	char buffer[MSG_LEN];

	fd_set readfds;

	char *message = "Welcome to socketCHATter \r\n";

	for (i = 0; i < max_clients; i++){
		client_socket[i] = 0;
	}

	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) < 0){
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 ){
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(argv[1]);
    std::cout<<"ip: "<<address.sin_addr.s_addr<<std::endl;
	address.sin_port = htons( PORT );

	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	std::cout<<"Server on Port: "<<PORT<<std::endl;
		
	if (listen(master_socket, 20) < 0){
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	addrlen = sizeof(address);
	std::cout<<"Waiting for connections"<<std::endl;
		
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
			std::cout<<"select error"<<std::endl;
		}

		if (FD_ISSET(master_socket, &readfds)){
			if ((new_socket = accept(master_socket,
					(struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
				perror("accept");
				exit(EXIT_FAILURE);
			}
			
            std::cout<<"New connection: "<<new_socket<<" on ip: "<<inet_ntoa(address.sin_addr)<<" port: "<<ntohs(address.sin_port)<<std::endl;
		
			if( send(new_socket, message, strlen(message), 0) != strlen(message) ){
				perror("send");
			}
				
			puts("msg sent");

			for (i = 0; i < max_clients; i++){
				if( client_socket[i] == 0 ){
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
						
					break;
				}
			}
		}

		for (i = 0; i < max_clients; i++){
			sd = client_socket[i];

			if (FD_ISSET( sd , &readfds)){
				if ((valread = read( sd , buffer, 1024)) == 0){
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    std::cout<<"Host disconnected: "<<new_socket<<" on ip: "<<inet_ntoa(address.sin_addr)<<" port: "<<ntohs(address.sin_port)<<std::endl;
					close( sd );
					client_socket[i] = 0;
				}

				else{
                    buffer[valread] = '\0';
                    char* res = resulter(buffer);
                    // sprintf(res, "Result: %s", res);
                    // std::cout<<res<<std::endl;
                    send(sd, res, strlen(res), 0);
				}
			}

		}
	}
		
	return 0;
}
