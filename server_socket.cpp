#include "server_socket.h"
#include <iostream>

Server::Server(int port) {
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Failed to initialize Winsock. Error: " << WSAGetLastError() << std::endl;
        exit(1);
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Could not create socket. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(1);
    }

    // Set up server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Bind
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        exit(1);
    }
}

Server::~Server() {
    closesocket(serverSocket);
    WSACleanup();
}

bool Server::startListening() {
    listen(serverSocket, 3);
    std::cout << "Waiting for incoming connections..." << std::endl;
    
    clientAddrLen = sizeof(struct sockaddr_in);
    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed. Error: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "Connection accepted." << std::endl;
    return true;
}

std::string Server::receiveMessage() {
    char buffer[1024];
    int recvSize = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (recvSize == SOCKET_ERROR) {
        std::cerr << "Receive failed. Error: " << WSAGetLastError() << std::endl;
        return "";
    }
    buffer[recvSize] = '\0';
    std::cout << "Received message: " << buffer << std::endl;
    return std::string(buffer);
}

void Server::stop() {
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
}
