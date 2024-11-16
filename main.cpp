#include "client_socket.h"
#include "server_socket.h"
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

 
    if (argc == 2 && std::string(argv[1]) == "server") {
        // Start server
        Server server(54000);
        if (server.start()) {
            std::cout <<"listening";
            server.listenForConnections();  // Synchronously listen for connections
        }
    } else if (argc == 2 && std::string(argv[1]) == "client") {
        // Start client
        Client client("192.168.2.2", 54000);
        if (client.connectToServer()) {
            int choice;
            std::cout  << "Input: "; std::cin >> choice;
            switch(choice){
                case 1: client.sendShutdownRequest(); break;
                case 2: client.sendRestartRequest(); break;
                case 3: client.sendListOfAppRequest(); break;
                case 4: client.sendListOfServiceRequest(); break;
                case 5: {
                    std::string appName;
                   getline(std::cin, appName);
                    getline(std::cin, appName);
                    client.sendOpenAppRequest(appName);
                }; break;
                case 6: {
                    std::string processId;
                    std::cout << "Input process ID: ";
                    std::cin.ignore();
                    getline(std::cin, processId);
                    client.sendCloseAppRequest(processId);
                };break;
                case 7:{
                    std::string command;
                    while (true) {
                        std::cout << "Enter 'start' to start keylogger, 'stop' to stop keylogger, or 'exit' to quit: ";
                        std::cin >> command;

                        if (command == "start") {
                            client.sendKeyloggerStartRequest();
                            std::cout << "Keylogger started." << std::endl;
                        } 
                        else if (command == "stop") {
                            client.sendKeyloggerOffRequest();
                            std::cout << "Keylogger stopped and log file sent to server." << std::endl;
                        } 
                        else if (command == "exit") {
                            break;  // Exit the loop and end the client session
                        } 
                        else {
                            std::cout << "Unknown command. Try 'start', 'stop', or 'exit'." << std::endl;
                        }

                        // Optional delay between commands
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }; break;
                default: break;
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