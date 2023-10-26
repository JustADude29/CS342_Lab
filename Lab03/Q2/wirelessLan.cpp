#include "wirelessLan.hpp"
#include <cstdlib>
#include <cstring>
int main(int argc, char* argv[]) {
    if(argc == 2){
        if(strcmp(argv[1], "-f")==0){
            backoff_fail = true;
        }
        else{
            std::cerr<<"ERROR: correct format:\n./executable <flags>\nNo flag for default values\n-f for backoff fail(using exponentially increasing delay)\n-p <value> to set propagation time"<<endl;
            exit(1);
        }
    }
    else if(argc == 3){
        if(strcmp(argv[1], "-p")==0){
            propogation_time = atoi(argv[2]);
        }else{
            std::cerr<<"ERROR: correct format:\n./executable <flags>\nNo flag for default values\n-f for backoff fail(using exponentially increasing delay)\n-p <value> to set propagation time"<<endl;
            exit(1);
        }
    }
    else if(argc == 4){
        if(strcmp(argv[1], "-f")==0){
            backoff_fail = true;
        }
        else if(strcmp(argv[1], "-p")==0){
            propogation_time = atoi(argv[2]);
        }
        else{
            std::cerr<<"ERROR: correct format:\n./executable <flags>\nNo flag for default values\n-f for backoff fail(using exponentially increasing delay)\n-p <value> to set propagation time"<<endl;
            exit(1);
        }
        if(backoff_fail){
            if(strcmp(argv[2], "-p")==0){
                propogation_time = atoi(argv[3]);
            }else{
                std::cerr<<"ERROR: correct format:\n./executable <flags>\nNo flag for default values\n-f for backoff fail(using exponentially increasing delay)\n-p <value> to set propagation time"<<endl;
                exit(1);
            }
        }else{
            if(strcmp(argv[3], "-f")==0){
                backoff_fail = true;
            }else{
                std::cerr<<"ERROR: correct format:\n./executable <flags>\nNo flag for default values\n-f for backoff fail(using exponentially increasing delay)\n-p <value> to set propagation time"<<endl;
                exit(1);
            }
        }
    }
    if(backoff_fail){
        cout<<"Starting with exponentially increasing delay and failures possible\n";
    }else{
        cout<<"Starting with constant backoff interval and failures not possible\n";
    }
    cout<<"Propagation time set to "<<propogation_time<<endl;
    srand(static_cast<unsigned int>(time(nullptr)));
    cout<<"Enter the number of nodes (At any time during simulation enter -1 to exit): \n";
    int num_nodes; cin >> num_nodes;
    vector<double> transmission_times;

    for(int i=0;i<num_nodes;i++){
       cout<<"Enter the transmission time of node "<<i+1<<endl;
       double x; cin>>x;
       transmission_times.push_back(x);
    }

    for(int i=1;i<=num_nodes;i++) cout << "Node " << i << " has joined \n";

    vector<Node> nodes;

    startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_nodes; ++i) {
        Node node(i + 1,  backoff_max);
        nodes.push_back(node);
        thread(handleNode,node,transmission_times[i]).detach();
        // create thread for a new thread
    }

    handleChannel();
    return 0;
}