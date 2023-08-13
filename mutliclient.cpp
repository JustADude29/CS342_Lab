#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, valread, activity;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    fd_set readfds;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"Error: Command usage is \'%s <hostname> <port>\n\'", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
            exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    while(1) {

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        activity = select( sockfd + 1 , &readfds , NULL , NULL , NULL);
        if ((activity < 0) && (errno!=EINTR))
		{
			printf("select error");
		}
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            valread = read( STDIN_FILENO , buffer, 1024);
            buffer[valread] = '\0';
            send(sockfd , buffer , strlen(buffer) , 0 );
            printf("Sent!\n");
        }
        else {
            valread = read( sockfd , buffer, 1024);
            buffer[valread] = '\0';
            if(strlen(buffer) == 0) break;
            printf("%s\n", buffer);
        }
    }


    // while(1){
    //     printf("Please enter the message: ");
    //     bzero(buffer,256);
    //     fgets(buffer,255,stdin);
    //     n = write(sockfd, buffer, strlen(buffer));
    //     if((buffer[0] == 'x' || buffer[0] == 'X') && buffer[2] == 0) break;
    //     if (n < 0) 
    //         error("ERROR writing to socket");
    //     bzero(buffer,256);
    //     n = read(sockfd, buffer, 255);
    //     if (n < 0) 
    //         error("ERROR reading from socket");
    //     printf("%s\n", buffer);
    // }
    close(sockfd);
    return 0;
}