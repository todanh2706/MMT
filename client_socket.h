#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include "socket.h"

class ClientSocket : public Socket {
private:
    sockaddr_in serverAddr;

public:
    bool connectToServer(const char* serverIP, int port);

    // ************************** CODE FOR FUNCTION OF PROJECT **************************
};

#endif  // CLIENT_SOCKET_H
