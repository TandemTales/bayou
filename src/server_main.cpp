#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <sqlite3.h> // Added for SQLite
#include <algorithm> // Added for std::max
#include <cmath>     // Added for std::pow in Elo calculation

#include "GameState.h"      // For GameState and its sf::Packet operators
#include "Move.h"           // For Move and its sf::Packet operators
#include "NetworkProtocol.h"  // For MessageType enum and operators
#include "GameInitializer.h"  // For initializing the game state
#include "PlayerSide.h"       // For PlayerSide enum (used in PlayerAssignment)
#include "GameRules.h"        // For game logic
#include "TurnManager.h"      // For turn management
#include "Square.h"           // For Square::setGlobalPieceFactory
#include "PieceFactory.h"     // For PieceFactory
#include "PieceDefinitionManager.h" // For PieceDefinitionManager
#include "CardCollection.h"  // For Deck and CardCollection
#include "CardFactory.h"     // For creating cards from IDs

using namespace BayouBonanza;

const unsigned short PORT = 50000;
const int REQUIRED_PLAYERS = 2;

struct GameSession; // Forward declaration

struct ClientConnection {
    sf::TcpSocket socket;
    PlayerSide playerSide; // Assign PlayerSide to each connection
    std::string username;  // Player's username
    int rating = 0;        // Player's rating, default to 0
    bool connected = false;
    bool lookingForMatch = false; // Whether the player is actively looking for a match
    CardCollection collection; // Player's owned cards
    Deck deck;                 // Player's current deck
    std::weak_ptr<GameSession> session; // Game this client is in
};

// Global vector to store client connections (needs thread safety if accessed by multiple threads)
std::vector<std::shared_ptr<ClientConnection>> clients;
std::mutex clientsMutex; // To protect access to the clients vector

struct GameSession {
    GameState gameState;
    std::unique_ptr<TurnManager> turnManager;
    std::shared_ptr<ClientConnection> player1;
    std::shared_ptr<ClientConnection> player2;
};
std::vector<std::shared_ptr<GameSession>> gameSessions;
std::mutex gamesMutex;

// Game logic components
std::unique_ptr<GameInitializer> gameInitializer; // Will be initialized after PieceFactory setup
GameRules gameRules; // Game rules for move validation and processing

// Global PieceFactory for piece creation (needed for card play)
PieceDefinitionManager globalPieceDefManager;
std::unique_ptr<PieceFactory> globalPieceFactory;

// Find an existing game session that involves the given username
std::shared_ptr<GameSession> findGameSessionByUsername(const std::string& username) {
    std::lock_guard<std::mutex> gamesLock(gamesMutex);
    for (auto& session : gameSessions) {
        if ((session->player1 && session->player1->username == username) ||
            (session->player2 && session->player2->username == username)) {
            return session;
        }
    }
    return nullptr;
}

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
// Helper function to print card hands for debugging
void printCardHands(const GameState& gameState) {
    
    
    // Player 1 hand
    const Hand& p1Hand = gameState.getHand(PlayerSide::PLAYER_ONE);
    std::cout << "Player 1 Hand (" << p1Hand.size() << " cards, " 
              << gameState.getSteam(PlayerSide::PLAYER_ONE) << " steam):" << std::endl;
    for (size_t i = 0; i < p1Hand.size(); ++i) {
        const Card* card = p1Hand.getCard(i);
        if (card) {
            std::cout << "  [" << i << "] " << card->getName() 
                      << " (Cost: " << card->getSteamCost() << ")" << std::endl;
        }
    }
    
    // Player 2 hand
    const Hand& p2Hand = gameState.getHand(PlayerSide::PLAYER_TWO);
    std::cout << "Player 2 Hand (" << p2Hand.size() << " cards, " 
              << gameState.getSteam(PlayerSide::PLAYER_TWO) << " steam):" << std::endl;
    for (size_t i = 0; i < p2Hand.size(); ++i) {
        const Card* card = p2Hand.getCard(i);
        if (card) {
            std::cout << "  [" << i << "] " << card->getName() 
                      << " (Cost: " << card->getSteamCost() << ")" << std::endl;
        }
    }
    std::cout << "=========================" << std::endl;
}

void broadcastGameState(std::shared_ptr<GameSession> session) {
    if (!session) return;
    sf::Packet updatePacket;
    updatePacket << MessageType::GameStateUpdate << session->gameState;

    // Print card hands for debugging
    printCardHands(session->gameState);

    for (auto& client : {session->player1, session->player2}) {
        if (client && client->connected) {
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

void sendCardPlayRejection(std::shared_ptr<ClientConnection> client, const std::string& reason) {
    sf::Packet rejectPacket;
    rejectPacket << MessageType::CardPlayRejected;
    // Note: Could add reason string if MessageType::CardPlayRejected supports it
    
    if (client->socket.send(rejectPacket) != sf::Socket::Done) {
        std::cerr << "Error sending card play rejection to client " 
                  << client->socket.getRemoteAddress() << std::endl;
    }
    
    std::cout << "Card play rejected for " << client->socket.getRemoteAddress() 
              << ": " << reason << std::endl;
}

void tryStartMatchmaking() {
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    // Find two players who are looking for a match
    std::vector<std::shared_ptr<ClientConnection>> matchmakers;
    for (auto& client : clients) {
        if (client->lookingForMatch && client->connected) {
            matchmakers.push_back(client);
            if (matchmakers.size() == 2) break;
        }
    }
    
    if (matchmakers.size() == 2) {
        std::cout << "Found two players looking for a match. Starting game..." << std::endl;
        
        // Assign player sides
        matchmakers[0]->playerSide = PlayerSide::PLAYER_ONE;
        matchmakers[1]->playerSide = PlayerSide::PLAYER_TWO;
        
        // Send PlayerAssignment messages to inform clients of their sides
        sf::Packet assignment1;
        assignment1 << MessageType::PlayerAssignment << matchmakers[0]->playerSide;
        if (matchmakers[0]->socket.send(assignment1) != sf::Socket::Done) {
            std::cerr << "Error sending PlayerAssignment to " << matchmakers[0]->username << std::endl;
        } else {
            std::cout << "PlayerAssignment sent to " << matchmakers[0]->username << " (PLAYER_ONE)" << std::endl;
        }
        
        sf::Packet assignment2;
        assignment2 << MessageType::PlayerAssignment << matchmakers[1]->playerSide;
        if (matchmakers[1]->socket.send(assignment2) != sf::Socket::Done) {
            std::cerr << "Error sending PlayerAssignment to " << matchmakers[1]->username << std::endl;
        } else {
            std::cout << "PlayerAssignment sent to " << matchmakers[1]->username << " (PLAYER_TWO)" << std::endl;
        }
        
        // Clear their matchmaking flags
        matchmakers[0]->lookingForMatch = false;
        matchmakers[1]->lookingForMatch = false;
        
        // Create a new game session
        auto session = std::make_shared<GameSession>();
        gameInitializer->initializeNewGame(session->gameState, matchmakers[0]->deck, matchmakers[1]->deck);
        session->turnManager = std::make_unique<TurnManager>(session->gameState, gameRules);
        session->player1 = matchmakers[0];
        session->player2 = matchmakers[1];

        {
            std::lock_guard<std::mutex> gamesLock(gamesMutex);
            gameSessions.push_back(session);
        }
        matchmakers[0]->session = session;
        matchmakers[1]->session = session;

        // Debug: Print board state after initialization

        const GameBoard& board = session->gameState.getBoard();
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
        
        // Print initial card hands
        printCardHands(session->gameState);
        
        std::cout << "Game initialized. Broadcasting GameStart and initial state." << std::endl;

        // Prepare extended GameStart message data
        std::string p1_username = matchmakers[0]->username;
        int p1_rating = matchmakers[0]->rating;
        std::string p2_username = matchmakers[1]->username;
        int p2_rating = matchmakers[1]->rating;

        std::cout << "P1: " << p1_username << " (" << p1_rating << "), P2: " << p2_username << " (" << p2_rating << ")" << std::endl;

        // Send GameStart, usernames, ratings, and initial GameState to both players
        sf::Packet gameStartPacket; // Create one packet for both
        gameStartPacket << MessageType::GameStart
                        << p1_username << p1_rating
                        << p2_username << p2_rating
                        << session->gameState;

        if (matchmakers[0]->socket.send(gameStartPacket) != sf::Socket::Done) {
            std::cerr << "Error sending GameStart packet to " << matchmakers[0]->username << std::endl;
        } else {
            std::cout << "GameStart packet sent to " << matchmakers[0]->username << std::endl;
        }

        if (matchmakers[1]->socket.send(gameStartPacket) != sf::Socket::Done) {
            std::cerr << "Error sending GameStart packet to " << matchmakers[1]->username << std::endl;
        } else {
            std::cout << "GameStart packet sent to " << matchmakers[1]->username << std::endl;
        }
    }
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
                auto session = client->session.lock();
                if (!session) {
                    sendMoveRejection(client, "Not in a game");
                    continue;
                }
                Move clientMove;
                if (packet >> clientMove) { // Deserialize the rest of the packet as Move
                    std::cout << "Move received: "
                              << clientMove.getFrom().x << "," << clientMove.getFrom().y
                              << " -> "
                              << clientMove.getTo().x << "," << clientMove.getTo().y << std::endl;

                    // Reconstruct the move with the actual piece reference
                    Move completeMove = reconstructMoveWithPiece(clientMove, session->gameState);
                    
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
                    if (client->playerSide != session->gameState.getActivePlayer()) {
                        sendMoveRejection(client, "Not your turn");
                        continue;
                    }


                    // Process the move using TurnManager
                    if (session->turnManager) {
                        bool moveProcessed = false;
                        std::string resultMessage;

                        session->turnManager->processMoveAction(completeMove, [&](const ActionResult& result) {
                            moveProcessed = true;
                            resultMessage = result.message;

                            if (result.success) {
                                std::cout << "Move processed successfully: " << result.message << std::endl;

                                // Broadcast updated game state to all clients
                                broadcastGameState(session);

                                // Check for game over and update ratings
                                if (gameRules.isGameOver(session->gameState)) {
                                    std::cout << "Game Over detected." << std::endl;

                                    // Schedule session cleanup to prevent reconnection to finished games
                                    {
                                        auto session_to_cleanup = session;
                                        std::thread([session_to_cleanup]() {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                            {
                                                std::lock_guard<std::mutex> lock(gamesMutex);
                                                gameSessions.erase(std::remove_if(gameSessions.begin(), gameSessions.end(),
                                                    [&](const std::shared_ptr<GameSession>& s){ return s.get() == session_to_cleanup.get(); }),
                                                    gameSessions.end());
                                            }
                                            if (session_to_cleanup->player1) session_to_cleanup->player1->session.reset();
                                            if (session_to_cleanup->player2) session_to_cleanup->player2->session.reset();
                                        }).detach();
                                    }

                                    PlayerSide winner = PlayerSide::NEUTRAL; // Default to draw
                                    if (gameRules.hasPlayerWon(session->gameState, PlayerSide::PLAYER_ONE)) {
                                        winner = PlayerSide::PLAYER_ONE;
                                    } else if (gameRules.hasPlayerWon(session->gameState, PlayerSide::PLAYER_TWO)) {
                                        winner = PlayerSide::PLAYER_TWO;
                                    }

                                    auto player1_conn = session->player1;
                                    auto player2_conn = session->player2;

                                    if (!player1_conn || !player2_conn) {
                                        std::cerr << "Error: Could not find player connections for rating update." << std::endl;
                                    } else {
                                        int p1_old_rating = player1_conn->rating;
                                        int p2_old_rating = player2_conn->rating;

                                        // Elo rating calculation with +1000 adjustment
                                        int p1_rating_adjusted = p1_old_rating + 1000;
                                        int p2_rating_adjusted = p2_old_rating + 1000;

                                        // Calculate expected scores
                                        double expected_p1 = 1.0 / (1.0 + std::pow(10.0, (p2_rating_adjusted - p1_rating_adjusted) / 400.0));
                                        double expected_p2 = 1.0 / (1.0 + std::pow(10.0, (p1_rating_adjusted - p2_rating_adjusted) / 400.0));

                                        // K-factor for rating changes
                                        const int K_FACTOR = 32;

                                        // Calculate new adjusted ratings based on game outcome
                                        int p1_new_rating_adjusted, p2_new_rating_adjusted;
                                        if (winner == PlayerSide::PLAYER_ONE) {
                                            // Player 1 wins
                                            p1_new_rating_adjusted = p1_rating_adjusted + static_cast<int>(K_FACTOR * (1 - expected_p1));
                                            p2_new_rating_adjusted = p2_rating_adjusted + static_cast<int>(K_FACTOR * (0 - expected_p2));
                                            std::cout << "Player 1 (" << player1_conn->username << ") wins." << std::endl;
                                        } else if (winner == PlayerSide::PLAYER_TWO) {
                                            // Player 2 wins
                                            p1_new_rating_adjusted = p1_rating_adjusted + static_cast<int>(K_FACTOR * (0 - expected_p1));
                                            p2_new_rating_adjusted = p2_rating_adjusted + static_cast<int>(K_FACTOR * (1 - expected_p2));
                                            std::cout << "Player 2 (" << player2_conn->username << ") wins." << std::endl;
                                        } else {
                                            // Draw
                                            p1_new_rating_adjusted = p1_rating_adjusted + static_cast<int>(K_FACTOR * (0.5 - expected_p1));
                                            p2_new_rating_adjusted = p2_rating_adjusted + static_cast<int>(K_FACTOR * (0.5 - expected_p2));
                                            std::cout << "Game is a draw." << std::endl;
                                        }

                                        // Subtract 1000 adjustment and clamp to 0 minimum
                                        int p1_new_rating = std::max(0, p1_new_rating_adjusted - 1000);
                                        int p2_new_rating = std::max(0, p2_new_rating_adjusted - 1000);

                                        sqlite3* db;
                                        if (sqlite3_open("bayou_bonanza.db", &db) == SQLITE_OK) {
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
                                        } else {
                                            std::cerr << "Failed to open database for rating update" << std::endl;
                                        }
                                    }
                                }
                            } else {
                                std::cout << "Move failed: " << result.message << std::endl;
                                sendMoveRejection(client, result.message);
                            }
                        });
                    } else {
                        sendMoveRejection(client, "Game not properly initialized");
                    }
                } else {
                    std::cerr << "Error deserializing move data from " 
                              << client->socket.getRemoteAddress() << std::endl;
                }
            } else if (messageType == MessageType::CardPlayToServer) {
                CardPlayData cardPlayData;
                if (packet >> cardPlayData) {
                    std::cout << "Card play received: card " << cardPlayData.cardIndex 
                              << " at (" << cardPlayData.targetX << ", " << cardPlayData.targetY 
                              << ") from " << client->socket.getRemoteAddress() << std::endl;
                    
                    auto session = client->session.lock();
                    if (!session) {
                        sendCardPlayRejection(client, "Not in a game");
                        continue;
                    }
                    
                    // Verify it's the client's turn
                    if (client->playerSide != session->gameState.getActivePlayer()) {
                        sendCardPlayRejection(client, "Not your turn");
                        continue;
                    }
                    
                    // Process the card play using TurnManager
                    if (session->turnManager) {
                        Position targetPosition(cardPlayData.targetX, cardPlayData.targetY);
                        bool cardPlayProcessed = false;
                        std::string resultMessage;
                        
                        session->turnManager->processPlayCardAction(cardPlayData.cardIndex, targetPosition,
                            [&](const ActionResult& result) {
                                cardPlayProcessed = true;
                                resultMessage = result.message;
                                
                                if (result.success) {
                                    std::cout << "Card play processed successfully: " << result.message << std::endl;
                                    // Broadcast updated game state to all clients
                                    broadcastGameState(session);
                                    
                                    // Check for game over (same logic as move handling)
                                    if (gameRules.isGameOver(session->gameState)) {
                                        std::cout << "Game Over detected after card play." << std::endl;
                                        // Cleanup finished session so clients won't auto-resume
                                        {
                                            auto session_to_cleanup = session;
                                            std::thread([session_to_cleanup]() {
                                                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                                {
                                                    std::lock_guard<std::mutex> lock(gamesMutex);
                                                    gameSessions.erase(std::remove_if(gameSessions.begin(), gameSessions.end(),
                                                        [&](const std::shared_ptr<GameSession>& s){ return s.get() == session_to_cleanup.get(); }),
                                                        gameSessions.end());
                                                }
                                                if (session_to_cleanup->player1) session_to_cleanup->player1->session.reset();
                                                if (session_to_cleanup->player2) session_to_cleanup->player2->session.reset();
                                            }).detach();
                                        }
                                    }
                                } else {
                                    std::cout << "Card play failed: " << result.message << std::endl;
                                    sendCardPlayRejection(client, result.message);
                                }
                            });
                        
                        // If no callback was called (shouldn't happen), handle as error
                        if (!cardPlayProcessed) {
                            sendCardPlayRejection(client, "Card play processing failed");
                        }
                    } else {
                        sendCardPlayRejection(client, "Game not properly initialized");
                    }
                } else {
                    std::cerr << "Error deserializing card play data from " 
                              << client->socket.getRemoteAddress() << std::endl;
                }
            } else if (messageType == MessageType::SaveDeck) {
                std::string deckStr;
                if (packet >> deckStr) {
                    Deck newDeck;
                    if (newDeck.deserialize(deckStr)) {
                        std::cout << "Deck deserialized successfully. Size: " << newDeck.size() << " cards" << std::endl;
                        if (newDeck.isValidForEditing()) {
                            std::cout << "Deck validation passed for editing" << std::endl;
                            client->deck = std::move(newDeck);
                            bool saveSuccessful = false;

                        sqlite3* db;
                        if (sqlite3_open("bayou_bonanza.db", &db) == SQLITE_OK) {
                            const char* sql_upd = "REPLACE INTO decks (username, deck) VALUES (?, ?);";
                            sqlite3_stmt* stmt;
                            if (sqlite3_prepare_v2(db, sql_upd, -1, &stmt, 0) == SQLITE_OK) {
                                sqlite3_bind_text(stmt, 1, client->username.c_str(), -1, SQLITE_STATIC);
                                sqlite3_bind_text(stmt, 2, deckStr.c_str(), -1, SQLITE_STATIC);
                                if (sqlite3_step(stmt) == SQLITE_DONE) {
                                    saveSuccessful = true;
                                    std::cout << "Deck saved successfully for user: " << client->username << std::endl;
                                } else {
                                    std::cerr << "Failed to save deck for user: " << client->username 
                                              << " - " << sqlite3_errmsg(db) << std::endl;
                                }
                                sqlite3_finalize(stmt);
                            } else {
                                std::cerr << "Failed to prepare deck save statement: " << sqlite3_errmsg(db) << std::endl;
                            }
                            sqlite3_close(db);
                        } else {
                            std::cerr << "Failed to open database for deck save" << std::endl;
                        }

                        // Send confirmation message back to client
                        sf::Packet confirmationPacket;
                        if (saveSuccessful) {
                            confirmationPacket << MessageType::DeckSaved;
                            std::cout << "Sending deck save confirmation to " << client->username << std::endl;
                        } else {
                            confirmationPacket << MessageType::Error << std::string("Failed to save deck to database");
                            std::cout << "Sending deck save error to " << client->username << std::endl;
                        }
                            client->socket.send(confirmationPacket);
                        } else {
                            // Validation failed
                            sf::Packet errorPacket;
                            errorPacket << MessageType::Error << std::string("Deck validation failed - too many copies of a card");
                            client->socket.send(errorPacket);
                            std::cerr << "Deck validation failed for " << client->username << " - too many copies" << std::endl;
                        }
                    } else {
                        // Deserialization failed
                        sf::Packet errorPacket;
                        errorPacket << MessageType::Error << std::string("Failed to deserialize deck data");
                        client->socket.send(errorPacket);
                        std::cerr << "Failed to deserialize deck data from " << client->username << std::endl;
                    }
                } else {
                    // Failed to deserialize deck string
                    sf::Packet errorPacket;
                    errorPacket << MessageType::Error << std::string("Failed to parse deck data");
                    client->socket.send(errorPacket);
                    std::cerr << "Failed to parse deck data from " << client->username << std::endl;
                }
            } else if (messageType == MessageType::EndTurn) {
                std::cout << "End turn received from " << client->socket.getRemoteAddress() << std::endl;
                
                // Verify it's the client's turn
                auto session = client->session.lock();
                if (!session) {
                    std::cout << "EndTurn rejected: not in game" << std::endl;
                    continue;
                }
                if (client->playerSide != session->gameState.getActivePlayer()) {
                    std::cout << "EndTurn rejected: Not your turn" << std::endl;
                    continue;
                }

                // Process the phase advance using TurnManager
                if (session->turnManager) {
                    bool phaseAdvanced = false;
                    std::string resultMessage;

                    session->turnManager->nextPhase([&](const ActionResult& result) {
                        phaseAdvanced = true;
                        resultMessage = result.message;
                        
                        if (result.success) {
                            std::cout << "Phase advanced successfully: " << result.message << std::endl;
                            // Broadcast updated game state to all clients
                            broadcastGameState(session);
                            // If the phase advance resulted in game over, cleanup session
                            if (gameRules.isGameOver(session->gameState)) {
                                std::cout << "Game Over detected after phase advance." << std::endl;
                                auto session_to_cleanup = session;
                                std::thread([session_to_cleanup]() {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                                    {
                                        std::lock_guard<std::mutex> lock(gamesMutex);
                                        gameSessions.erase(std::remove_if(gameSessions.begin(), gameSessions.end(),
                                            [&](const std::shared_ptr<GameSession>& s){ return s.get() == session_to_cleanup.get(); }),
                                            gameSessions.end());
                                    }
                                    if (session_to_cleanup->player1) session_to_cleanup->player1->session.reset();
                                    if (session_to_cleanup->player2) session_to_cleanup->player2->session.reset();
                                }).detach();
                            }
                        } else {
                            std::cout << "Phase advance failed: " << result.message << std::endl;
                        }
                    });
                    
                    // If no callback was called (shouldn't happen), handle as error
                    if (!phaseAdvanced) {
                        std::cout << "Phase advance processing failed" << std::endl;
                    }
                } else {
                    std::cout << "Game not properly initialized for phase advance" << std::endl;
                }
            } else if (messageType == MessageType::RequestMatchmaking) {
                std::cout << "Matchmaking request received from " << client->username << std::endl;
                client->lookingForMatch = true;
                
                // Send WaitingForOpponent message to the client
                sf::Packet waitingPacket;
                waitingPacket << MessageType::WaitingForOpponent;
                client->socket.send(waitingPacket);
                
                // Try to start matchmaking
                tryStartMatchmaking();
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
    // Use path relative to project root since working directory is now set to base directory
    std::string db_path = "bayou_bonanza.db"; // Database in project root
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

    const char* sql_create_users =
        "CREATE TABLE IF NOT EXISTS users ("
        "username TEXT PRIMARY KEY NOT NULL,"
        "rating INTEGER NOT NULL DEFAULT 0"
        ");";

    const char* sql_create_collections =
        "CREATE TABLE IF NOT EXISTS collections ("
        "username TEXT PRIMARY KEY NOT NULL,"
        "cards TEXT"
        ");";

    const char* sql_create_decks =
        "CREATE TABLE IF NOT EXISTS decks ("
        "username TEXT PRIMARY KEY NOT NULL,"
        "deck TEXT"
        ");";

    rc = sqlite3_exec(db, sql_create_users, 0, 0, &err_msg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg); // Free error message
    } else {
        std::cout << "Table 'users' created successfully or already exists" << std::endl;
    }

    rc = sqlite3_exec(db, sql_create_collections, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }

    rc = sqlite3_exec(db, sql_create_decks, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }

    sqlite3_close(db);
}

int main() {
    initialize_database(); // Initialize the database at the start

    // Initialize global PieceFactory for piece creation (needed for card play)
    if (!globalPieceDefManager.loadDefinitions("assets/data/cards.json")) {
        std::cerr << "FATAL: Could not load piece definitions from assets/data/cards.json" << std::endl;
        return -1;
    }
    globalPieceFactory = std::make_unique<PieceFactory>(globalPieceDefManager);

    // Set the global PieceFactory for Square deserialization and card play
    Square::setGlobalPieceFactory(globalPieceFactory.get());

    // Initialize the GameInitializer with the loaded PieceDefinitionManager and PieceFactory
    gameInitializer = std::make_unique<GameInitializer>(globalPieceDefManager, *globalPieceFactory);

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
            {
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
                            int rc_db = sqlite3_open("bayou_bonanza.db", &db);
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
                                            sqlite3_bind_int(stmt_insert, 2, 0); // Default rating
                                            if (sqlite3_step(stmt_insert) == SQLITE_DONE) {
                                                new_client_conn->rating = 0;
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
                                // Load or create card collection and deck
                                const char* sql_select_collection = "SELECT cards FROM collections WHERE username = ?;";
                                const char* sql_insert_collection = "INSERT INTO collections (username, cards) VALUES (?, ?);";
                                const char* sql_select_deck = "SELECT deck FROM decks WHERE username = ?;";
                                const char* sql_insert_deck = "INSERT INTO decks (username, deck) VALUES (?, ?);";

                                std::string collectionStr;
                                std::string deckStr;

                                sqlite3_stmt* stmt_coll;
                                if (sqlite3_prepare_v2(db, sql_select_collection, -1, &stmt_coll, 0) == SQLITE_OK) {
                                    sqlite3_bind_text(stmt_coll, 1, received_username.c_str(), -1, SQLITE_STATIC);
                                    if (sqlite3_step(stmt_coll) == SQLITE_ROW) {
                                        const unsigned char* text = sqlite3_column_text(stmt_coll, 0);
                                        if (text) collectionStr = reinterpret_cast<const char*>(text);
                                    }
                                    sqlite3_finalize(stmt_coll);
                                }

                                if (collectionStr.empty()) {
                                    auto starter = CardFactory::createStarterDeck();
                                    CardCollection cc(std::move(starter));
                                    collectionStr = cc.serialize();
                                    sqlite3_stmt* stmt_ins;
                                    if (sqlite3_prepare_v2(db, sql_insert_collection, -1, &stmt_ins, 0) == SQLITE_OK) {
                                        sqlite3_bind_text(stmt_ins, 1, received_username.c_str(), -1, SQLITE_STATIC);
                                        sqlite3_bind_text(stmt_ins, 2, collectionStr.c_str(), -1, SQLITE_STATIC);
                                        sqlite3_step(stmt_ins);
                                        sqlite3_finalize(stmt_ins);
                                    }
                                }

                                sqlite3_stmt* stmt_deck;
                                if (sqlite3_prepare_v2(db, sql_select_deck, -1, &stmt_deck, 0) == SQLITE_OK) {
                                    sqlite3_bind_text(stmt_deck, 1, received_username.c_str(), -1, SQLITE_STATIC);
                                    if (sqlite3_step(stmt_deck) == SQLITE_ROW) {
                                        const unsigned char* text = sqlite3_column_text(stmt_deck, 0);
                                        if (text) deckStr = reinterpret_cast<const char*>(text);
                                    }
                                    sqlite3_finalize(stmt_deck);
                                }

                                if (deckStr.empty()) {
                                    auto starter = CardFactory::createStarterDeck();
                                    Deck d(std::move(starter));
                                    deckStr = d.serialize();
                                    sqlite3_stmt* stmt_ins;
                                    if (sqlite3_prepare_v2(db, sql_insert_deck, -1, &stmt_ins, 0) == SQLITE_OK) {
                                        sqlite3_bind_text(stmt_ins, 1, received_username.c_str(), -1, SQLITE_STATIC);
                                        sqlite3_bind_text(stmt_ins, 2, deckStr.c_str(), -1, SQLITE_STATIC);
                                        sqlite3_step(stmt_ins);
                                        sqlite3_finalize(stmt_ins);
                                    }
                                }

                                new_client_conn->collection.deserialize(collectionStr);
                                new_client_conn->deck.deserialize(deckStr);

                                sqlite3_close(db);
                                new_client_conn->username = received_username;
                                login_successful = true;

                                // Check for existing game session for this user
                                auto existingSession = findGameSessionByUsername(new_client_conn->username);
                                if (existingSession && !gameRules.isGameOver(existingSession->gameState)) {
                                    std::cout << "Reconnecting user " << new_client_conn->username << " to ongoing game" << std::endl;
                                    if (existingSession->player1 && existingSession->player1->username == new_client_conn->username) {
                                        existingSession->player1 = new_client_conn;
                                        new_client_conn->playerSide = PlayerSide::PLAYER_ONE;
                                    } else if (existingSession->player2 && existingSession->player2->username == new_client_conn->username) {
                                        existingSession->player2 = new_client_conn;
                                        new_client_conn->playerSide = PlayerSide::PLAYER_TWO;
                                    }
                                    new_client_conn->session = existingSession;

                                    new_client_conn->socket.setBlocking(false);

                                    // Send PlayerAssignment
                                    sf::Packet assignmentPacket;
                                    assignmentPacket << MessageType::PlayerAssignment << new_client_conn->playerSide;
                                    new_client_conn->socket.send(assignmentPacket);

                                    // Send collection and deck
                                    sf::Packet collectionPacket;
                                    collectionPacket << MessageType::CardCollectionData << new_client_conn->collection.serialize();
                                    new_client_conn->socket.send(collectionPacket);

                                    sf::Packet deckPacket;
                                    deckPacket << MessageType::DeckData << new_client_conn->deck.serialize();
                                    new_client_conn->socket.send(deckPacket);

                                    // Send current game state as GameStart packet (acts as resume)
                                    sf::Packet gameStartPacket;
                                    std::string p1_username = existingSession->player1->username;
                                    int p1_rating = existingSession->player1->rating;
                                    std::string p2_username = existingSession->player2->username;
                                    int p2_rating = existingSession->player2->rating;
                                    gameStartPacket << MessageType::GameStart << p1_username << p1_rating
                                                    << p2_username << p2_rating << existingSession->gameState;
                                    new_client_conn->socket.send(gameStartPacket);

                                    clients.push_back(new_client_conn);
                                    client_threads.emplace_back(handle_client, new_client_conn);
                                    continue; // Skip normal post-login flow
                                }
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

                // Assign default PlayerSide until matchmaking
                new_client_conn->playerSide = PlayerSide::NEUTRAL;
                clients.push_back(new_client_conn);
                
                std::string sideStr = "Neutral";
                if (new_client_conn->playerSide == PlayerSide::PLAYER_ONE) sideStr = "One";
                else if (new_client_conn->playerSide == PlayerSide::PLAYER_TWO) sideStr = "Two";
                std::cout << "User '" << new_client_conn->username << "' (Rating: " << new_client_conn->rating
                          << ") connected as Player " << sideStr
                          << " from " << new_client_conn->socket.getRemoteAddress() << std::endl;
                std::cout << "Current players connected: " << clients.size() << std::endl;
                
                // Send PlayerAssignment
                sf::Packet assignmentPacket;
                assignmentPacket << MessageType::PlayerAssignment << new_client_conn->playerSide;
                new_client_conn->socket.send(assignmentPacket);

                // Send player collection and deck
                sf::Packet collectionPacket;
                collectionPacket << MessageType::CardCollectionData << new_client_conn->collection.serialize();
                new_client_conn->socket.send(collectionPacket);

                sf::Packet deckPacket;
                deckPacket << MessageType::DeckData << new_client_conn->deck.serialize();
                new_client_conn->socket.send(deckPacket);

                // Don't automatically start games - wait for explicit matchmaking requests
                std::cout << "Player connected. Total players: " << clients.size() << std::endl;
                
                // Start thread for each client to handle their messages
                client_threads.emplace_back(handle_client, new_client_conn);

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
