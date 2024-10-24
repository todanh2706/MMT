#include "client_socket.h"
#include "server_socket.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "server") {
        // Start server
        Server server(54000);
        if (server.start()) {
            server.listenForConnections();  // Synchronously listen for connections
        }
    } else if (argc == 2 && std::string(argv[1]) == "client") {
        // Start client
        Client client("192.168.226.131", 54000);
        if (client.connectToServer()) {
            client.sendShutdownRequest();
        }
    } else {
        std::cerr << "Usage: " << argv[0] << " [server | client]" << std::endl;
    }

    return 0;
}