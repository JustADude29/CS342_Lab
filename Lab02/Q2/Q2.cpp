#include <iostream>
#include <string>
#include <list>
#include <unordered_map>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

using namespace std;

// Define a structure to represent a web page
struct WebPage {
    string url;
    string content;
};

class WebCache {
public:
    WebCache(int capacity) : capacity(capacity) {}

    string getWebPage(const string& url) {
        // Check if the page is in the cache
        if (cacheMap.find(url) != cacheMap.end()) {
            // Move the accessed page to the front of the list (MRU position)
            moveToMostRecent(url);
            return cacheMap[url]->content;
        } else {
            // Page not in cache, fetch it via HTTP GET request
            string content = fetchWebPage(url);

            // Add the fetched page to the cache (potentially evicting LRU page)
            addWebPage(url, content);

            return content;
        }
    }

    void displayCache() {
        for (const auto& page : cacheList) {
            cout << page.url << endl;
        }
    }

private:
    int capacity;
    list<WebPage> cacheList;
    unordered_map<string, list<WebPage>::iterator> cacheMap;

    // Fetch a web page using HTTP GET request
    string fetchWebPage(const string& url) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            cerr << "Error opening socket" << endl;
            exit(1);
        }

        struct hostent* server = gethostbyname(url.c_str());
        if (server == NULL) {
            cerr << "Error, no such host" << endl;
            exit(1);
        }

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(80);
        memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);

        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            cerr << "Error connecting to the server" << endl;
            exit(1);
        }

        string request = "GET / HTTP/1.1\r\nHost: " + url + "\r\n\r\n";
        send(sockfd, request.c_str(), request.length(), 0);

        string response;
        char buffer[4096];
        ssize_t bytes_received;
        while ((bytes_received = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
            response.append(buffer, bytes_received);
        }

        close(sockfd);

        return response;
    }

    // Add a web page to the cache
    void addWebPage(const string& url, const string& content) {
        if (cacheList.size() >= capacity) {
            // If the cache is full, evict the least recently used page
            cacheMap.erase(cacheList.back().url);
            cacheList.pop_back();
        }

        // Add the new page to the front of the list (MRU position)
        cacheList.push_front({url, content});
        cacheMap[url] = cacheList.begin();
    }

    // Move the accessed page to the front of the list (MRU position)
    void moveToMostRecent(const string& url) {
        auto it = cacheMap[url];
        cacheList.splice(cacheList.begin(), cacheList, it);
    }
};

int main() {
    WebCache cache(5);

    // Test the cache with example web page URLs
    string url1 = "http://quietsilverfreshmelody.neverssl.com/online/";
    string url2 = "https://www.example.com";
    string url3 = "https://www.google.com";

    cout << "Fetching and caching web pages..." << endl;
    string content1 = cache.getWebPage(url1);
    string content2 = cache.getWebPage(url2);
    string content3 = cache.getWebPage(url3);

    cout << "Displaying cached web pages:" << endl;
    cache.displayCache();

    return 0;
}
