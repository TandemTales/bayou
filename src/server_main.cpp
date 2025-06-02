#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <sqlite3.h> // Added for SQLite
#include <algorithm> // Added for std::max

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
    std::string username;  // Player's username
    int rating = 1000;     // Player's rating, default to 1000
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

                                // Check for game over and update ratings
                                if (gameRules.isGameOver(globalGameState)) {
                                    std::cout << "Game Over detected." << std::endl;
                                    PlayerSide winner = PlayerSide::NEUTRAL; // Default to draw
                                    if (gameRules.hasPlayerWon(globalGameState, PlayerSide::PLAYER_ONE)) {
                                        winner = PlayerSide::PLAYER_ONE;
                                    } else if (gameRules.hasPlayerWon(globalGameState, PlayerSide::PLAYER_TWO)) {
                                        winner = PlayerSide::PLAYER_TWO;
                                    }

                                    std::shared_ptr<ClientConnection> player1_conn = nullptr;
                                    std::shared_ptr<ClientConnection> player2_conn = nullptr;
                                    
                                    // Lock clientsMutex when accessing the clients vector outside of main connection loop
                                    std::lock_guard<std::mutex> lock(clientsMutex);
                                    if (clients.size() == REQUIRED_PLAYERS) { 
                                        // This simplified logic assumes clients[0] and clients[1] are the players.
                                        // It also assumes their playerSide was set correctly upon connection.
                                        if (clients[0]->playerSide == PlayerSide::PLAYER_ONE) {
                                            player1_conn = clients[0];
                                            player2_conn = clients[1];
                                        } else if (clients[0]->playerSide == PlayerSide::PLAYER_TWO) { // Check if client[0] is P2
                                            player1_conn = clients[1]; // Then client[1] must be P1
                                            player2_conn = clients[0];
                                        } else { // Fallback if playerSides are mixed up or not set as expected.
                                             // Attempt to find by iterating - more robust
                                            for(const auto& c : clients) {
                                                if (c->playerSide == PlayerSide::PLAYER_ONE) player1_conn = c;
                                                else if (c->playerSide == PlayerSide::PLAYER_TWO) player2_conn = c;
                                            }
                                        }
                                    }
                                    
                                    if (!player1_conn || !player2_conn) {
                                        std::cerr << "Error: Could not find player connections for rating update." << std::endl;
                                    } else {
                                        int p1_old_rating = player1_conn->rating;
                                        int p2_old_rating = player2_conn->rating;
                                        int p1_new_rating = p1_old_rating;
                                        int p2_new_rating = p2_old_rating;
                                        const int RATING_CHANGE = 10;

                                        if (winner == PlayerSide::PLAYER_ONE) {
                                            p1_new_rating += RATING_CHANGE;
                                            p2_new_rating -= RATING_CHANGE;
                                            std::cout << "Player 1 (" << player1_conn->username << ") wins." << std::endl;
                                        } else if (winner == PlayerSide::PLAYER_TWO) {
                                            p2_new_rating += RATING_CHANGE;
                                            p1_new_rating -= RATING_CHANGE;
                                            std::cout << "Player 2 (" << player2_conn->username << ") wins." << std::endl;
                                        } else {
                                            std::cout << "Game is a draw." << std::endl;
                                        }
                                        
                                        p1_new_rating = std::max(0, p1_new_rating);
                                        p2_new_rating = std::max(0, p2_new_rating);

                                        sqlite3* db;
                                        if (sqlite3_open("../../bayou_bonanza.db", &db) == SQLITE_OK) {
                                            const char* sql_update = "UPDATE users SET rating = ? WHERE username = ?;";
                                            sqlite3_stmt* stmt_update;

                                            if (sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, 0) == SQLITE_OK) {
                                                sqlite3_bind_int(stmt_update, 1, p1_new_rating);
                                                sqlite3_bind_text(stmt_update, 2, player1_conn->username.c_str(), -1, SQLITE_STATIC);
                                                if (sqlite3_step(stmt_update) != SQLITE_DONE) {
                                                     std::cerr << "Error updating rating for " << player1_conn->username << ": " << sqlite3_errmsg(db) << std::endl;
                                                }
                                                sqlite3_finalize(stmt_update);
                                                player1_conn->rating = p1_new_rating; 
                                            } else {
                                                std::cerr << "Failed to prepare update statement for " << player1_conn->username << ": " << sqlite3_errmsg(db) << std::endl;
                                            }
                                            
                                            if (sqlite3_prepare_v2(db, sql_update, -1, &stmt_update, 0) == SQLITE_OK) {
                                                sqlite3_bind_int(stmt_update, 1, p2_new_rating);
                                                sqlite3_bind_text(stmt_update, 2, player2_conn->username.c_str(), -1, SQLITE_STATIC);
                                                if (sqlite3_step(stmt_update) != SQLITE_DONE) {
                                                    std::cerr << "Error updating rating for " << player2_conn->username << ": " << sqlite3_errmsg(db) << std::endl;
                                                }
                                                sqlite3_finalize(stmt_update);
                                                player2_conn->rating = p2_new_rating;
                                            } else {
                                                 std::cerr << "Failed to prepare update statement for " << player2_conn->username << ": " << sqlite3_errmsg(db) << std::endl;
                                            }
                                            sqlite3_close(db);
                                            std::cout << player1_conn->username << " new rating: " << p1_new_rating << std::endl;
                                            std::cout << player2_conn->username << " new rating: " << p2_new_rating << std::endl;
                                        } else {
                                            std::cerr << "Error opening database for rating update: " << sqlite3_errmsg(db) << std::endl;
                                        }
                                    }
                                }
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

void initialize_database() {
    sqlite3* db;
    char* err_msg = 0;
    // Use absolute path to project root for consistent database location
    std::string db_path = "../../bayou_bonanza.db"; // Relative to build/Debug/ directory
    int rc = sqlite3_open(db_path.c_str(), &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        // Though sqlite3_open() documentation says it always returns a DB handle,
        // it's good practice to close if it's not NULL, especially if other parts of the code
        // might try to use it. If sqlite3_open fails, db might be NULL or garbage,
        // sqlite3_close(NULL) is a no-op and safe.
        if (db) {
            sqlite3_close(db);
        }
        // It might be better to exit or throw an exception here if the DB is critical
        return; 
    } else {
        std::cout << "Opened database successfully at: " << db_path << std::endl;
    }

    const char* sql_create_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY NOT NULL,"
        "rating INTEGER NOT NULL DEFAULT 1000"
        ");";

    rc = sqlite3_exec(db, sql_create_table, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg); // Free error message
    } else {
        std::cout << "Table 'users' created successfully or already exists" << std::endl;
    }

    sqlite3_close(db);
}

int main() {
    initialize_database(); // Initialize the database at the start

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
                // Handle UserLogin
                new_client_conn->socket.setBlocking(true);
                sf::Packet loginPacket;
                std::string received_username;
                bool login_successful = false;

                if (new_client_conn->socket.receive(loginPacket) == sf::Socket::Done) {
                    MessageType msg_type;
                    if (loginPacket >> msg_type && msg_type == MessageType::UserLogin) {
                        if (loginPacket >> received_username && !received_username.empty()) {
                            sqlite3* db;
                            int rc_db = sqlite3_open("../../bayou_bonanza.db", &db);
                            if (rc_db) {
                                std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
                                if(db) sqlite3_close(db);
                                // Optionally, reject connection or use default rating
                            } else {
                                std::cout << "Database opened for user: " << received_username << std::endl;
                                const char* sql_select = "SELECT rating FROM users WHERE username = ?;";
                                sqlite3_stmt* stmt_select;
                                if (sqlite3_prepare_v2(db, sql_select, -1, &stmt_select, 0) == SQLITE_OK) {
                                    sqlite3_bind_text(stmt_select, 1, received_username.c_str(), -1, SQLITE_STATIC);
                                    int rc_step = sqlite3_step(stmt_select);
                                    if (rc_step == SQLITE_ROW) {
                                        new_client_conn->rating = sqlite3_column_int(stmt_select, 0);
                                        std::cout << "User " << received_username << " found with rating " << new_client_conn->rating << std::endl;
                                    } else if (rc_step == SQLITE_DONE) {
                                        // User not found, insert new user
                                        const char* sql_insert = "INSERT INTO users (username, rating) VALUES (?, ?);";
                                        sqlite3_stmt* stmt_insert;
                                        if (sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, 0) == SQLITE_OK) {
                                            sqlite3_bind_text(stmt_insert, 1, received_username.c_str(), -1, SQLITE_STATIC);
                                            sqlite3_bind_int(stmt_insert, 2, 1000); // Default rating
                                            if (sqlite3_step(stmt_insert) == SQLITE_DONE) {
                                                new_client_conn->rating = 1000;
                                                std::cout << "New user " << received_username << " inserted with default rating 1000." << std::endl;
                                            } else {
                                                std::cerr << "SQL error inserting user: " << sqlite3_errmsg(db) << std::endl;
                                            }
                                            sqlite3_finalize(stmt_insert);
                                        } else {
                                            std::cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << std::endl;
                                        }
                                    } else {
                                        std::cerr << "SQL error selecting user: " << sqlite3_errmsg(db) << std::endl;
                                    }
                                    sqlite3_finalize(stmt_select);
                                } else {
                                    std::cerr << "Failed to prepare select statement: " << sqlite3_errmsg(db) << std::endl;
                                }
                                sqlite3_close(db);
                                new_client_conn->username = received_username;
                                login_successful = true;
                            }
                        } else {
                             std::cerr << "Failed to deserialize username or username empty." << std::endl;
                        }
                    } else {
                        std::cerr << "Login failed: Did not receive UserLogin message type or failed to deserialize type." << std::endl;
                    }
                } else {
                    std::cerr << "Failed to receive login packet from " << new_client_conn->socket.getRemoteAddress() << std::endl;
                }
                
                new_client_conn->socket.setBlocking(false); // Set back to non-blocking for game loop

                if (!login_successful) {
                    std::cout << "Login failed for client " << new_client_conn->socket.getRemoteAddress() << ". Disconnecting." << std::endl;
                    new_client_conn->socket.disconnect(); // Disconnect if login failed
                    continue; // Skip adding to clients list and further processing
                }

                // Assign PlayerSide
                new_client_conn->playerSide = (clients.empty()) ? PlayerSide::PLAYER_ONE : PlayerSide::PLAYER_TWO;
                clients.push_back(new_client_conn);
                
                std::cout << "User '" << new_client_conn->username << "' (Rating: " << new_client_conn->rating 
                          << ") connected as Player " << (new_client_conn->playerSide == PlayerSide::PLAYER_ONE ? "One" : "Two")
                          << " from " << new_client_conn->socket.getRemoteAddress() << std::endl;
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

                    // Prepare extended GameStart message data
                    // Ensure clients[0] is Player One and clients[1] is Player Two as per current logic
                    std::string p1_username = clients[0]->username;
                    int p1_rating = clients[0]->rating;
                    std::string p2_username = clients[1]->username;
                    int p2_rating = clients[1]->rating;

                    std::cout << "P1: " << p1_username << " (" << p1_rating << "), P2: " << p2_username << " (" << p2_rating << ")" << std::endl;

                    // Send GameStart, usernames, ratings, and initial GameState to both players
                    sf::Packet gameStartPacket; // Create one packet for both
                    gameStartPacket << MessageType::GameStart 
                                    << p1_username << p1_rating 
                                    << p2_username << p2_rating 
                                    << globalGameState; 

                    if (clients[0]->socket.send(gameStartPacket) != sf::Socket::Done) {
                        std::cerr << "Error sending GameStart packet to " << clients[0]->username << std::endl;
                    } else {
                        std::cout << "GameStart packet sent to " << clients[0]->username << std::endl;
                    }
                    
                    if (clients[1]->socket.send(gameStartPacket) != sf::Socket::Done) {
                        std::cerr << "Error sending GameStart packet to " << clients[1]->username << std::endl;
                    } else {
                        std::cout << "GameStart packet sent to " << clients[1]->username << std::endl;
                    }

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
