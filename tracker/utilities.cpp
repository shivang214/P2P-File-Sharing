#include "tracker_header.h"

void clearLog() {
    int fd = open(logFileName.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    
    if (fd != -1) {
        close(fd);
    }
}

 

bool pathExists(const string &s) {
    struct stat buffer;
    return (stat(s.c_str(), &buffer) == 0);
}

std::vector<std::string> splitString(string str, string delim) {
    std::vector<std::string> res;

    size_t pos = 0;
    while ((pos = str.find(delim)) != string::npos) {
        std::string t = str.substr(0, pos);
        res.push_back(t);
        str.erase(0, pos + delim.length());
    }
    res.push_back(str);

    return res;
}
 
void* check_input(void* arg) {
    while (true) {
        string inputline;
        getline(cin, inputline);
        if (inputline == "quit") {
            exit(0);
        }
    }
}

std::vector<string> getTrackerInfo(char* path) {
    int fd = open(path, O_RDONLY);
    std::vector<string> res;

    if (fd != -1) {
        char buffer[512];
        ssize_t bytesRead;
        std::string currentLine;

        while (0 < (bytesRead = read(fd, buffer, sizeof(buffer)))) {
            {ssize_t i = 0;
while(bytesRead > i){
	{
                if (buffer[i] == '\n' || buffer[i] == '\r' || buffer[i] == ' ' || buffer[i] == '\t') {
                    if (!currentLine.empty()) {
                        res.push_back(currentLine);
                        currentLine.clear();
                    }
                } else {
                    currentLine += buffer[i];
                }
            }
	i += 1;
}}
        }

        if (!currentLine.empty()) {
            res.push_back(currentLine);
        }

        close(fd);
    } else {
        std::cout << "Tracker Info file not found.\n";
        exit(-1);
    }

    return res;
}

void processArgs(int argc, char *argv[]) {
    logFileName = "trackerlog" + string(argv[2]) + ".txt";
    clearLog();

    std::vector<std::string> trackeraddress = getTrackerInfo(argv[1]);
    if (string(argv[2]) == "1") {
        tracker1_ip = trackeraddress[0];
        tracker1_port = stoi(trackeraddress[1]);
        curTrackerIP = tracker1_ip;
        curTrackerPort = tracker1_port;
    } else {
        tracker2_ip = trackeraddress[2];
        tracker2_port = stoi(trackeraddress[3]);
        curTrackerIP = tracker2_ip;
        curTrackerPort = tracker2_port;
    }

 
}
