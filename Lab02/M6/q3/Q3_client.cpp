#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <cstdlib>
#include <sstream>

using namespace std;

#define MAX_CLIENTS 10

const char *SERVER_IP = "127.0.0.1";  // Server IP address
const int SERVER_PORT = 3000;

string MY_IP;
int PORT = 3000;

int MY_ID = -1;  // Initialize the client ID to -1

// Define ANSI escape codes for colors
const string ANSI_RESET = "\033[0m";
const string ANSI_RED = "\033[91m";
const string ANSI_BLUE = "\033[94m";
const string ANSI_GREEN = "\033[92m";
const string ANSI_YELLOW = "\033[33m";

int client_socket[MAX_CLIENTS];
int master_socket;

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

        if(strcmp(buffer, "/exit") == 0) {
            client_socket[clientClientId] = 0;
            close(clientSocket);
            cout << ANSI_YELLOW << "Client " << clientClientId << " gracefuly disconnected." << ANSI_RESET << endl;
            break;
        }
        cout << ANSI_GREEN << "Client " << to_string(clientClientId) << " to you: " << ANSI_RESET << buffer << endl;
    }
}

// Handling messages coming from server socket
void receiveMessages(int serverSocket) {
    // Create a buffer to receive messages
    char buffer[1024];
    int valread;

    while (true) {
        valread = recv(serverSocket, buffer, sizeof(buffer), 0);
        if (valread <= 0) {
            cout << ANSI_RED << "Disconnected from the server." << ANSI_RESET << endl;
            break;
        }

        buffer[valread] = '\0';
        cout << buffer << endl;
    }
}

// Accepting new users connecting to the client
void newClients(void) {
    int new_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    while(1) {
        if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        char buffer[1024] = {0};
        valread = recv(new_socket, buffer, sizeof(buffer), 0);

        if (valread <= 0 || strcmp(buffer, "/exit") == 0) {
            close(new_socket);
            continue;
        }

        int cl_id = atoi(buffer);
        cout << ANSI_YELLOW << "New client connected. Client ID: " << cl_id << ANSI_RESET << endl;
        client_socket[cl_id] = new_socket;
        thread clientThread(handleClient, cl_id);
        clientThread.detach();
    }
    
}

int main() {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        client_socket[i] = 0;
    }

    // First, connecting to the server

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Socket creation failed." << endl;
        return EXIT_FAILURE;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr << "Connection to the server failed." << endl;
        close(serverSocket);
        return EXIT_FAILURE;
    }

    // Receiving the assigned client ID from the server, as well as info of other clients connected to the server
    char clientIdMessage[1024] = {0};
    int bytesRead = recv(serverSocket, clientIdMessage, sizeof(clientIdMessage), 0);
    if (bytesRead > 0) {
        clientIdMessage[bytesRead] = '\0';
    } else {
        string err_res = "/exit";
        send(serverSocket, err_res.c_str(), err_res.size(), 0);
        cerr << "Failed to receive client ID." << endl;
        close(serverSocket);
        exit(0);
    }

    char Client_IP[20];
    sscanf(clientIdMessage, "%[^/]", Client_IP);
    MY_IP = Client_IP;
    int new_socket, addrlen, i, valread;
    int max_sd;

    struct sockaddr_in address;

    char buffer[1025];

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
    address.sin_addr.s_addr = inet_addr(Client_IP);
    address.sin_port = htons(PORT);

    while(bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        // MAX_CLIENTS = 10, so all ports for listening can be found in this range
        if(PORT == 3050) {
            string err_res = "/exit";
            send(serverSocket, err_res.c_str(), err_res.size(), 0);
            cerr << "Bind function failed." << endl;
            close(serverSocket);
            exit(0);
        }
        PORT++;
        address.sin_port = htons(PORT);
    }

    if (listen(master_socket, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Begining accepting new connection requests from other clients
    thread newClientTrd(newClients);
    newClientTrd.detach();

    string server_conf = (string)Client_IP + " " + to_string(PORT);
    send(serverSocket, server_conf.c_str(), server_conf.size(), 0);

    char ip_port_list[1024], ip_port_buffer[100], temp_ip[20];
    sscanf(clientIdMessage, "%[^/]/%d/%s", Client_IP, &MY_ID, ip_port_list);
    
    cout << "Connected to" << ANSI_GREEN << " ChatServer v2.0.0.1." << ANSI_YELLOW << " You're client ID: " << MY_ID << ANSI_RESET <<  endl;
    cout << ANSI_YELLOW << "Message format: " << ANSI_RESET << "'/ALL' to broadcast\n\t\t'/<client_ID>' to send message privately to a client\n\t\t'/exit' to exit\n";
    
    // Begining monitoring server messages
    thread serverTrd(receiveMessages, serverSocket);
    serverTrd.detach();

    // Connecting to all clients the server was connected to when the current client connected 
    char* cptr = &clientIdMessage[0] + strlen(Client_IP) + 2 + to_string(MY_ID).size();
    while(*cptr != '\0'){
        sscanf(cptr, "%[^/]", ip_port_buffer);
        cptr += strlen(ip_port_buffer) + 1;
        int temp_id, temp_port;
        sscanf(ip_port_buffer, "%d %s %d", &temp_id, temp_ip, &temp_port);
        
        int newSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (newSocket == -1) {
            cerr << "Socket creation failed." << endl;
            return EXIT_FAILURE;
        }

        sockaddr_in newAddr{};
        newAddr.sin_family = AF_INET;
        newAddr.sin_port = htons(temp_port);
        newAddr.sin_addr.s_addr = inet_addr(temp_ip);

        if (connect(newSocket, (struct sockaddr *)&newAddr, sizeof(newAddr)) == -1) {
            cerr << "Connection to client " << temp_id << " failed." << endl;
            close(newSocket);
            continue;
        }

        // Asinging the same ID to the client as used by the server
        client_socket[temp_id] = newSocket;
        cout << ANSI_YELLOW << "Connected to client with ID: " << temp_id << ANSI_RESET << endl;

        string ID_msg = to_string(MY_ID);
        send(newSocket, ID_msg.c_str(), ID_msg.size(), 0);
        thread clientThread(handleClient, temp_id);
        clientThread.detach();
    }

    while(1) {
        string message;
        getline(cin, message);

        // Checking for prefix in the message from user, and handling the message accordingly
        char pref[7], suff[1024];
        sscanf(message.c_str(), "%6s", pref);
        sprintf(suff, "%s", message.c_str() + 1 + strlen(pref));
        string msg_pref = pref;

        // Exit if the user types "/exit"
        if (msg_pref == "/exit") {
            send(serverSocket, message.c_str(), message.size(), 0);
            for(int i = 0; i < MAX_CLIENTS; i++) {
                if(client_socket[i] != 0) {
                    send(client_socket[i], message.c_str(), message.size(), 0);
                }
            }
            cout << ANSI_YELLOW << "Gracefully exiting." << ANSI_RESET << endl;
            break;
        } else if(msg_pref[0] == '/' && isdigit(msg_pref[1])) {
            if(atoi(msg_pref.c_str() + 1) < MAX_CLIENTS && client_socket[atoi(msg_pref.c_str() + 1)] != 0) {
                send(client_socket[atoi(msg_pref.c_str() + 1)], suff, strlen(suff), 0);
            } else if(atoi(msg_pref.c_str() + 1) == MY_ID) {
                cout << ANSI_YELLOW << "Message to self: " << ANSI_RESET << suff << endl;
            }
            else {
                cout << ANSI_RED << "Invalid Prefix: " << ANSI_RESET << "Client " << atoi(msg_pref.c_str() + 1) << " not connected.\n";
            }
        } else if(msg_pref == "/ALL") {
            send(serverSocket, suff, strlen(suff), 0);
        }
        else {
            cout << ANSI_RED << "Invalid Prefix.\n" << ANSI_RESET;
        }
    }
    close(serverSocket);
    return 0;
}
