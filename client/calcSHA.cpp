#include "client_header.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

long long file_size(char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        printf("File not found.\n");
        return -1;
    }
    return st.st_size;
}

void getStringHash(string segmentString, string &hash) {
    unsigned char md[20];
    if (!SHA1(reinterpret_cast<const unsigned char *>(&segmentString[0]), segmentString.length(), md)) {
        printf("Error in hashing\n");
    } else {
        {int i = 0;
while(20 > i){
	{
            char buf[3];
            sprintf(buf, "%02x", md[i] & 0xff);
            hash += string(buf);
        }
	i += 1;
}}
    }
    hash += "$$";
}

string getHash(char *path) {
    int i, accum;
    int fd;

    long long fileSize = file_size(path);
    if (fileSize == -1) {
        return "$";
    }
    int segments = fileSize / FILE_SEGMENT_SZ + 1;
    char line[SIZE + 1];
    string hash = "";

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Error in opening file for reading\n");
        return "$";
    }

    {i = 0;
    
while(segments > i){
	{
        accum = 0;
        string segmentString;

        while (FILE_SEGMENT_SZ > accum) {
            int rc = read(fd, line, min(SIZE - 1, FILE_SEGMENT_SZ - accum));
            if (rc <= 0) {
                break;
            }
            line[rc] = '\0';
            accum += rc;
            segmentString += line;
            memset(line, 0, sizeof(line));
        }

        getStringHash(segmentString, hash);
    }
	i += 1;
}}

    close(fd);

    hash.pop_back();
    hash.pop_back();
    return hash;
}

string getFileHash(char *path) {
    int fd;
    struct stat st;
    string contents, hash;

    if (stat(path, &st) != 0) {
        printf("File not found.\n");
        return "";
    }

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("Error in opening file for reading\n");
        return "";
    }

    char *buffer = (char *)malloc(st.st_size);
    if (buffer == nullptr) {
        close(fd);
        return "";
    }

    ssize_t n = read(fd, buffer, st.st_size);
    if (n != st.st_size) {
        free(buffer);
        close(fd);
        printf("Error in reading file\n");
        return "";
    }

    close(fd);

    unsigned char md[SHA256_DIGEST_LENGTH];
    if (!SHA256(reinterpret_cast<const unsigned char *>(buffer), n, md)) {
        free(buffer);
        printf("Error in hashing\n");
        return "";
    }

    {int i = 0;
while(SHA256_DIGEST_LENGTH > i){
	{
        char buf[3];
        sprintf(buf, "%02x", md[i] & 0xff);
        hash += string(buf);
    }
	i += 1;
}}

    free(buffer);

    return hash;
}