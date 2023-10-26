#include <bits/stdc++.h>
#include <vector>
#include <limits>
#include <queue>
#include <map>

class Router {
private:
    int noRouters;
    int router_id;
    std::vector<Router*> neighbours; // Stores just next neighbours of router
    std::map<Router*, std::pair<int, Router*>> routing_table; // Maps other routers to the cost and next hop router

public:

    Router(){}

    int getRouterId() const { //provides Id of the router
        return router_id;
    }
    
    void setRouterValues(int id, int num, Router* router) //initialising router variables
    {
        router_id = id;
        noRouters = num;
        auto iterator = router;
        for(int i=0; i<num; i++)
        {
            if(i!=id)
            routing_table[iterator] = std::make_pair(INT_MAX, this);
            iterator++;
        }
    }

    void addNeighbor(Router* neighbour, int cost) { //Adding new neighbours to router
        neighbours.push_back(neighbour);
        routing_table[neighbour] = std::make_pair(cost, neighbour); //Updating cost for the neighbours
    }

    void updateRoutingTable() {
        // Dijkstra's algorithm to calculate shortest paths
        std::priority_queue<std::pair<std::pair<int, Router*>, Router*>, std::vector<std::pair<std::pair<int, Router*>, Router*>>, std::greater<std::pair<std::pair<int, Router*>, Router*>>> pq;
        std::map<Router*, std::pair<int, Router*>> distance;//maps other router to the cost and nexthop router

        for(auto it: routing_table) //initialising distances with infinity
        {
            distance[it.first] = {(it.second).first, (it.second).second};
        }

        for (auto& neighbor : neighbours) { //initialising neighbours in priority queue and distance vector
            distance[neighbor] = std::make_pair(routing_table[neighbor].first, neighbor);
            pq.push({{distance[neighbor].first, neighbor}, neighbor});
        }

        while (!pq.empty()) {
            auto tp = pq.top();
            std::pair<int, Router*> distAndNextHop = tp.first;
            int dist = distAndNextHop.first;
            Router* nextHop = distAndNextHop.second;
            Router* current = tp.second;
            pq.pop();

            for (auto& neighbor : current->neighbours) {
                int new_dist = dist + current->routing_table[neighbor].first;
                if (new_dist < distance[neighbor].first) {
                    distance[neighbor] = std::make_pair(new_dist, nextHop);
                    pq.push({{new_dist, nextHop}, neighbor});
                }
            }
        }

        for (auto x : distance) {   //Updating router table from calculated distance vector
            Router* neighbor = x.first;
            if(neighbor->getRouterId()==this->getRouterId())
                continue;
            std::pair<int, Router*> distAndNextHop = x.second;
            routing_table[neighbor] = distAndNextHop;
        }
    }

    void printRoutingTable() const { //printing routing table for given router
        std::cout << "Routing table for Router " << router_id << ":\n";
        for (const auto x : routing_table) {
            Router* neighbor = x.first;
            std::pair<int, Router*> costAndNextHop = x.second;
            if(costAndNextHop.first!=INT_MAX)
            std::cout << "To Router "<< neighbor->getRouterId()<< ": Cost " << costAndNextHop.first
                      << ", Next Hop " << costAndNextHop.second->getRouterId() << "\n";
            else
            std::cout << "To Router "<< neighbor->getRouterId()<< ": Cost " << "INF"
                      << ", Next Hop " << "DNE" << "\n";
        }
        std::cout << "-----------------------------\n";
    }
};

int main() {
    // Create routers
    int N, M;
    std::cout<<"Number of Routers:";
    std::cin>>N;
    Router router[N];
    for(int i=0; i<N; i++)
        router[i].setRouterValues(i, N, router);

    // Connect routers
    std::cout<<"Number of Connections:";
    std::cin>>M;
    std::cout<<"Give Connections and Cost as {Node1 Node2 cost}:";
    std::cout<<std::endl;
    for(int i=0;i<M;i++)
    {
        int a,b,cost;
        std::cin>>a>>b>>cost;
        router[a].addNeighbor(&router[b], cost);
        router[b].addNeighbor(&router[a], cost);
    }

    //Update and print routing tables
    for(int i=0; i<N; i++)
        router[i].updateRoutingTable();

    for(int i=0; i<N; i++)
        router[i].printRoutingTable();
    return 0;
}
