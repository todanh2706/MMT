#include "client_socket.h"
#include "server_socket.h"
// #include "GmailClient.h"
#include <iostream>
#include <sstream>

void handleDemand(std::string demands[10], std::string demand)
{
    std::stringstream ss(demand.c_str());

    int i = 0;
    while (ss >> demands[i++]){}
    i--;
    demands[i] = "end!";
}

int main(int argc, char* argv[]) {
    if (std::string(argv[1]) == "server") {
        // Start server
        Server server(54000);
        if (server.start()) {
            server.listenForConnections();  // Synchronously listen for connections
        }
    } else if (std::string(argv[1]) == "client") {
        // Start client
        Client client(std::string(argv[2]), 54000);
        if (client.connectToServer()) {
            std::string endflag = "end!", demands[10], demand;
            while (1)
            {
                std::cout << "Enter the demand: ";
                std::getline(std::cin, demand);
                handleDemand(demands, demand);

                if (demands[0] == "shutdown" && demands[1] == endflag) client.sendShutdownRequest();
                if (demands[0] == "screenshot" && demands[1] == endflag) client.sendScreenshotRequest();
                if (demands[0] == "copyfile" && demands[3] == endflag) client.sendFileCopyRequest(demands[1], demands[2]);
                if (demands[0] == "webcam" && demands[1] == endflag) client.sendWebcamRequest();
                if (demands[0] == "closeconnection" && demands[1] == endflag) {
                    client.sendCloseConnection();
                    break;
                }
                // client.sendCloseWebcamRequest();
                // client.sendStartWebcamRequest();
                // client.sendStopWebcamRequest();
            }
        }
    } else {
        std::cerr << "Usage: " << argv[0] << " [server | client]" << std::endl;
    }
    // GmailClient client("credential.json");
    // client.renewAccessToken();
    // client.listUnreadEmail();
    // client.writeAccessToken("credential.json");
    return 0;
}