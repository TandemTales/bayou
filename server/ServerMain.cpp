#include "Server.h"
#include <iostream>

int main() {
    unsigned short port = 12345; // Example port number
    std::cout << "Starting Bayou Bonanza Server..." << std::endl;

    try {
        Server gameServer(port);
        // The constructor already prints if it's listening or if there was an error.
        // We could add a getter like gameServer.isListening() if we want to check state here.
        
        gameServer.waitForClients();
        
        std::cout << "Server operations complete. Exiting." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1; // Indicate an error
    } catch (...) {
        std::cerr << "An unknown error occurred." << std::endl;
        return 1; // Indicate an error
    }

    return 0; // Success
}
