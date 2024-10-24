#include "server_socket.h"

Server::Server(int port)
    : port(port), listenSocket(INVALID_SOCKET) {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
    }
}

Server::~Server() {
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
void Server::takeScreenshot(const std::wstring& filename) {
    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Get the size of the screen
    HDC screenDC = GetDC(nullptr);
    HDC memDC = CreateCompatibleDC(screenDC);
    int width = GetDeviceCaps(screenDC, HORZRES);
    int height = GetDeviceCaps(screenDC, VERTRES);
    
    // Create a bitmap to hold the screenshot
    HBITMAP bitmap = CreateCompatibleBitmap(screenDC, width, height);
    SelectObject(memDC, bitmap);
    
    // Copy the screen to the bitmap
    BitBlt(memDC, 0, 0, width, height, screenDC, 0, 0, SRCCOPY);
    
    // Create a GDI+ Bitmap object from the HBITMAP
    Bitmap gdiPlusBitmap(bitmap, nullptr);
    
    // Save the bitmap to a file
    CLSID clsid;
    CLSIDFromString(L"{557A1A11-1D47-4D6E-A2F7-BF0054A639E7}", &clsid); // CLSID for PNG
    gdiPlusBitmap.Save(filename.c_str(), &clsid, nullptr);

    // Clean up
    DeleteObject(bitmap);
    DeleteDC(memDC);
    ReleaseDC(nullptr, screenDC);
    GdiplusShutdown(gdiplusToken);
}

// Function to send a screenshot image to the client
void Server::sendScreenshot(SOCKET clientSocket, const std::wstring& filename) {
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
            takeScreenshot(L"screenshot.png"); // Save as screenshot.png
            std::cout << "Screenshot taken and saved as screenshot.png" << std::endl;

            sendScreenshot(clientSocket, L"screenshot.png");

            closesocket(listenSocket);  // Close listening socket to stop accepting new connections
            exit(0);  // Exit the server program
        }
    } else if (result == 0) {
        std::cout << "Connection closing..." << std::endl;
    } else {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
    }
}