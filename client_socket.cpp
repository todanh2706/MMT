#include "client_socket.h"
#include <iostream>

Client::Client(const std::string& serverIP, int port) {
    // Initialize Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "Failed to initialize Winsock. Error: " << WSAGetLastError() << std::endl;
        exit(1);
    }

    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Could not create socket. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(1);
    }

    // Set up server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    serverAddr.sin_port = htons(port);
}

Client::~Client() {
    closesocket(clientSocket);
    WSACleanup();
}

bool Client::connectToServer() {
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed. Error: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "Connected to server." << std::endl;
    return true;
}

bool Client::sendMessage(const std::string& message) {
    if (send(clientSocket, message.c_str(), message.size(), 0) < 0) {
        std::cerr << "Send failed. Error: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "Message sent: " << message << std::endl;
    return true;
}

void Client::cleanup() {
    closesocket(clientSocket);
    WSACleanup();
}
