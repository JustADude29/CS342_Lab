#include <ostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <iostream>

static const int B64index[256] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 62, 63, 62, 62, 63, 52, 53, 54, 55,
56, 57, 58, 59, 60, 61,  0,  0,  0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,
7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,  0,
0,  0,  0, 63,  0, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51 };

std::string b64decode(char* data, const size_t len)
{
    unsigned char* p = (unsigned char*)data;
    int pad = len > 0 && (len % 4 || p[len - 1] == '=');
    const size_t L = ((len + 3) / 4 - pad) * 4;
    std::string str(L / 4 * 3 + pad, '\0');

    for (size_t i = 0, j = 0; i < L; i += 4)
    {
        int n = B64index[p[i]] << 18 | B64index[p[i + 1]] << 12 | B64index[p[i + 2]] << 6 | B64index[p[i + 3]];
        str[j++] = n >> 16;
        str[j++] = n >> 8 & 0xFF;
        str[j++] = n & 0xFF;
    }
    if (pad)
    {
        int n = B64index[p[L]] << 18 | B64index[p[L + 1]] << 12;
        str[str.size() - 1] = n >> 16;

        if (len > L + 2 && p[L + 2] != '=')
        {
            n |= B64index[p[L + 2]] << 6;
            str.push_back(n >> 8 & 0xFF);
        }
    }
    return str;
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

	for (i = 0; i < max_clients; i++)
	{
		client_socket[i] = 0;
	}

	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
		sizeof(opt)) < 0 )
	{
		perror("setsockopt error");
		exit(EXIT_FAILURE);
	}
	
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(argv[1]);
    std::cout<<"ip: "<<address.sin_addr.s_addr<<std::endl;
	address.sin_port = htons( PORT );

	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	std::cout<<"Server on Port: "<<PORT<<std::endl;
		
	if (listen(master_socket, 3) < 0)
	{
		perror("listen error");
		exit(EXIT_FAILURE);
	}

	addrlen = sizeof(address);
	std::cout<<"Waiting for connections"<<std::endl;
		
	
	while(1){
		FD_ZERO(&readfds);

		FD_SET(master_socket, &readfds);
		max_sd = master_socket;
		
		for ( i = 0 ; i < max_clients ; i++)
		{
			sd = client_socket[i];

			if(sd > 0)
				FD_SET( sd , &readfds);

			if(sd > max_sd)
				max_sd = sd;
		}

		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
	
		if ((activity < 0) && (errno!=EINTR))
		{
			std::cout<<"select error"<<std::endl;
		}

		if (FD_ISSET(master_socket, &readfds))
		{
			if ((new_socket = accept(master_socket,
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
			{
				perror("accept");
				exit(EXIT_FAILURE);
			}
			
            std::cout<<"New connection: "<<new_socket<<" on ip: "<<inet_ntoa(address.sin_addr)<<" port: "<<ntohs(address.sin_port)<<std::endl;
		
			if( send(new_socket, message, strlen(message), 0) != strlen(message) )
			{
				perror("send");
			}
				
			puts("msg sent");

			for (i = 0; i < max_clients; i++)
			{
				if( client_socket[i] == 0 )
				{
					client_socket[i] = new_socket;
					printf("Adding to list of sockets as %d\n" , i);
						
					break;
				}
			}
		}

		for (i = 0; i < max_clients; i++)
		{
			sd = client_socket[i];

			if (FD_ISSET( sd , &readfds))
			{
				if ((valread = read( sd , buffer, 1024)) == 0)
				{
					getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    std::cout<<"Host disconnected: "<<new_socket<<" on ip: "<<inet_ntoa(address.sin_addr)<<" port: "<<ntohs(address.sin_port)<<std::endl;
					close( sd );
					client_socket[i] = 0;
				}

				else
				{
                    buffer[valread]='\0';
                    std::string res = b64decode(buffer, strlen(buffer));
                    char* decoded_msg = new char[res.size()];
                    for(int j=0; j<res.size(); j++) decoded_msg[i]=res[i];
                    
                    int msg_type = res[0]-'0';
                    std::cout<<msg_type<<std::endl;
                    std::string msg_body;
                    for(int j=2; j<res.size(); j++) msg_body.push_back(res[j]);

                    if(msg_type==1){
                        std::cout<<"Message recieved from client: "<<sd<<" :"<<msg_body<<std::endl;
                        // send(sd, "message recieved by server", 17, 0);
                    }else if(msg_type==2){
                        std::cout<<"Acknowledgement recieved from client: "<<sd<<" "<<msg_body<<std::endl;
                        // send(sd, "acknowledgement recieved", 25, 0);
                    }else if(msg_type==3){
                        std::cout<<"Received close communication from client: "<<sd<<" "<<msg_body<<std::endl;
                        close(sd);
                        client_socket[i]=0;
                    }
				}
			}
		}
	}
		
	return 0;
}
