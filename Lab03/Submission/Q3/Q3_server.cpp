#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

using namespace std;

uint16_t calculateChecksum(const string& message) {
    string paddedMessage = message;

    if (paddedMessage.length() % 2 != 0) {
        paddedMessage.push_back('\0');
    }

    uint32_t sum = 0;

    for (size_t i = 0; i < paddedMessage.length(); i += 2) {
        uint16_t word = (paddedMessage[i] << 8);
        if (i + 1 < paddedMessage.length()) {
            word |= paddedMessage[i + 1];
        }
        sum += word;

        if (sum & 0xFFFF0000) {
            sum &= 0xFFFF;
            sum++;
        }
    }

    return ~sum & 0xFFFF;
}

int main(int argc, char* argv[]) {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    int PORT = 8000;
    string IP = "127.0.0.1";
    if(argc == 2){
        PORT = atoi(argv[1]);
    }
    else
        cout<<"Using default port: 8000\nUse ./<executable> <PORT> to set custom onen";

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr<<"Error creating socket"<<endl;
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr<<"Error binding socket"<<endl;
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, 5) == -1) {
        cerr<<"Error listening for connections"<<endl;
        exit(EXIT_FAILURE);
    }

    cout << "Server is listening for connections on port " << PORT << "..." << endl;

    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == -1) {
        cerr<<"Error accepting client connection"<<endl;
        exit(EXIT_FAILURE);
    }

    cout << "Client connected." << endl;

    char buffer[1024];
    while (true) {
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            cout << "Client disconnected." << endl;
            break;
        }
        buffer[bytesRead] = '\0';
        int i = 0;
        while(true)
        {
            if (buffer[i] == ':')
                break;
            i++;
        }
        string buffer_str(buffer);
        int checksum_recv = stoi(buffer_str.substr(0, i));
        int checksum = calculateChecksum(buffer_str.substr(i + 2, buffer_str.size() - i - 2));
        if (checksum != checksum_recv)
        {
            cout << "Received message is corrupted. Sending NAK!" << endl;
            cout << "Received checksum: " << checksum_recv << ", Calculated checksum: " << checksum << endl;
            const char* nackMessage = "NAK: Message received by server was corrupted.";
            send(clientSocket, nackMessage, strlen(nackMessage), 0);
        }
        else
        {
            cout << "Recieved from Client: " << buffer << endl;
            const char* ackMessage = "ACK: Server received your message.";
            send(clientSocket, ackMessage, strlen(ackMessage), 0);
        }
    }

    close(clientSocket);
    close(serverSocket);

    return 0;
}
