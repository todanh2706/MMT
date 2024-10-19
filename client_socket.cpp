#include "client_socket.h"

// Connect to the server
bool ClientSocket::connectToServer(const char* serverIP, int port) {
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(port);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed" << std::endl;
        return false;
    }
    std::cout << "Connected to server!" << std::endl;
    return true;
}

// ************************** CODE FOR FUNCTION OF PROJECT **************************