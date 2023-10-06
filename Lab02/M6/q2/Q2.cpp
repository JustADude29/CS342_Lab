#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
using namespace std;

// Define constants for cache size and URL length
#define CACHE_SIZE 5
#define URL_MAX_LEN 1024
#define PAGE_MAX_LEN 1024*1024

// Define ANSI escape codes for colors
const string ANSI_RESET = "\033[0m";
const string ANSI_RED = "\033[91m";
const string ANSI_BLUE = "\033[94m";
const string ANSI_GREEN = "\033[92m";
const string ANSI_YELLOW = "\033[33m";

// Structure to represent a web page
typedef struct WebPage {
    char url[URL_MAX_LEN];
    char content[PAGE_MAX_LEN]; // Adjust content size as needed
    struct WebPage* next;
} WebPage;

// Structure to represent the cache
typedef struct Cache {
    WebPage* head;
    int size;
} Cache;

// Initialize the cache
void initializeCache(Cache* cache) {
    cache->head = NULL;
    cache->size = 0;
}

// Function to fetch a web page using an HTTP GET request
char* fetchWebPage(const char* url) {

    // Parsing the url into host and path
    char* host = (char*) malloc(strlen(url) + 1);
    char* path = (char*) malloc(strlen(url) + 1);
    char* prot_chk = (char*) malloc(strlen(url) + 1);

    // Getting the protocal in the url
    sscanf(url, "%[^/]", prot_chk);
    if(strcmp(prot_chk, "http:") != 0) {
        fprintf(stderr, "%sPlease enter a URL with the protocol http (http://%s)%s\n", ANSI_RED.c_str(), url, ANSI_RESET.c_str());
        return NULL;
    }
    if(strlen(url) - strlen(prot_chk) < 3 || *(url + strlen(prot_chk)) != '/' || *(url + strlen(prot_chk) + 1) != '/') {
        fprintf(stderr, "%sInvalid URL format: %s%s\n", ANSI_RED.c_str(), url, ANSI_RESET.c_str());
        return NULL;
    }

    // Only http requests can be managed with this question, thus such a strict criteria.
    if (sscanf(url, "http://%[^/]/%s", host, path) == 0) {
        fprintf(stderr, "%sInvalid URL format: %s%s\n", ANSI_RED.c_str(), url, ANSI_RESET.c_str());
        return NULL;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        string msg = ANSI_RED + "Error creating socket" + ANSI_RESET;
        perror(msg.c_str());
        return NULL;
    }

    // Resolve the host name to an IP address
    struct hostent* server = gethostbyname(host);
    if (server == NULL) {
        cout << ANSI_RED << "Error resolving host" << ANSI_RESET << endl;
        close(sockfd);
        return NULL;
    }

    // Prepare the server address structure
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(80); // HTTP port

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        string msg = ANSI_RED + "Error connecting to server" + ANSI_RESET;
        perror(msg.c_str());
        close(sockfd);
        return NULL;
    }

    // Create an HTTP GET request
    char request[1024];
    snprintf(request, sizeof(request), "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", host);

    // Send the request
    if (send(sockfd, request, strlen(request), 0) < 0) {
        string msg = ANSI_RED + "Error sending request" + ANSI_RESET;
        perror(msg.c_str());
        close(sockfd);
        return NULL;
    }

    char buffer[PAGE_MAX_LEN]; // Adjust buffer size as needed
    char* content = (char*)malloc(PAGE_MAX_LEN); // Allocate memory for content
    int content_length = 0;
    int bytesReceived;

    // Keep adding received data at the end of the data already recieved
    while (bytesReceived = recv(sockfd, content + strlen(content), PAGE_MAX_LEN - strlen(content), 0 ) > 0);

    // Close the socket
    close(sockfd);

    // Null-terminate the content
    content[strlen(content)] = '\0';

    return content;
}

// Function to evict the least recently used page from the cache
void evictLRUPage(Cache* cache) {
    if (cache->head == NULL) {
        return; // Cache is empty
    }

    WebPage* current = cache->head;
    WebPage* prev = NULL;

    // Traverse to the end of the cache to find the LRU page
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    // Remove the LRU page from the cache
    free(current);
    if (prev != NULL) {
        prev->next = NULL;
    } else {
        cache->head = NULL;
    }
}

// Function to add a web page to the cache
void addToCache(Cache* cache, const char* url, const char* content) {
    // Create a new web page node
    WebPage* newPage = (WebPage*)malloc(sizeof(WebPage));
    strncpy(newPage->url, url, URL_MAX_LEN - 1);
    strncpy(newPage->content, content, PAGE_MAX_LEN - 1);

    // Add the new page to the front of the cache
    newPage->next = cache->head;
    cache->head = newPage;

    // Check if the cache size exceeds the limit, and evict the LRU page if needed
    if (cache->size >= CACHE_SIZE) {
        evictLRUPage(cache);
    } else {
        cache->size++;
    }
}

// Function to retrieve a web page from the cache or fetch it if not present
const char* retrieveWebPage(Cache* cache, const char* url) {
    // Search for the URL in the cache
    WebPage* current = cache->head;
    WebPage* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->url, url) == 0) {
            // Move the accessed page to the front (MRU position)
            if (prev != NULL) {
                prev->next = current->next;
                current->next = cache->head;
                cache->head = current;
            }
            return current->content;
        }
        prev = current;
        current = current->next;
    }

    // Page not found in cache, fetch it
    char* content = fetchWebPage(url);

    if(content == NULL) return NULL;
    // Add the fetched page to the cache
    addToCache(cache, url, content);

    return content;
}

// Function to display the contents of the cache
void displayCache(const Cache* cache) {
    WebPage* current = cache->head;
    cout << ANSI_GREEN << "Cache Contents" << ANSI_RESET << " (Most Recently Used to Least Recently Used):\n";
    while (current != NULL) {
        printf("%s\n", current->url);
        current = current->next;
    }
}

int main() {
    // Initializing the cache
    Cache cache;
    initializeCache(&cache);

    cout << "Welcome to " << ANSI_YELLOW << "WebpageRetriever v0.0.2" << ANSI_RESET << endl;
    string str;

    // Retrieve a web page from the cache or fetch it
    while(1) {
        cout << ANSI_BLUE << "Enter the URL to retrieve(or type \\X to exit): " << ANSI_RESET;
        cin >> str;
        if(str == "\\X") {
            cout << ANSI_YELLOW << "Exiting WebpageRetriever.\nBye!\n" << ANSI_RESET;
            break;
        }
        const char* urlToRetrieve = const_cast<char*>(str.c_str());
        const char* content = retrieveWebPage(&cache, urlToRetrieve);

        if(content != NULL) {
            cout << ANSI_GREEN << "Retrieved webpage:" << ANSI_RESET << endl << content  << endl;

            // Displaying the cache contents
            displayCache(&cache);
        }
    }

    // Cleaning up the cache (free memory) before exiting
    while(cache.head != NULL) evictLRUPage(&cache);

    return 0;
}
