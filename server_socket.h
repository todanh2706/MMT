

// server_socket.h
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H


#include <winsock2.h>
#include <windows.h>
#include <winerror.h>
#include <tlhelp32.h> //using for list app
#include <string>
#include <iostream> 
#include <thread> //using for keyLogger
#include <mutex> //using for keyLogger
#include <gdiplus.h>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <map> //using for keyLogger
#include <tchar.h> //using for open App
// Ensure NO_ERROR is defined
#ifndef NO_ERROR
#define NO_ERROR 0
#endif

using namespace Gdiplus;
#pragma comment (lib,"gdiplus.lib")



class Server {
public:
    Server(int port);
    ~Server();
    bool start();
    void listenForConnections();
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
    
    //shutdown and restart
    void shutdownServer();
    void restartServer();
    //keylogger
    bool isLogging = false;
    std::thread keyLoggerThread;
    std::mutex logMutex;
    void startKeyLogger();
    void stopKeyLogger(SOCKET);
    void keyLogger();
    //List/Turn on/Turn off App
    bool hasVisibleWindow(DWORD processID);
    void ListApplications(SOCKET);
    void openApp(const std::string& appName, bool status, SOCKET clientSocket);
    //List/Turn on/Turn off services
    

};

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

#endif // SERVER_SOCKET_H