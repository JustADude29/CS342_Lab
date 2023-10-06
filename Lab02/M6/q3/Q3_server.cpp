#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>

using namespace std;

const char *HOST = "127.0.0.1";  // Bind to all available network interfaces
const int PORT = 3000;
const int MAX_CLIENTS = 10;

int client_socket[MAX_CLIENTS];
string client_ip[MAX_CLIENTS];
int client_port[MAX_CLIENTS];
int master_socket;

// Define ANSI escape codes for colors
const string ANSI_RESET = "\033[0m";
const string ANSI_RED = "\033[91m";
const string ANSI_BLUE = "\033[94m";
const string ANSI_GREEN = "\033[92m";
const string ANSI_YELLOW = "\033[33m";

// Function to broadcast a message from one client to all others
void broadcast(const string &message, int senderClientId) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_socket[i] != 0 && i != senderClientId) {
            send(client_socket[i], message.c_str(), message.size(), 0);
        }
    }
}

// Handling messages coming from a client socket
void handleClient(int clientClientId) {
    int clientSocket = client_socket[clientClientId];
    char buffer[1024];
    int valread;

    while (true) {
        valread = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (valread <= 0) {
            cout << ANSI_RED << "Client " << clientClientId << " abruplty disconnected." << ANSI_RESET << endl;
            close(clientSocket);
            client_socket[clientClientId] = 0;
            break;
        }

        buffer[valread] = '\0';
        /*/exit is setup so that the client themself send to all clients 
        they are connected to and thus, doesn't need to be broadcasted. */ 
        if(strcmp(buffer, "/exit") == 0) {
            client_socket[clientClientId] = 0;
            client_ip[clientClientId] = "";
            client_port[clientClientId] = -1;
            close(clientSocket);
            cout << ANSI_YELLOW << "Client " << clientClientId << " disconnected." << ANSI_RESET << endl;
            break;
        }
        // Sending message to be braodcasted
        string formattedMessage = ANSI_BLUE + "Client " + to_string(clientClientId) + " to everyone: " + ANSI_RESET + buffer;
        broadcast(formattedMessage, clientClientId);
    }
}

int main() {
    int new_socket, addrlen, i, valread;
    struct sockaddr_in address;
    char buffer[1025];

    for (i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
        client_port[i] = -1;
        client_ip[i] = "";
    }

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = true;
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(HOST);
    address.sin_port = htons(PORT);

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    cout << "Listener on port " << PORT << endl;

    if (listen(master_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    cout << "Waiting for connections ..." << endl;

    while (true) {
        if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        cout << "New connection request, socket fd is " << new_socket << ", client IP is : " << inet_ntoa(address.sin_addr) << ", client port is: " << ntohs(address.sin_port) << endl;

        // Find an available client ID
        int clientId = -1;
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_socket[i] == 0) {
                clientId = i;
                client_socket[i] = new_socket;
                cout << "Adding to list of sockets as client " << clientId << endl;
                break;
            }
        }

        // If there are too many clients, send a message and close the socket
        if (clientId == -1) {
            cout << "Couldn't establish connection: Max Clients Reached\n";
            string maxClientsMessage = ANSI_RED + "Maximum number of clients reached. Connection closed.\n" + ANSI_RESET;
            send(new_socket, maxClientsMessage.c_str(), maxClientsMessage.size(), 0);
            close(new_socket);
        } else {
            /*Sending a standardised message from the server to the new client, 
              which gives information(ID, IP, Port num.) about all clients currently connected to the server*/ 
            string clientIdMessage = (string)inet_ntoa(address.sin_addr) + "/" + to_string(clientId) + "/";
            for(int i = 0; i < MAX_CLIENTS; i++) {
                if(client_socket[i] > 0 && i != clientId) {
                    clientIdMessage += to_string(i) + " " + client_ip[i] + " " + to_string(client_port[i]) + "/";
                }
            }
            send(client_socket[clientId], clientIdMessage.c_str(), clientIdMessage.size(), 0);
            
            /*The client returns a standardised message telling the port it is listening on for p2p connections*/
            valread = recv(client_socket[clientId], buffer, sizeof(buffer), 0);
            if (valread <= 0 || strcmp(buffer, "/exit") == 0) {
                cout << ANSI_RED << "Client " << clientId << " disconnected." << ANSI_RESET << endl;
                close(client_socket[clientId]);
                client_socket[clientId] = 0;
                continue;
            }
            
            char buf_ip[50];
            int buf_port;
            sscanf(buffer, "%s %d", buf_ip, &buf_port);

            // Fixing the IP and Port num. of new client before going on and accepting connections from other clients
            client_port[clientId] = buf_port;
            client_ip[clientId] = buf_ip;

            // Handle the new client in a separate thread
            thread clientThread(handleClient, clientId);
            clientThread.detach();
        }

    }

    return 0;
}
