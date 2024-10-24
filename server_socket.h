#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")  // Winsock Library

class Server {
private:
    WSADATA wsa;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen;

public:
    Server(int port);
    ~Server();
    bool startListening();
    std::string receiveMessage();
    void stop();
};

#endif // SERVER_SOCKET_H
