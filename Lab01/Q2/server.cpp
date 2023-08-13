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
                    // buffer[valread]='\0';
                    // std::string res = buffer;
                    // char* decoded_msg = new char[res.size()];
                    // for(int j=0; j<res.size(); j++) decoded_msg[i]=res[i];
                    
                    // int msg_type = res[0]-'0';
                    // std::cout<<msg_type<<std::endl;
                    // std::string msg_body;
                    // for(int j=2; j<res.size(); j++) msg_body.push_back(res[j]);

                    // if(msg_type==1){
                    //     std::cout<<"Message recieved from client: "<<sd<<" :"<<msg_body<<std::endl;
                    //     // send(sd, "message recieved by server", 27, 0);
                    // }else if(msg_type==2){
                    //     std::cout<<"Acknowledgement recieved from client: "<<sd<<" "<<msg_body<<std::endl;
                    //     // send(sd, "acknowledgement recieved", 25, 0);
                    // }else if(msg_type==3){
                    //     std::cout<<"Received close communication from client: "<<sd<<" "<<msg_body<<std::endl;
                    //     close(sd);
                    //     client_socket[i]=0;
                    // }
					buffer[valread] = '\0';
					std::cout<<buffer<<std::endl;
				}
			}

		}
		if(!first){
			char* buffer;
			std::cin>>buffer;

			for(int i=0; i<max_clients; i++){
				sd = client_socket[i];
				if(FD_ISSET(sd, &readfds)){
					send(sd, buffer, strlen(buffer), 0);
				}
			}
		}else{
			first = false;
		}
	}
		
	return 0;
}
