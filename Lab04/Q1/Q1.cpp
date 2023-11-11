#include "Q1.hpp"
#include <chrono>
#include <cstdio>
#include <thread>
#include <iostream>
#include <ostream>
#include <unistd.h>
#include <sys/wait.h>


void clear_terminal() {
    pid_t pid = fork();
    if(pid==0){
        execlp("clear", "clear", NULL);
        exit(0);
    }
    wait(NULL);
}

int main(int argc, char* argv[]){
    LoadBalancer _loadBalancer;
    int z;
    std::chrono::duration<double> durationInSeconds(0.5);

    int testCase;

    while(1){
        clear_terminal();
        std::cout<< BLUE << "\nEnter Test Case number:\n" << RESET;
        std::cout << YELLOW << "\nTest Case 1: Basic Load Balancing" << RESET << std::endl;
        std::cout << YELLOW << "Test Case 2: Differential Bandwidth Allocation" << RESET << std::endl;
        std::cout << YELLOW << "Test Case 3: Differential Processing Power Allocation " << RESET << std::endl;
        std::cout << YELLOW << "Test Case 4: Equal Allocations " << RESET << std::endl;
        std::cout << YELLOW << "Test Case 5: Large Number of Requests" << RESET << std::endl;
        std::cout << YELLOW << "Test Case 6: Empty Queues" << RESET << std::endl;
        std::cout << YELLOW << "Test Case 7: Unequal Bandwidth and Processing Power" << RESET << std::endl;
        std::cout << YELLOW << "Test Case 8: Edge Case - Single Website" << RESET << std::endl;
        std::cout << YELLOW << "-1 to exit" << RESET << std::endl;
        
        std::cin>>testCase;
        
        switch (testCase) {
            case 1:
            // Test Case 1: Basic Load Balancing
                std::cout << "\nBasic Load Balancing" << std::endl;
                _loadBalancer.add_website(0, 0, 5, 5);
                _loadBalancer.add_website(1, 1, 15, 5);

                _loadBalancer.enqueue_request(HttpRequest(0, 1, 1));
                _loadBalancer.enqueue_request(HttpRequest(1, 0, 1));
                _loadBalancer.enqueue_request(HttpRequest(2, 1, 1));
                _loadBalancer.scheduler();
                z = _loadBalancer.getRequestsCount();
                for (int i = 0; i < z; i++)
                {
                    _loadBalancer.dequeueRequest();
                }
                break;
        
            case 2:
                _loadBalancer = LoadBalancer(); // Reset the load balancer for the next test case.
                // Test Case 2: Differential Bandwidth Allocation
                std::cout << "\nDifferential Bandwidth Allocation" << std::endl;

                _loadBalancer.add_website(0, 0, 10, 4);
                _loadBalancer.add_website(1, 1, 2, 4);

                _loadBalancer.enqueue_request(HttpRequest(0, 0, 3));
                _loadBalancer.enqueue_request(HttpRequest(1, 1, 2));
                _loadBalancer.scheduler();
                z = _loadBalancer.getRequestsCount();
                for (int i = 0; i < z; i++)
                {
                    _loadBalancer.dequeueRequest();
                }
                break;

            case 3:
                _loadBalancer = LoadBalancer();
                // Test Case 3: Differential Processing Power Allocation
                std::cout << "\nDifferential Processing Power Allocation " << std::endl;
                _loadBalancer.add_website(0, 0, 5, 5);
                _loadBalancer.add_website(1, 1, 5, 10);

                _loadBalancer.enqueue_request(HttpRequest(0, 0, 3));
                _loadBalancer.enqueue_request(HttpRequest(1, 1, 2));
                _loadBalancer.scheduler();
                z = _loadBalancer.getRequestsCount();
                for (int i = 0; i < z; i++)
                {
                    _loadBalancer.dequeueRequest();
                }
                break;

            case 4:
                _loadBalancer = LoadBalancer();
                //  Test Case 4: Equal Allocations
                std::cout << "\nEqual Allocations " << std::endl;
                _loadBalancer.add_website(0, 0, 5, 5);
                _loadBalancer.add_website(1, 1, 5, 5);

                _loadBalancer.enqueue_request(HttpRequest(0, 0, 2));
                _loadBalancer.enqueue_request(HttpRequest(1, 1, 2));
                _loadBalancer.enqueue_request(HttpRequest(2, 0, 2));
                _loadBalancer.enqueue_request(HttpRequest(3, 1, 2));
                _loadBalancer.enqueue_request(HttpRequest(4, 0, 2));
                _loadBalancer.enqueue_request(HttpRequest(5, 1, 2));
                _loadBalancer.scheduler();
                z = _loadBalancer.getRequestsCount();
                for (int i = 0; i < z; i++)
                {
                    _loadBalancer.dequeueRequest();
                }
                break;

            case 5:
                _loadBalancer = LoadBalancer();
                //  Test Case 5: Large Number of Requests
                std::cout << "\nLarge Number of Requests" << std::endl;
                _loadBalancer.add_website(0, 0, 5, 10);
                _loadBalancer.add_website(1, 1, 10, 5);

                for (int i = 0; i < 100; ++i)
                {
                    _loadBalancer.enqueue_request(HttpRequest(i, i % 2, rand() % 10));
                }
                _loadBalancer.scheduler();
                z = _loadBalancer.getRequestsCount();
                for (int i = 0; i < z; i++)
                {
                    _loadBalancer.dequeueRequest();
                }
                break;

            case 6:
                _loadBalancer = LoadBalancer();
                //  Test Case 6: Empty Queues
                std::cout << "\nEmpty Queues" << std::endl;
                _loadBalancer.scheduler();
                z = 11;
                for (int i = 0; i < z; i++)
                {
                    _loadBalancer.dequeueRequest();
                }
                break;

            case 7:
                _loadBalancer = LoadBalancer();
                // Test Case 7: Unequal Bandwidth and Processing Power
                std::cout << "\nUnequal Bandwidth and Processing Power" << std::endl;
                _loadBalancer.add_website(0, 0, 5, 20);
                _loadBalancer.add_website(1, 1, 10, 5);

                _loadBalancer.enqueue_request(HttpRequest(0, 0, 3));
                _loadBalancer.enqueue_request(HttpRequest(1, 1, 2));
                _loadBalancer.scheduler();
                z = _loadBalancer.getRequestsCount();
                for (int i = 0; i < z; i++)
                {
                    _loadBalancer.dequeueRequest();
                }
                break;
                
            case 8:
                _loadBalancer = LoadBalancer();
                // Test Case 8: Edge Case - Single Website
                std::cout << "\nEdge Case - Single Website" << std::endl;
                _loadBalancer.add_website(0, 0, 5, 5);

                _loadBalancer.enqueue_request(HttpRequest(0, 0, 3));
                _loadBalancer.enqueue_request(HttpRequest(1, 0, 2));
                _loadBalancer.scheduler();
                z = _loadBalancer.getRequestsCount();
                for (int i = 0; i < z; i++)
                {
                    _loadBalancer.dequeueRequest();
                }
                break;
            
            case -1:
                std::cout<< GREEN << "Exiting" << RESET;
                for(int i = 0; i < 3; i++) {
                    std::this_thread::sleep_for(durationInSeconds);
                    std::cout<< GREEN "." << RESET;std::flush(std::cout);
                }
                std::this_thread::sleep_for(durationInSeconds);
                clear_terminal();
                exit(0);
                break;

            default:
                std::cout<<"Choose case from 1-8 or -1 to exit" <<std::endl;
        }
        std::cout<<"Press any key to continue\n";
        char temp;
        std::cin>>temp;
    }

}