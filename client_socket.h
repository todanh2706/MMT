#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <winsock2.h>
#include <string>

class Client {
public:
    Client(const std::string& serverIP, int port);
    ~Client();
    bool connectToServer();
    bool sendShutdownRequest();
    bool sendRestartRequest();

private:
    std::string serverIP;
    int port;
    SOCKET clientSocket;
};

#endif // CLIENT_SOCKET_H
