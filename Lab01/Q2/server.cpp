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

#define LOG(x) std::cout<<x<<std::endl;
	
int main(int argc , char *argv[])
{
	if(argc!=2){
		std::cerr<<"Usage: ./server <PORT>"<<std::endl;
		return -1;
	}
    uint PORT = std::stoi(argv[1]);
    uint MSG_LEN=1024;
    
    int max_clients = 30;
	
	int master_socket, addrlen, new_socket, client_socket[max_clients], activity, i, valread, sd;
	int max_sd;
	struct sockaddr_in address;
		
	char buffer[MSG_LEN];

	fd_set readfds;

	char *message = "Welcome to socketCHATter 2\n";

	for (i = 0; i < max_clients; i++){
		client_socket[i] = 0;
	}

	if((master_socket = socket(AF_INET , SOCK_STREAM , 0)) < 0){
		std::cerr<<"socket error"<<std::endl;
		return -1;
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );

	if(bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){
		std::cerr<<"bind error"<<std::endl;
		return -1;
	}
	std::cout<<"Server on Port: "<<PORT<<std::endl;
		
	if(listen(master_socket, 20) < 0){
		std::cerr<<"listen error"<<std::endl;
		return -1;
	}

	addrlen = sizeof(address);
	std::cout<<"Waiting for connections"<<std::endl;
		
	bool first = true;
	while(1){
		FD_ZERO(&readfds);

		FD_SET(master_socket, &readfds);
		FD_SET(STDIN_FILENO, &readfds);
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

		if(FD_ISSET(STDIN_FILENO, &readfds)){
			valread = read(STDIN_FILENO, buffer, MSG_LEN);
			buffer[valread]='\0';
			sd = client_socket[(int)(buffer[0]-'0')];
			if(sd==0){
				LOG("No user with id "  +  buffer[0]);
			}else{
				send(sd, buffer, strlen(buffer), 0);
			}
			continue;
		}

		if (FD_ISSET(master_socket, &readfds)){
			if ((new_socket = accept(master_socket,
					(struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
				std::cerr<<"accept error"<<std::endl;
				return -1;
			}
			
            std::cout<<"New connection: "<<new_socket<<" on ip: "<<inet_ntoa(address.sin_addr)<<" port: "<<ntohs(address.sin_port)<<std::endl;
		
			if( send(new_socket, message, strlen(message), 0) != strlen(message) ){
				std::cerr<<"send error"<<std::endl;
			}

			for (i = 0; i < max_clients; i++){
				if( client_socket[i] == 0 ){
					client_socket[i] = new_socket;
					std::cout<<"Adding to socket list at: "	<<i<<std::endl;
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
					std::cout<<buffer<<std::endl;
				}
			}

		}
	}
		
	return 0;
}
