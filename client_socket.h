#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")  // Winsock Library

class Client {
private:
    WSADATA wsa;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;

public:
    Client(const std::string& serverIP, int port);
    ~Client();
    bool connectToServer();
    bool sendMessage(const std::string& message);
    void cleanup();
};

#endif // CLIENT_SOCKET_H
