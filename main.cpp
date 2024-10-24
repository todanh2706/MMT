#include "Client.h"
#include "Server.h"
#include <iostream>
#include <thread>

void runServer() {
    Server server(8888);
    if (server.startListening()) {
        std::string message = server.receiveMessage();
        if (message == "shutdown") {
            std::cout << "Shutdown command received. Stopping server..." << std::endl;
        }
        server.stop();
    }
}

void runClient() {
    Client client("127.0.0.1", 8888);
    if (client.connectToServer()) {
        client.sendMessage("shutdown");
        client.cleanup();
    }
}

int main() {
    // Run server in a separate thread
    std::thread serverThread(runServer);
    
    // Wait for a moment to let server initialize
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Run client in the main thread
    runClient();

    // Wait for the server thread to finish
    serverThread.join();

    return 0;
}
