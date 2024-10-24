#include "server_socket.h"

Server::Server(int port)
    : port(port), listenSocket(INVALID_SOCKET) {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
    }

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
}

Server::~Server() {
    // Clean up GDI+
    GdiplusShutdown(gdiplusToken);
    
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
    }
    WSACleanup();
}

bool Server::start() {
    // Create socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return false;
    }

    // Setup sockaddr_in structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    // Bind socket
    int result = bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return false;
    }

    // Listen for incoming connections
    result = listen(listenSocket, SOMAXCONN);
    if (result == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return false;
    }

    std::cout << "Server listening on port " << port << std::endl;
    return true;
}

void Server::listenForConnections() {
    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        // Handle the connected client synchronously
        handleClient(clientSocket);

        closesocket(clientSocket);
    }
}

// Function to take a screenshot and save it as a PNG file
void Server::takeScreenshot(const std::string& filename) {
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Get the screen dimensions
    HDC screenDC = GetDC(NULL);
    HDC memoryDC = CreateCompatibleDC(screenDC);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    // Create a compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(screenDC, width, height);
    SelectObject(memoryDC, hBitmap);

    // BitBlt to copy the screen to the bitmap
    BitBlt(memoryDC, 0, 0, width, height, screenDC, 0, 0, SRCCOPY);

    // Create GDI+ Image from the bitmap
    Bitmap bitmap(hBitmap, NULL);
    CLSID clsid;
    HRESULT result = CLSIDFromString(L"{557CC3D0-1A3A-11D1-AD6E-00A0C9138F8C}", &clsid); // CLSID for PNG
    if (FAILED(result)) {
        std::cerr << "Failed to get CLSID for PNG." << std::endl;
        return; // Handle error appropriately
    }

    // Save the image as a PNG file
    WCHAR wFilename[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, wFilename, MAX_PATH);
    bitmap.Save(wFilename, &clsid, NULL);

    // Clean up
    DeleteObject(hBitmap);
    DeleteDC(memoryDC);
    ReleaseDC(NULL, screenDC);
    GdiplusShutdown(gdiplusToken);
}

// Function to send a screenshot image to the client
// Function to send a screenshot image to the client
void Server::sendScreenshot(SOCKET clientSocket, const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open screenshot file: " << filename << std::endl;
        return;
    }

    // Get file size
    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Send file size first
    send(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);

    // Buffer for sending the file
    char buffer[4096];
    while (fileSize > 0) {
        file.read(buffer, sizeof(buffer));
        std::streamsize bytesRead = file.gcount();
        send(clientSocket, buffer, bytesRead, 0);
        fileSize -= bytesRead;
    }

    std::cout << "Screenshot sent to client successfully!" << std::endl;
}

void Server::handleClient(SOCKET clientSocket) {
    char recvbuf[512];
    int recvbuflen = 512;

    // Receive data from the client
    int result = recv(clientSocket, recvbuf, recvbuflen, 0);
    if (result > 0) {
        std::string receivedMessage(recvbuf, result);
        std::cout << "Received message: " << receivedMessage << std::endl;

        if (receivedMessage == "shutdown") {
            std::cout << "Shutdown command received. Server is shutting down..." << std::endl;
            closesocket(listenSocket);  // Close listening socket to stop accepting new connections
            exit(0);  // Exit the server program
        }
        else if (receivedMessage == "screenshot")
        {
            std::cout << "Screenshot command received. Taking a screenshot..." << std::endl;
            // Take a screenshot and save it
            takeScreenshot("screenshot.png"); // Save as screenshot.png
            std::cout << "Screenshot taken and saved as screenshot.png" << std::endl;

            // Send the screenshot back to the client
            sendScreenshot(clientSocket, "screenshot.png");

            closesocket(listenSocket);  // Close listening socket to stop accepting new connections
            exit(0);  // Exit the server program
        }
    } else if (result == 0) {
        std::cout << "Connection closing..." << std::endl;
    } else {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
    }
}