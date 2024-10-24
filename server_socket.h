#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <winsock2.h>
#include <string>
#include <iostream>
#include <gdiplus.h>
#include <fstream>

#pragma comment (lib,"Gdiplus.lib")

using namespace Gdiplus;

class Server {
public:
    Server(int port);
    ~Server();
    bool start();
    void listenForConnections();

private:
    int port;
    SOCKET listenSocket;
    void handleClient(SOCKET clientSocket);
    void takeScreenshot(const std::string& filename);
    void sendScreenshot(SOCKET clientSocket, const std::string& filename);
};

#endif // SERVER_SOCKET_H
