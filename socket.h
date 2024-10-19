#ifndef SOCKET_H
#define SOCKET_H

#include <winsock2.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")  // Link with Winsock library

class Socket {
protected:
    SOCKET sock;
    WSADATA wsaData;

public:
    Socket();
    bool init();
    bool createSocket(int type = SOCK_STREAM);
    bool sendData(const char* data, int len);
    int receiveData(char* buffer, int len);
    void closeSocket();
    void cleanup();
    virtual ~Socket();
};

#endif  // SOCKET_H
