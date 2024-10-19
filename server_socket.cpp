#include "server_socket.h"

// Constructor
ServerSocket::ServerSocket() : clientSize(sizeof(clientAddr)), clientSocket(INVALID_SOCKET) {}

// Bind the socket
bool ServerSocket::bindSocket(int port) {
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Binding socket failed" << std::endl;
        return false;
    }
    return true;
}

// Listen for incoming connections
bool ServerSocket::listenForConnections() {
    if (listen(sock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listening failed" << std::endl;
        return false;
    }
    std::cout << "Server is listening..." << std::endl;
    return true;
}

// Accept a connection
bool ServerSocket::acceptConnection() {
    clientSocket = accept(sock, (sockaddr*)&clientAddr, &clientSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accepting connection failed" << std::endl;
        return false;
    }
    std::cout << "Client connected!" << std::endl;
    return true;
}

// Handle client communication
void ServerSocket::handleClient() {
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0) {
        std::cout << "Received: " << std::string(buffer, 0, bytesReceived) << std::endl;
        send(clientSocket, buffer, bytesReceived, 0);  // Echo back
    }
    closesocket(clientSocket);
}

// ************************** CODE FOR FUNCTION OF PROJECT **************************