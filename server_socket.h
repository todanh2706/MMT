#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "socket.h"

class ServerSocket : public Socket {
private:
    sockaddr_in serverAddr, clientAddr;
    SOCKET clientSocket;
    int clientSize;

public:
    ServerSocket();
    bool bindSocket(int port);
    bool listenForConnections();
    bool acceptConnection();
    void handleClient();

    // ************************** CODE FOR FUNCTION OF PROJECT **************************
    
};

#endif  // SERVER_SOCKET_H
