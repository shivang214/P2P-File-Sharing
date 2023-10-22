#include "tracker_header.h"

string logFileName, tracker1_ip, tracker2_ip, curTrackerIP,
            seederFileName;
uint16_t tracker1_port, tracker2_port, curTrackerPort;
std::unordered_map<string, string> loginCreds;
std::unordered_map<string, bool> isLoggedIn;
std::unordered_map<string, unordered_map<string, set<string>>> seederList; // groupid -> {map of filenames -> peer address}
std::unordered_map<string, string> fileSize;
std::unordered_map<string, string> grpAdmins;
std::vector<string> allGroups;
std::unordered_map<string, set<string>> groupMembers;
std::unordered_map<string, set<string>> grpPendngRequests;
std::unordered_map<string, string> unameToPort;
std::unordered_map<string, string> piecewiseHash; 

int main(int argc, char *argv[]){ 

    if(argc != 3){
        std::cout << "Give arguments as <tracker info file name> and <tracker_number>\n";
        return -1;
    }

    processArgs(argc, argv);

    int tracker_socket; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    pthread_t  exitDetectionThreadId;
       
    if ((tracker_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
     
       
    if (setsockopt(tracker_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_port = htons(curTrackerPort); 

    if(inet_pton(AF_INET, &curTrackerIP[0], &address.sin_addr)<=0)  { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
       
    if (0 > bind(tracker_socket, (SA *)&address,  sizeof(address))) { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
 

    if (0 > listen(tracker_socket, 3)) { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
   

    std::vector<thread> threadVector;

    if(pthread_create(&exitDetectionThreadId, NULL, check_input, NULL) == -1){
        perror("pthread"); 
        exit(EXIT_FAILURE); 
    }

    while(true){
        int client_socket;

        if(0 > (client_socket = accept(tracker_socket, (SA *)&address, (socklen_t *)&addrlen))){
            perror("Acceptance error");
             
        }
      

        threadVector.push_back(thread(handle_connection, client_socket));
    }
    {auto i=threadVector.begin();
while(i!=threadVector.end()){
	{
        if(i->joinable()) i->join();
    }
	i += 1;
}}

 
    return 0; 
} 
