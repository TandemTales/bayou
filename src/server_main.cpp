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
#include "GameRules.h"        // For game logic
#include "TurnManager.h"      // For turn management

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

// Game logic components
GameState globalGameState; // Properly initialized by GameInitializer once game starts
GameInitializer gameInitializer; // Instance of GameInitializer
GameRules gameRules; // Game rules for move validation and processing
std::unique_ptr<TurnManager> turnManager; // Turn manager for game flow

// Helper function to find piece at position and reconstruct move
Move reconstructMoveWithPiece(const Move& clientMove, const GameState& gameState) {
    const GameBoard& board = gameState.getBoard();
    const Position& from = clientMove.getFrom();
    
    // Get the piece at the from position
    if (!board.isValidPosition(from.x, from.y)) {
        return Move(); // Invalid position, return default move
    }
    
    const Square& fromSquare = board.getSquare(from.x, from.y);
    if (fromSquare.isEmpty()) {
        return Move(); // No piece at position, return default move
    }
    
    // Create a temporary shared_ptr wrapper for the Move constructor
    // Note: Using no-op deleter since the piece is owned by Square
    Piece* rawPiece = fromSquare.getPiece();
    std::shared_ptr<Piece> piece(rawPiece, [](Piece*){});
    
    // Create complete move with piece reference
    if (clientMove.isPromotion()) {
        return Move(piece, from, clientMove.getTo(), clientMove.getPromotionType());
    } else {
        return Move(piece, from, clientMove.getTo());
    }
}

// Helper function to broadcast game state to all connected clients
void broadcastGameState(const GameState& gameState) {
    sf::Packet updatePacket;
    updatePacket << MessageType::GameStateUpdate << gameState;
    
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (auto& client : clients) {
        if (client->connected) {
            if (client->socket.send(updatePacket) != sf::Socket::Done) {
                std::cerr << "Error sending game state update to client " 
                          << client->socket.getRemoteAddress() << std::endl;
            }
        }
    }
}

// Helper function to send move rejection to specific client
void sendMoveRejection(std::shared_ptr<ClientConnection> client, const std::string& reason) {
    sf::Packet rejectPacket;
    rejectPacket << MessageType::MoveRejected;
    // Note: Could add reason string if MessageType::MoveRejected supports it
    
    if (client->socket.send(rejectPacket) != sf::Socket::Done) {
        std::cerr << "Error sending move rejection to client " 
                  << client->socket.getRemoteAddress() << std::endl;
    }
    
    std::cout << "Move rejected for " << client->socket.getRemoteAddress() 
              << ": " << reason << std::endl;
}

void handle_client(std::shared_ptr<ClientConnection> client) {
    std::cout << "Thread started for client: " << client->socket.getRemoteAddress() 
              << ":" << client->socket.getRemotePort() << std::endl;
    client->connected = true;

    sf::Socket::Status status;
    while (client->connected) {
        sf::Packet packet;
        status = client->socket.receive(packet);

        if (status == sf::Socket::Done) {
            // Received data from client
            MessageType messageType;
            if (!(packet >> messageType)) {
                std::cerr << "Error deserializing message type from " 
                          << client->socket.getRemoteAddress() << std::endl;
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

                    // Reconstruct the move with the actual piece reference
                    Move completeMove = reconstructMoveWithPiece(clientMove, globalGameState);
                    
                    if (!completeMove.getPiece()) {
                        sendMoveRejection(client, "No piece at source position");
                        continue;
                    }
                    
                    // Verify the move is from the correct player
                    if (completeMove.getPiece()->getSide() != client->playerSide) {
                        sendMoveRejection(client, "Cannot move opponent's piece");
                        continue;
                    }
                    
                    // Verify it's the client's turn
                    if (client->playerSide != globalGameState.getActivePlayer()) {
                        sendMoveRejection(client, "Not your turn");
                        continue;
                    }
                    
                    // Process the move using TurnManager
                    if (turnManager) {
                        bool moveProcessed = false;
                        std::string resultMessage;
                        
                        turnManager->processMoveAction(completeMove, [&](const ActionResult& result) {
                            moveProcessed = true;
                            resultMessage = result.message;
                            
                            if (result.success) {
                                std::cout << "Move processed successfully: " << result.message << std::endl;
                                // Broadcast updated game state to all clients
                                broadcastGameState(globalGameState);
                            } else {
                                std::cout << "Move failed: " << result.message << std::endl;
                                sendMoveRejection(client, result.message);
                            }
                        });
                        
                        // If no callback was called (shouldn't happen), handle as error
                        if (!moveProcessed) {
                            sendMoveRejection(client, "Move processing failed");
                        }
                    } else {
                        sendMoveRejection(client, "Game not properly initialized");
                    }
                    
                } else {
                    std::cerr << "Error deserializing move data from " 
                              << client->socket.getRemoteAddress() << std::endl;
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
            std::cerr << "Network error receiving from client: " 
                      << client->socket.getRemoteAddress() << std::endl;
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
                
                std::cout << "Client connected: " << new_client_conn->socket.getRemoteAddress() 
                          << ":" << new_client_conn->socket.getRemotePort() 
                          << " as Player " << (new_client_conn->playerSide == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
                std::cout << "Current players connected: " << clients.size() << "/" << REQUIRED_PLAYERS << std::endl;

                // Send PlayerAssignment
                sf::Packet assignmentPacket;
                assignmentPacket << MessageType::PlayerAssignment << new_client_conn->playerSide;
                new_client_conn->socket.send(assignmentPacket);

                if (clients.size() == 1) {
                    // First client connected, send WaitingForOpponent
                    sf::Packet waitingPacket;
                    waitingPacket << MessageType::WaitingForOpponent;
                    clients[0]->socket.send(waitingPacket);
                    std::cout << "Sent WaitingForOpponent to Player One." << std::endl;
                } else if (clients.size() == REQUIRED_PLAYERS) {
                    std::cout << "All players connected. Initializing and starting game..." << std::endl;
                    
                    // Initialize the game state and create turn manager
                    gameInitializer.initializeNewGame(globalGameState);
                    turnManager = std::make_unique<TurnManager>(globalGameState, gameRules);
                    
                    // Debug: Print board state after initialization
                    std::cout << "DEBUG: Board state after initialization:" << std::endl;
                    const GameBoard& board = globalGameState.getBoard();
                    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
                        std::cout << y << " | ";
                        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
                            const Square& square = board.getSquare(x, y);
                            char symbol = '.';
                            if (!square.isEmpty()) {
                                Piece* piece = square.getPiece();
                                std::string symbol_str = piece->getSymbol();
                                symbol = symbol_str.empty() ? '.' : symbol_str[0];
                                if (piece->getSide() == PlayerSide::PLAYER_TWO) {
                                    symbol = std::tolower(symbol);
                                }
                            }
                            std::cout << symbol << ' ';
                        }
                        std::cout << "|" << std::endl;
                    }
                    
                    std::cout << "Game initialized. Broadcasting GameStart and initial state." << std::endl;

                    // Send GameStart and initial GameState to both players
                    sf::Packet gameStartPacketP1, gameStartPacketP2;
                    gameStartPacketP1 << MessageType::GameStart << globalGameState;
                    clients[0]->socket.send(gameStartPacketP1);

                    // Create a new packet for player 2 to avoid issues with packet read position
                    gameStartPacketP2 << MessageType::GameStart << globalGameState; 
                    clients[1]->socket.send(gameStartPacketP2);

                    // Start thread for the first client now that the game is starting
                    client_threads.emplace_back(handle_client, clients[0]);
                    // Start thread for the second client
                    client_threads.emplace_back(handle_client, clients[1]);
                } else if (clients.size() == 1) {
                     std::cout << "Waiting for the second player..." << std::endl;
                }

            } else {
                // Max players reached, reject new connection
                std::cout << "Max players reached. Rejecting new connection from: " 
                          << new_client_conn->socket.getRemoteAddress() << std::endl;
                sf::Packet rejectPacket;
                rejectPacket << MessageType::Error;
                new_client_conn->socket.send(rejectPacket);
                new_client_conn->socket.disconnect();
            }
        }

        // Add a small sleep to prevent busy-waiting
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait for all client threads to finish (this won't be reached in the current design)
    for (auto& thread : client_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    return 0;
}
