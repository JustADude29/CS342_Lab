#ifndef Q1_HPP
#define Q1_HPP

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
#include <deque>
#include <map>
#include <unordered_map>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"

class HttpRequest{
private:
    int request_id;
    int website_id;
    int processing_time;

public:
    HttpRequest(int id, int websiteId, int processingTime)
        : request_id(id), website_id(websiteId), processing_time(processingTime) {}

    int getWebsiteId() const {
        return website_id;
    }

    int getProcessingTime() const {
        return processing_time;
    }

    int getID() const {
        return request_id;
    }

};

class Website{
private:
    int website_id;
    int owner_id;
    int allocated_bandwidth;
    int allocated_processing_power;
    std::vector<HttpRequest> request_queue;

public:
    Website(int id, int owner, int bandwidth, int processing_power)
        : website_id(id), owner_id(owner), allocated_bandwidth(bandwidth), allocated_processing_power(processing_power) {}

    int getID() const {
        return website_id;
    }

    int getOwner() const {
        return owner_id;
    }

    int getBandwidth() const{
        return allocated_bandwidth;
    }

    int getProcessingPower() const
    {
        return allocated_processing_power;
    }

    void enqueue_request(const HttpRequest &request)
    {
        request_queue.push_back(request);
    }

    std::vector<int> getRequestQueue()
    {
        int i;
        std::vector<int> d;
        for (i = 0; i < request_queue.size(); i++)
        {
            d.push_back(request_queue[i].getID());
        }
        return d;
    }

    std::vector<int> getWeightQueue()
    {
        int i;
        std::vector<int> d;
        for (i = 0; i < request_queue.size(); i++)
        {
            d.push_back(request_queue[i].getProcessingTime());
        }
        return d;
    }
};

class LoadBalancer
{
private:
    std::vector<Website> websites;
    std::unordered_map<int, int> reqToWebsite;
    std::vector<std::pair<double, int>> processing_times;
    std::unordered_map<int, double> real_processing_times;

public:
    double t_time = 0;
    void add_website(int website_id, int owner_id, int bandwidth, int processing_power)
    {
        Website website(website_id, owner_id, bandwidth, processing_power);
        websites.push_back(website);
        std::cout<< GREEN << "Website added with id: " << website_id << ", bandwidth: " << bandwidth << ", and processing power: "<< processing_power<<RESET<<std::endl;
    }

    void enqueue_request(const HttpRequest &request)
    {
        int websiteId = request.getWebsiteId();
        if (websiteId < 0 || websiteId >= websites.size())
        {
            std::cout << "Invalid website ID: " << websiteId << std::endl;
            return;
        }
        websites[websiteId].enqueue_request(request);
        std::cout<< BLUE << "Request added with id: " << request.getID() << ", to website: " << request.getWebsiteId() << ", and processing time: "<< request.getProcessingTime() <<RESET<<std::endl;
    }

    int getRequestsCount(){
        return processing_times.size();
    }

    void scheduler()
    {
        t_time = 0;

        std::vector<std::pair<int, double>> weights;
        double total = 0;
        for (auto &website : websites)
        {
            double w = website.getBandwidth() + website.getProcessingPower();
            total += w;
            int id = website.getID();
            weights.push_back({w, id});
        }
        std::unordered_map<int, double> band;
        for (auto &website : websites)
        {
            int id = website.getID();
            double w = website.getBandwidth() + website.getProcessingPower();
            band[id] = (w / total);
        }
        std::vector<std::pair<int, int>> virtual_time;
        std::unordered_map<int, double> processing_times_temp;

        for (auto &website : websites)
        {
            std::vector<int> reqs = website.getRequestQueue(); // contains request ids
            std::vector<int> times = website.getWeightQueue(); // contains processing times
            int id = website.getID();
            double t = 0;
            for (int i = 0; i < reqs.size(); i++)
            {
                double p_time = times[i];
                t += times[i] / band[id];
                processing_times_temp[reqs[i]] = t;
                real_processing_times[reqs[i]] = times[i];
                reqToWebsite[reqs[i]] = website.getID();
            }
        }

        for (auto i : processing_times_temp)
        {
            int f = i.first;
            double s = i.second;
            processing_times.push_back({s, f});
        }
        std::sort(processing_times.begin(), processing_times.end());
    }
    void dequeueRequest()
    {
        if (processing_times.size() > 0)
        {
            auto i = processing_times[0];

            int id = i.second;
            t_time += real_processing_times[id];
            std::cout << YELLOW << "Request with request ID " << BLUE << id << YELLOW << " is dequeued " << YELLOW << " By website "<< BLUE << reqToWebsite[id] << RESET << '\n';
            processing_times.erase(processing_times.begin());
        }
        else
        {
            std::cout << RED << "There are no requests to dequeue\n" << RESET;
        }
    }
};

#endif