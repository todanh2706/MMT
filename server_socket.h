#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include <winsock2.h>
#include <string>

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
};

#endif // SERVER_SOCKET_H
