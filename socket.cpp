#include "socket.h"

// Constructor
Socket::Socket() : sock(INVALID_SOCKET) {}

// Initialize Winsock
bool Socket::init() {
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed" << std::endl;
        return false;
    }
    return true;
}

// Create a socket
bool Socket::createSocket(int type) {
    sock = socket(AF_INET, type, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        return false;
    }
    return true;
}

// Send data
bool Socket::sendData(const char* data, int len) {
    if (send(sock, data, len, 0) == SOCKET_ERROR) {
        std::cerr << "Send failed" << std::endl;
        return false;
    }
    return true;
}

// Receive data
int Socket::receiveData(char* buffer, int len) {
    int bytesReceived = recv(sock, buffer, len, 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Receive failed" << std::endl;
        return -1;
    }
    return bytesReceived;
}

// Close socket
void Socket::closeSocket() {
    closesocket(sock);
}

// Cleanup Winsock
void Socket::cleanup() {
    WSACleanup();
}

// Destructor
Socket::~Socket() {
    closeSocket();
    cleanup();
}
