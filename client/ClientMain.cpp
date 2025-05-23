#include "Client.h"
#include <iostream>

int main() {
    // Placeholder for ServerMain.cpp
    // int main() {
    // return 0;
    // }
    // The above was the content from the placeholder.
    // New ClientMain content:
    std::string serverIp = "127.0.0.1"; // Default to localhost
    unsigned short serverPort = 12345;   // Default port

    // Potentially parse command-line arguments for IP and port here
    // For now, using defaults.

    std::cout << "Starting Bayou Bonanza Client..." << std::endl;
    std::cout << "Attempting to connect to server at " << serverIp << ":" << serverPort << std::endl;

    try {
        Client client(serverIp, serverPort);
        client.run();
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred in Client: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "An unknown error occurred in Client." << std::endl;
        return 1;
    }

    std::cout << "Bayou Bonanza Client finished." << std::endl;
    return 0;
}
