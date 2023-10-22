#include "client_header.h"



vector<string> splitString(string address, string delim = ":") {
    std::vector<string> res;
    size_t pos = 0;
    while ((pos = address.find(delim)) != string::npos) {
        string t = address.substr(0, pos);
        res.push_back(t);
        address.erase(0, pos + delim.length());
    }
    res.push_back(address);
    return res;
}

vector<string> getTrackerInfo(char* path) {
    int fd = open(path, O_RDONLY);
    std::vector<string> res;

    if (fd != -1) {
        char buffer[512];
        ssize_t bytesRead;
        string currentLine;

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
        cout << "Tracker Info file not found.\n";
        exit(-1);
    }

    return res;
}

void setChunkVector(string filename, ll l, ll r, bool isUpload) {
    if (isUpload) {
        std::vector<int> tmp(r - l + 1, 1);
        fileChunkInfo[filename] = tmp;
    } else {
        fileChunkInfo[filename][l] = 1;
    }
}

void processArgs(int argc, char *argv[]) {
    string peerInfo = argv[1];
    string trackerInfoFilename = argv[2];

    logFileName = peerInfo + "_log.txt";
    

    std::vector<string> peeraddress = splitString(peerInfo);
    peer_ip = peeraddress[0];
    peer_port = stoi(peeraddress[1]);

    char curDir[128];
    getcwd(curDir, 128);

    string path = string(curDir);
    path += "/" + trackerInfoFilename;
    vector<string> trackerInfo = getTrackerInfo(&path[0]);

    tracker1_ip = trackerInfo[0];
    tracker1_port = stoi(trackerInfo[1]);
    tracker2_ip = trackerInfo[2];
    tracker2_port = stoi(trackerInfo[3]);

    
}

int connectToTracker(int trackerNum, struct sockaddr_in &serv_addr, int sock) {
    char* curTrackIP;
    uint16_t curTrackPort;
    if (trackerNum == 1) {
        curTrackIP = &tracker1_ip[0];
        curTrackPort = tracker1_port;
    } else {
        curTrackIP = &tracker2_ip[0];
        curTrackPort = tracker2_port;
    }

    bool err = 0;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(curTrackPort);

    if (inet_pton(AF_INET, curTrackIP, &serv_addr.sin_addr) <= 0) {
        err = 1;
    }
    if (0 > connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
        err = 1;
    }
    if (err) {
        if (trackerNum == 1)
            return connectToTracker(2, serv_addr, sock);
        else
            return -1;
    }
    return 0;
}
