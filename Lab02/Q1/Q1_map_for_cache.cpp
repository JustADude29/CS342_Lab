#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <utility>
#include <ctime>

#define MSG_LEN 1024

const char *DNS_SERVER_IP = "8.8.8.8";
const int DNS_SERVER_PORT = 53;

std::map <std::string, std::pair<std::time_t, std::vector<std::pair<int, std::string>>> > dns_cache;

std::vector<uint8_t> dnsReq(std::string &query)
{
    std::vector<uint8_t> request{0x67, 0xA0, 0x01, 0x00,
                                 0x00, 0x01, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00};

    std::istringstream iss(query);
    std::string token;

    while (std::getline(iss, token, '.'))
    {
        request.push_back(token.size());
        std::copy(token.begin(), token.end(), std::back_inserter(request));
    }
    request.push_back(0);
    request.push_back(0);
    request.push_back(1);
    request.push_back(0);
    request.push_back(1);
    return request;
}

std::vector<std::pair<int, std::string>> dnsr_parse(uint8_t *buffer, int msglen)
{
    std::vector<std::pair<int, std::string>> response;

    // Check if the message is at least 12 bytes long (DNS header size).
    if (msglen < 12)
    {
        std::cerr << "Invalid DNS response length" << std::endl;
        return response;
    }

    // Extract the number of answer records from the DNS header.
    int num_answers = (buffer[6] << 8) + buffer[7];

    // Pointer to the start of the answer section.
    uint8_t *answer_ptr = buffer + 12;
    while (*answer_ptr != 0)
    {
        answer_ptr++;
    }
    answer_ptr += 5;

    // Iterate through each answer record.
    for (int i = 0; i < num_answers; ++i)
    {
        answer_ptr += 2;
        int type = (answer_ptr[0] << 8) + answer_ptr[1];
        answer_ptr += 4;
        int ttl = (answer_ptr[0] << 24) + (answer_ptr[1] << 16) + (answer_ptr[2] << 8) + (answer_ptr[3]);
        answer_ptr += 4;
        int len = (answer_ptr[0] << 8) + answer_ptr[1];
        answer_ptr += 2;
        std::string r = "";
        while (len--)
        {
            if (type == 1) r += std::to_string((*answer_ptr));
            if (type == 1 && len > 0) r += ".";
            answer_ptr++;
        }
        if (type == 1) response.push_back({ttl, r});
    }

    return response;
}

void print(uint8_t *buf, int n)
{
    for (int i = 0; i < n; i++)
    {
        std::cout << (int)buf[i] << " ";
    }
    std::cout << std::endl;
}

int main()
{
    printf("DNS lookup service v0.0.6.9 \n");

    int fd_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_sock < 0)
    {
        fprintf(stderr, "ERROR: socket func failed\n");
        exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(DNS_SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(DNS_SERVER_IP);

    std::string domain_inp;
    while(1) {
        std::cout << "Enter a domain to lookup(or \\X to exit): ";
        std::cin >> domain_inp;

        if(domain_inp == "\\X") {
            printf("Exiting DNS lookup service.\nGoodbye! :'(\n");
            break;
        }
        std::vector<uint8_t> dns_request = dnsReq(domain_inp);

        bool chk = 1;
        if(dns_cache.count(domain_inp) > 0){
            time_t cache_t = dns_cache[domain_inp].first, pres_t = std::time(nullptr);
            for(auto p: dns_cache[domain_inp].second) {
                if(p.first + cache_t < pres_t) chk = 0;
            }
            if(chk) {
                std::cout << "Number of responses(cache): " << dns_cache[domain_inp].second.size() << "\n";
                for(auto p: dns_cache[domain_inp].second) {
                    std::cout << p.second << ", ttl(remaining) = " << cache_t + p.first - pres_t << "\n";
                }
                continue;
            }
            else {
                dns_cache.erase(domain_inp);
            }
        }

        sendto(fd_sock, dns_request.data(), dns_request.size(), 0, (struct sockaddr *)&server, sizeof(server));

        uint8_t buffer[MSG_LEN + 1] = {0};
        socklen_t addrlen = sizeof(server);
        int msglen = recvfrom(fd_sock, buffer, MSG_LEN, 0, (struct sockaddr *)&server, &addrlen);
        if (msglen > 0)
            buffer[msglen] = '\0';
        unsigned int ttl;
        std::vector<std::pair<int, std::string>> dns_response = dnsr_parse(buffer, msglen);
        if(dns_response.size() != 0) 
            dns_cache[domain_inp] = {std::time(nullptr), dns_response};
        std::cout << "Number of reponses = " << dns_response.size() << std::endl;
        for (auto p : dns_response)
            std::cout << p.second << ", ttl = " << p.first <<  std::endl;
    }
    return 0;
}