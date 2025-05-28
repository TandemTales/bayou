#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>

#include "GameState.h"      // For GameState and its sf::Packet operators
#include "Move.h"           // For Move and its sf::Packet operators
#include "NetworkProtocol.h"  // For MessageType enum and operators
#include "GameInitializer.h"  // For initializing the game state
#include "PlayerSide.h"       // For PlayerSide enum (used in PlayerAssignment)
// #include "GameRules.h"   // Example for future game logic
// #include "TurnManager.h" // Example for future game logic

using namespace BayouBonanza;

const unsigned short PORT = 50000;
const int REQUIRED_PLAYERS = 2;

struct ClientConnection {
    sf::TcpSocket socket;
    PlayerSide playerSide; // Assign PlayerSide to each connection
    bool connected = false;
};

// Global vector to store client connections (needs thread safety if accessed by multiple threads)
std::vector<std::shared_ptr<ClientConnection>> clients;
std::mutex clientsMutex; // To protect access to the clients vector

// Placeholder for GameState object
GameState globalGameState; // Properly initialized by GameInitializer once game starts
GameInitializer gameInitializer; // Instance of GameInitializer
// GameRules gameRules; // Example for future game logic
// TurnManager turnManager(globalGameState, gameRules); // Example for future game logic


void handle_client(std::shared_ptr<ClientConnection> client) {
    std::cout << "Thread started for client: " << client->socket.getRemoteAddress() << ":" << client->socket.getRemotePort() << std::endl;
    client->connected = true;

    // Example: Assign player side (needs proper logic)
    // {
    //     std::lock_guard<std::mutex> lock(clientsMutex);
    //     if (clients.size() == 1) client->side = PlayerSide::PLAYER_ONE;
    //     else client->side = PlayerSide::PLAYER_TWO;
    //     // Send player assignment to client
    //     sf::Packet playerAssignmentPacket;
    //     playerAssignmentPacket << static_cast<sf::Uint8>(client->side); // Example message
    //     client->socket.send(playerAssignmentPacket);
    // }


    sf::Socket::Status status;
    while (client->connected) {
        sf::Packet packet;
        status = client->socket.receive(packet);

        if (status == sf::Socket::Done) {
            // Received data from client
            MessageType messageType;
            if (!(packet >> messageType)) {
                std::cerr << "Error deserializing message type from " << client->socket.getRemoteAddress() << std::endl;
                continue; // Try to receive next packet
            }
            std::cout << "Received message type: " << static_cast<int>(messageType) 
                      << " from " << client->socket.getRemoteAddress() << std::endl;

            if (messageType == MessageType::MoveToServer) {
                Move clientMove;
                if (packet >> clientMove) { // Deserialize the rest of the packet as Move
                    std::cout << "Move received: " 
                              << clientMove.getFrom().x << "," << clientMove.getFrom().y 
                              << " -> " 
                              << clientMove.getTo().x << "," << clientMove.getTo().y << std::endl;

                    // TODO: Process move with globalGameState and TurnManager.
                    // bool moveValid = turnManager.processMoveAction(clientMove, client->playerSide); 
                    // For now, assume valid and broadcast.
                    
                    sf::Packet updatePacketToAll;
                    // globalGameState should be updated by TurnManager or similar logic here
                    updatePacketToAll << MessageType::GameStateUpdate << globalGameState;
                    
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    for (auto& c : clients) {
                        if (c->connected) {
                            if (c->socket.send(updatePacketToAll) != sf::Socket::Done) {
                                std::cerr << "Error sending game state update to client " << c->socket.getRemoteAddress() << std::endl;
                            }
                        }
                    }
                } else {
                    std::cerr << "Error deserializing move data from " << client->socket.getRemoteAddress() << std::endl;
                }
            } else {
                // Handle other message types or log unexpected ones
                std::cout << "Received unhandled message type: " << static_cast<int>(messageType) 
                          << " from " << client->socket.getRemoteAddress() << std::endl;
            }

        } else if (status == sf::Socket::NotReady) {
            // No data received, non-blocking socket, this is normal
        } else if (status == sf::Socket::Disconnected) {
            std::cout << "Client disconnected: " << client->socket.getRemoteAddress() << std::endl;
            client->connected = false;
            // Handle disconnection: remove client, notify other player, etc.
            break;
        } else if (status == sf::Socket::Error) {
            std::cerr << "Network error receiving from client: " << client->socket.getRemoteAddress() << std::endl;
            client->connected = false;
            // Handle error
            break;
        }
        // Add a small sleep to prevent busy-waiting if not using selectors
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Cleanup client connection
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove_if(clients.begin(), clients.end(), [&](const std::shared_ptr<ClientConnection>& c){
            return c.get() == client.get();
        }), clients.end());
        std::cout << "Client removed. Current client count: " << clients.size() << std::endl;
    }
}

int main() {
    sf::TcpListener listener;

    // Bind the listener to a port
    if (listener.listen(PORT) != sf::Socket::Done) {
        std::cerr << "Error: Could not bind listener to port " << PORT << std::endl;
        return 1;
    }
    std::cout << "Server listening on port " << PORT << "..." << std::endl;
    std::cout << "Waiting for " << REQUIRED_PLAYERS << " players to connect..." << std::endl;

    listener.setBlocking(false); // Use non-blocking to allow checking for game start condition

    std::vector<std::thread> client_threads;

    while (true) { // Main server loop
        std::shared_ptr<ClientConnection> new_client_conn = std::make_shared<ClientConnection>();

        if (listener.accept(new_client_conn->socket) == sf::Socket::Done) {
            std::lock_guard<std::mutex> lock(clientsMutex); // Protect clients vector
            if (clients.size() < REQUIRED_PLAYERS) {
                // Assign PlayerSide
                new_client_conn->playerSide = (clients.empty()) ? PlayerSide::PLAYER_ONE : PlayerSide::PLAYER_TWO;
                clients.push_back(new_client_conn);
                
                std::cout << "Client connected: " << new_client_conn->socket.getRemoteAddress() << ":" << new_client_conn->socket.getRemotePort() 
                          << " as Player " << (new_client_conn->playerSide == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
                std::cout << "Current players connected: " << clients.size() << "/" << REQUIRED_PLAYERS << std::endl;

                // Send PlayerAssignment
                sf::Packet assignmentPacket;
                assignmentPacket << MessageType::PlayerAssignment << new_client_conn->playerSide; // PlayerSide enum is serializable
                new_client_conn->socket.send(assignmentPacket);

                if (clients.size() == 1) {
                    // First client connected, send WaitingForOpponent
                    sf::Packet waitingPacket;
                    waitingPacket << MessageType::WaitingForOpponent;
                    clients[0]->socket.send(waitingPacket);
                    std::cout << "Sent WaitingForOpponent to Player One." << std::endl;
                } else if (clients.size() == REQUIRED_PLAYERS) {
                    std::cout << "All players connected. Initializing and starting game..." << std::endl;
                    
                    gameInitializer.initializeNewGame(globalGameState); // Initialize the game state
                    std::cout << "Game initialized. Broadcasting GameStart and initial state." << std::endl;

                    // Send GameStart and initial GameState to both players
                    sf::Packet gameStartPacketP1, gameStartPacketP2;
                    gameStartPacketP1 << MessageType::GameStart << globalGameState;
                    clients[0]->socket.send(gameStartPacketP1);

                    // Create a new packet for player 2 to avoid issues with packet read position if send fails/retries
                    gameStartPacketP2 << MessageType::GameStart << globalGameState; 
                    clients[1]->socket.send(gameStartPacketP2);

                    // Start thread for the first client now that the game is starting
                    client_threads.emplace_back(handle_client, clients[0]);
                    // Start thread for the second client
                    client_threads.emplace_back(handle_client, clients[1]);
                } else if (clients.size() == 1) {
                     std::cout << "Waiting for the second player..." << std::endl;
                     // Optionally, send a "waiting for opponent" message to the first client
                }

            } else {
                // Max players reached, reject new connection
                std::cout << "Max players reached. Rejecting new connection from: " << new_client_conn->socket.getRemoteAddress() << std::endl;
                sf::Packet rejectPacket;
                std::string message = "Server is full.";
                rejectPacket << message;
                new_client_conn->socket.send(rejectPacket);
                new_client_conn->socket.disconnect();
            }
        }

        // Basic server running indication, can be removed or expanded later
        // std::cout << "."; // This will print a lot, maybe too much
        // std::flush(std::cout);
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Prevent busy loop

        // Optional: Add logic here to shut down the server gracefully
    }

    // Wait for all client threads to finish (though the current loop is infinite)
    for (auto& t : client_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    return 0;
}
