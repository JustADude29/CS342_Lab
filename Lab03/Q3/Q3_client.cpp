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

uint16_t calculateChecksum(const string message) {
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

        // Handle carry
        if (sum & 0xFFFF0000) {
            sum &= 0xFFFF;
            sum++;
        }
    }

    return ~sum & 0xFFFF;
}

string convertToString(char* a, int size)
{
    int i;
    string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

int main(int argc, char* argv[]) {
    int clientSocket;
    struct sockaddr_in serverAddr;

    int PORT = 8000;
    string IPv = "127.0.0.1";

    if(argc == 1){
        cout<<"Defaulting to IP: 127.0.0.1 and PORT: 8000\nUse ./<executable> <IP> <PORT> to set custom\n";
    }
    else if(argc == 3){
        IPv = argv[1];
        PORT = atoi(argv[2]);
    }else{
        cout<<"ERROR: wrong format\nUse ./<executable> <IP> <PORT> to set custom\n";
        exit(1);
    }

    const char* IP = IPv.c_str();

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        cerr<<"Error creating socket\n";
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        cerr<<"Error connecting to the server\n";
        exit(EXIT_FAILURE);
    }

    cout << "Connected to the server." << endl;

    while (true) {
        string message;
        cout << "Enter a message to send to the server (or 'q' to quit): ";
        getline(cin, message);

        if (message == "q") {
            break;
        }

        int checksum = calculateChecksum(message);
        string message_wsc = to_string(checksum) + ": " + message;
        if (rand() % 10 < 6)
            message_wsc[message_wsc.size() - 1]++;
        send(clientSocket, message_wsc.c_str(), message_wsc.length(), 0);

        char buffer[1024];
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead <= 0) {
            cerr << "Error receiving acknowledgment." << endl;
            break;
        }
        buffer[bytesRead] = '\0';
        string buffer2 = convertToString(buffer, bytesRead);
        if (buffer2.substr(0, 3) == "ACK")
            cout << "Recieved from Server: " << buffer << endl;
        else if(buffer2.substr(0, 3) == "NAK")
        {
            do
            {
                cout << "Recieved from Server: " << buffer << endl << "Re-transmitting..." << endl;
                message_wsc = to_string(checksum) + ": " + message;
                if (rand() % 10 < 6)
                    message_wsc[message_wsc.size() - 1]++;
                send(clientSocket, message_wsc.c_str(), message_wsc.length(), 0);
                bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
                buffer[bytesRead] = '\0';
                buffer2 = convertToString(buffer, bytesRead);
                cout << "Recieved from Server: " << buffer << endl;
            }
            while(buffer2.substr(0, 3) == "NAK");
        }
    }

    close(clientSocket);

    return 0;
}
