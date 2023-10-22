#include "client_header.h"

typedef struct peerFileDetails {
    std::string serverPeerIP;
    std::string filename;
    ll filesize;
} peerFileDetails;

typedef struct reqdChunkDetails {
   std:: string serverPeerIP;
   std:: string filename;
    ll chunkNum;
   std:: string destination;
} reqdChunkDetails;

void sendChunk(const char* filepath, int chunkNum, int client_socket) {
    int fd = open(filepath, O_RDONLY);
    if (fd == -1) {
        perror("[-]Error in opening file for reading");
        exit(1);
    }

    lseek(fd, chunkNum * FILE_SEGMENT_SZ, SEEK_SET);
    char buffer[FILE_SEGMENT_SZ] = {0};

    ssize_t count = read(fd, buffer, FILE_SEGMENT_SZ);
    if (count == -1) {
        perror("[-]Error in reading file");
        close(fd);
        exit(1);
    }

    if (send(client_socket, buffer, count, 0) == -1) {
      std::  perror("[-]Error in sending file.");
        close(fd);
        exit(1);
    }

    close(fd);
}

int writeChunk(int peersock, ll chunkNum, char* filepath) {
    int n, tot = 0;
    char buffer[FILE_SEGMENT_SZ];

    string content = "";
    while (tot < FILE_SEGMENT_SZ) {
        n = read(peersock, buffer, FILE_SEGMENT_SZ - 1);
        if (n <= 0) {
            break;
        }
        buffer[n] = 0;

        int fd = open(filepath, O_RDWR);
        if (fd == -1) {
            perror("[-]Error in opening file for writing");
            exit(1);
        }

        lseek(fd, chunkNum * FILE_SEGMENT_SZ + tot, SEEK_SET);
        ssize_t bytes_written = write(fd, buffer, n);
        if (bytes_written == -1) {
            perror("[-]Error in writing file");
            close(fd);
            exit(1);
        }

        close(fd);

        content += buffer;
        tot += n;
        memset(buffer, 0, FILE_SEGMENT_SZ);
    }

   std:: string hash = "";
    getStringHash(content, hash);
    hash.pop_back();
    hash.pop_back();
    if (hash != curFilePiecewiseHash[chunkNum]) {
        isCorruptedFile = true;
    }

  std::  string filename = splitString(std::string(filepath), "/").back();
    setChunkVector(filename, chunkNum, chunkNum, false);

    return 0;
}

void getChunkInfo(peerFileDetails* pf) {
   std:: vector<std::string> serverPeerAddress = splitString(std::string(pf->serverPeerIP), ":");
    string command = "get_chunk_vector$$" + std::string(pf->filename);
    string response = connectToPeer(&serverPeerAddress[0][0], &serverPeerAddress[1][0], command);

    for (size_t i = 0; i < curDownFileChunks.size(); i++) {
        if (response[i] == '1') {
            curDownFileChunks[i].push_back(string(pf->serverPeerIP));
        }
    }

    delete pf;
}

void getChunk(reqdChunkDetails* reqdChunk) {
    string filename = reqdChunk->filename;
  std::  vector<std::string> serverPeerIP = splitString(reqdChunk->serverPeerIP, ":");
    ll chunkNum = reqdChunk->chunkNum;
   std:: string destination = reqdChunk->destination;

  std::  string command = "get_chunk$$" + filename + "$$" + to_string(chunkNum) + "$$" + destination;
    connectToPeer(&serverPeerIP[0][0], &serverPeerIP[1][0], command);

    delete reqdChunk;
}

void piecewiseAlgo(vector<string> inpt, vector<string> peers) {
    ll filesize = stoll(peers.back());
    peers.pop_back();
    ll segments = filesize / FILE_SEGMENT_SZ + 1;
    curDownFileChunks.clear();
    curDownFileChunks.resize(segments);

 std::vector<thread> threads, threads2;

    for (size_t i = 0; i < peers.size(); i++) {
        peerFileDetails* pf = new peerFileDetails();
        pf->filename = inpt[2];
        pf->serverPeerIP = peers[i];
        pf->filesize = segments;
        threads.push_back(thread(getChunkInfo, pf));
    }
    for (auto it = threads.begin(); it != threads.end(); it++) {
        if (it->joinable()) it->join();
    }

    for (size_t i = 0; i < curDownFileChunks.size(); i++) {
        if (curDownFileChunks[i].size() == 0) {
            cout << "All parts of the file are not available." << std::endl;
            return;
        }
    }

    threads.clear();
    srand((unsigned)time(0));
    ll segmentsReceived = 0;

    string des_path = inpt[3] + "/" + inpt[2];
    int fd = open(&des_path[0], O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("[-]Error in opening file for writing");
        exit(1);
    }
    
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("[-]Error in getting file size");
        close(fd);
        exit(1);
    }

    if (file_size == 0) {
        string ss(filesize, '\0');
        ssize_t bytes_written = write(fd, ss.c_str(), ss.length());
        if (bytes_written == -1) {
          std::  perror("[-]Error in writing to the file");
            close(fd);
            exit(1);
        }
    }

    fileChunkInfo[inpt[2]].resize(segments, 0);
    isCorruptedFile = false;

    vector<int> tmp(segments, 0);
    fileChunkInfo[inpt[2]] = tmp;

    std::string peerToGetFilepath;

    while (segmentsReceived < segments) {
        ll randompiece;
        while (true) {
            randompiece = rand() % segments;
            if (fileChunkInfo[inpt[2]][randompiece] == 0) break;
        }
        ll peersWithThisPiece = curDownFileChunks[randompiece].size();
       std:: string randompeer = curDownFileChunks[randompiece][rand() % peersWithThisPiece];

        reqdChunkDetails* req = new reqdChunkDetails();
        req->filename = inpt[2];
        req->serverPeerIP = randompeer;
        req->chunkNum = randompiece;
        req->destination = inpt[3] + "/" + inpt[2];

        fileChunkInfo[inpt[2]][randompiece] = 1;

        threads2.push_back(thread(getChunk, req));
        segmentsReceived += 1;
        peerToGetFilepath = randompeer;
    }
    {auto it = threads2.begin();
while(it != threads2.end()){
	{
        if (it->joinable()) it->join();
    }
	it += 1;
}}

    if (!(isCorruptedFile)) {
        cout << "Download completed. No corruption detected." << endl;
    } else {
        cout << "Downloaded completed. The file may be corrupted." << endl;
    }
    downloadedFiles.insert({ inpt[2], inpt[1] });

    std::vector<std::string> serverAddress = splitString(peerToGetFilepath, ":");
    connectToPeer(&serverAddress[0][0], &serverAddress[1][0], "get_file_path$$" + inpt[2]);
}

void copyFile(const char* source, const char* destination) {
    int src_fd = open(source, O_RDONLY);
    if (src_fd == -1) {
        perror("[-]Error in opening source file for reading");
        exit(1);
    }

    int dest_fd = open(destination, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (dest_fd == -1) {
        perror("[-]Error in opening destination file for writing");
        close(src_fd);
        exit(1);
    }

    char buffer[FILE_SEGMENT_SZ] = {0};
    ssize_t n;

    while (0 < (n = read(src_fd, buffer, FILE_SEGMENT_SZ))) {
        if (write(dest_fd, buffer, n) == -1) {
            perror("[-]Error in writing to the destination file");
            close(src_fd);
            close(dest_fd);
            exit(1);
        }
    }

    close(src_fd);
    close(dest_fd);
}

int downloadFile(vector<string> inpt, int sock) {
    // inpt -  download_file​ <group_id> <file_name> <destination_path>
    if (inpt.size() != 4) {
        return 0;
    }
    string fileDetails = "";
    fileDetails += inpt[2] + "$$";
    fileDetails += inpt[3] + "$$";
    fileDetails += inpt[1];

    if (send(sock, &fileDetails[0], strlen(&fileDetails[0]), MSG_NOSIGNAL) == -1) {
        perror("[-]Error in sending file details");
        return -1;
    }

    char server_reply[524288] = {0};
    read(sock, server_reply, 524288);

    if (string(server_reply) == "File not found") {
        cout << server_reply << endl;
        return 0;
    }
    vector<string> peersWithFile = splitString(server_reply, "$$");

    char dum[5];
    strcpy(dum, "test");
    write(sock, dum, 5);

    bzero(server_reply, 524288);
    read(sock, server_reply, 524288);

   std:: vector<std::string> tmp = splitString(string(server_reply), "$$");
    curFilePiecewiseHash = tmp;

    piecewiseAlgo(inpt, peersWithFile);
    return 0;
}

int uploadFile(vector<string> inpt, int sock) {
    if (inpt.size() != 3) {
        return 0;
    }
    std::string fileDetails = "";
    char* filepath = &inpt[1][0];

    std::string filename = splitString(string(filepath), "/").back();

    if (isUploaded[inpt[2]].find(filename) != isUploaded[inpt[2]].end()) {
        cout << "File already uploaded" << endl;
        if (send(sock, "error", 5, MSG_NOSIGNAL) == -1) {
            perror("[-]Error in sending upload error");
            return -1;
        }
        return 0;
    } else {
        isUploaded[inpt[2]][filename] = true;
        fileToFilePath[filename] = string(filepath);
    }

    std::string piecewiseHash = getHash(filepath);
    if (piecewiseHash == "$") return 0;

    std::string filehash = getFileHash(filepath);
    string filesize = to_string(file_size(filepath));

    fileDetails += string(filepath) + "$$";
    fileDetails += string(peer_ip) + ":" + to_string(peer_port) + "$$";
    fileDetails += filesize + "$$";
    fileDetails += filehash + "$$";
    fileDetails += piecewiseHash;

    if (send(sock, &fileDetails[0], strlen(&fileDetails[0]), MSG_NOSIGNAL) == -1) {
        perror("[-]Error in sending file details");
        return -1;
    }

    char server_reply[10240] = {0};
    read(sock, server_reply, 10240);
    std::cout << server_reply << std::endl;

    setChunkVector(filename, 0, stoll(filesize) / FILE_SEGMENT_SZ + 1, true);
    return 0;
}