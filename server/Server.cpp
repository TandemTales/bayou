#include "Server.h"
#include "NetworkProtocol.h"
#include "GameBoard.h"
#include "Square.h"
#include "Piece.h"
#include "Move.h" // Make sure this is included for BayouBonanza::Move
#include <iostream>
#include <string> // For std::string in sendActionInvalid

// Using directives for convenience
using BayouBonanza::PlayerSide;
using BayouBonanza::Network::MessageType;
using BayouBonanza::Network::PieceType;
using BayouBonanza::Network::NetworkMove;
using BayouBonanza::Network::operator<<; 
using BayouBonanza::Network::operator>>; 

Server::Server(unsigned short port) 
    : listening(false), connectedClientsCount(0), gameRunning(false) { // Initialize gameRunning
    gameRules = std::make_unique<BayouBonanza::GameRules>();
    moveExecutor = std::make_unique<BayouBonanza::MoveExecutor>(); 
    gameInitializer = std::make_unique<BayouBonanza::GameInitializer>();

    if (listener.listen(port) != sf::Socket::Done) {
        std::cerr << "Error: Could not listen on port " << port << std::endl;
        return;
    }
    listening = true;
    std::cout << "Server is listening on port " << port << std::endl;
}

void Server::waitForClients() {
    if (!listening) {
        std::cerr << "Error: Server is not listening. Cannot wait for clients." << std::endl;
        return;
    }
    std::cout << "Waiting for clients to connect..." << std::endl;
    while (connectedClientsCount < 2) {
        if (listener.accept(clientSockets[connectedClientsCount]) == sf::Socket::Done) {
            std::cout << "Client connected: " << clientSockets[connectedClientsCount].getRemoteAddress() 
                      << " as potential Player " << (connectedClientsCount + 1) << std::endl;
            clientSockets[connectedClientsCount].setBlocking(false); 
            connectedClientsCount++;
            std::cout << connectedClientsCount << "/2 clients connected." << std::endl;
        } else {
            std::cerr << "Error: Failed to accept new client (or listener closed)." << std::endl;
        }
    }
    std::cout << "All 2 clients connected." << std::endl;
    startGameSession();
}

BayouBonanza::Network::PieceType Server::getNetworkPieceType(const std::string& typeName) const {
    if (typeName == "King") return PieceType::KING;
    if (typeName == "Queen") return PieceType::QUEEN;
    if (typeName == "Rook") return PieceType::ROOK;
    if (typeName == "Bishop") return PieceType::BISHOP;
    if (typeName == "Knight") return PieceType::KNIGHT;
    if (typeName == "Pawn") return PieceType::PAWN;
    return PieceType::NONE;
}

void Server::sendPacketToClient(PlayerSide player, sf::Packet& packet) {
    auto it = playerConnections.find(player);
    if (it != playerConnections.end() && it->second) {
        if (it->second->send(packet) != sf::Socket::Done) {
            std::cerr << "Error sending packet to player " 
                      << (player == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
        }
    } else {
        std::cerr << "Error: No connection found for player " 
                  << (player == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
    }
}

void Server::broadcastPacket(sf::Packet& packet) {
    sf::Packet packetCopy = packet; // Ensure each client gets a fresh copy
    sendPacketToClient(PlayerSide::PLAYER_ONE, packetCopy);
    sendPacketToClient(PlayerSide::PLAYER_TWO, packet); // Second client can use the original
}

void Server::startGameSession() {
    std::cout << "Starting game session..." << std::endl;
    gameState = gameInitializer->createInitialGameState();
    if (!gameState) {
        std::cerr << "Error: Failed to initialize game state." << std::endl;
        return;
    }
    if (!gameRules) { 
        std::cerr << "Error: GameRules not initialized." << std::endl;
        return;
    }
    turnManager = std::make_unique<BayouBonanza::TurnManager>(*gameState, *gameRules);
    
    playerConnections[PlayerSide::PLAYER_ONE] = &clientSockets[0];
    playerConnections[PlayerSide::PLAYER_TWO] = &clientSockets[1];
    socketToPlayerMap[&clientSockets[0]] = PlayerSide::PLAYER_ONE;
    socketToPlayerMap[&clientSockets[1]] = PlayerSide::PLAYER_TWO;

    sf::Packet p1IdentityPacket;
    PlayerSide p1Side = PlayerSide::PLAYER_ONE;
    p1IdentityPacket << static_cast<sf::Uint8>(MessageType::AssignPlayerIdentity) << p1Side;
    sendPacketToClient(PlayerSide::PLAYER_ONE, p1IdentityPacket);
    std::cout << "Sent AssignPlayerIdentity (PLAYER_ONE) to client 0." << std::endl;

    sf::Packet p2IdentityPacket;
    PlayerSide p2Side = PlayerSide::PLAYER_TWO;
    p2IdentityPacket << static_cast<sf::Uint8>(MessageType::AssignPlayerIdentity) << p2Side;
    sendPacketToClient(PlayerSide::PLAYER_TWO, p2IdentityPacket);
    std::cout << "Sent AssignPlayerIdentity (PLAYER_TWO) to client 1." << std::endl;

    sf::Packet gameStartPacket;
    gameStartPacket << static_cast<sf::Uint8>(MessageType::GameStart);
    gameStartPacket << gameState->getActivePlayer();
    const auto& board = gameState->getBoard();
    for (int y = 0; y < BayouBonanza::GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < BayouBonanza::GameBoard::BOARD_SIZE; ++x) {
            const auto& square = board.getSquare(x, y);
            if (square.isEmpty()) {
                gameStartPacket << static_cast<sf::Uint8>(PieceType::NONE);
            } else {
                auto piece = square.getPiece();
                gameStartPacket << static_cast<sf::Uint8>(getNetworkPieceType(piece->getTypeName()));
                gameStartPacket << piece->getSide(); 
                gameStartPacket << static_cast<sf::Int8>(piece->getHealth());
            }
        }
    }
    gameStartPacket << static_cast<sf::Int32>(gameState->getSteam(PlayerSide::PLAYER_ONE));
    gameStartPacket << static_cast<sf::Int32>(gameState->getSteam(PlayerSide::PLAYER_TWO));
    broadcastPacket(gameStartPacket);
    std::cout << "Sent GameStart to both clients." << std::endl;

    gameRunning = true;
    std::cout << "Game session started. Server is now entering the game loop." << std::endl;
    gameLoop(); // Start the main game loop
}

void Server::gameLoop() {
    while (gameRunning) {
        for (auto& pair : playerConnections) {
            PlayerSide currentSide = pair.first;
            sf::TcpSocket* clientSocket = pair.second;
            
            if (!clientSocket) continue;

            sf::Packet receivedPacket;
            sf::Socket::Status status = clientSocket->receive(receivedPacket);

            switch (status) {
                case sf::Socket::Done: {
                    sf::Uint8 msgTypeInt;
                    if (!(receivedPacket >> msgTypeInt)) {
                        std::cerr << "Error: Could not extract message type from player " 
                                  << (currentSide == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
                        continue; // Or handle error more robustly
                    }
                    MessageType msgType = static_cast<MessageType>(msgTypeInt);

                    if (msgType == MessageType::PlayerAction) {
                        handlePlayerAction(currentSide, receivedPacket);
                    } else if (msgType == MessageType::HeartbeatResponse) {
                        // std::cout << "Received HeartbeatResponse from Player " << (currentSide == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
                        // Potentially reset a timeout timer for this client
                    } else {
                        std::cerr << "Received unhandled message type: " << msgTypeInt 
                                  << " from player " << (currentSide == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
                    }
                    break;
                }
                case sf::Socket::NotReady:
                    // No data from this client, continue to next or do other things
                    break;
                case sf::Socket::Disconnected:
                case sf::Socket::Error:
                    std::cerr << "Player " << (currentSide == PlayerSide::PLAYER_ONE ? "One" : "Two") 
                              << " disconnected or socket error." << std::endl;
                    handleDisconnect(currentSide);
                    break; // Exit loop for this client, gameRunning will be false
                default:
                    break; 
            }
            if (!gameRunning) break; // If a disconnect ended the game, exit outer loop too
        }
        // Minimal sleep to prevent high CPU usage if no blocking operations occur
        sf::sleep(sf::milliseconds(10)); // Adjust as needed, e.g. 10-100ms
    }
    std::cout << "Game loop ended." << std::endl;
}

void Server::handlePlayerAction(PlayerSide senderSide, sf::Packet& packet) {
    if (!gameState || !turnManager || !gameRules || !moveExecutor) {
        std::cerr << "Error: Game components not initialized in handlePlayerAction." << std::endl;
        return;
    }

    if (gameState->getActivePlayer() != senderSide) {
        std::cerr << "Player " << (senderSide == PlayerSide::PLAYER_ONE ? "One" : "Two") 
                  << " sent action, but it's Player " 
                  << (gameState->getActivePlayer() == PlayerSide::PLAYER_ONE ? "One" : "Two") << "'s turn." << std::endl;
        NetworkMove receivedMove; // Still try to deserialize for error message
        packet >> receivedMove; // Assuming operator>> for NetworkMove is defined
        sendActionInvalid(senderSide, receivedMove, "Not your turn.");
        return;
    }

    NetworkMove receivedNetMove;
    if (!(packet >> receivedNetMove)) {
        std::cerr << "Error deserializing NetworkMove from Player " << (senderSide == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
        sendActionInvalid(senderSide, {}, "Malformed move data."); // Send empty move if deserialization failed early
        return;
    }

    // Reconstruct the full Move object
    auto piece = gameState->getBoard().getSquare(receivedNetMove.from.x, receivedNetMove.from.y).getPiece();
    if (!piece) {
        sendActionInvalid(senderSide, receivedNetMove, "No piece at 'from' position.");
        return;
    }
    if (piece->getSide() != senderSide) {
        sendActionInvalid(senderSide, receivedNetMove, "Cannot move opponent's piece.");
        return;
    }

    BayouBonanza::Move gameMove(piece, receivedNetMove.from, receivedNetMove.to);

    if (gameRules->isLegalMove(*gameState, gameMove)) {
        moveExecutor->executeMove(*gameState, gameMove); // This should update piece positions, health, etc.
        
        // Check for game over immediately after move execution
        // This might be done by GameRules or a dedicated GameOverDetector
        // For now, assume GameState might have a method or TurnManager updates it.
        // Let's say executeMove or advanceTurn updates gameResult.
        // turnManager->advanceTurn(); // This might also check for game over
        
        // Simplified turn advancement and game over check:
        if (gameState->getGameResult() == BayouBonanza::GameResult::IN_PROGRESS) {
            turnManager->advanceTurn(); // Advances active player, increments turn number
            if (gameState->getGameResult() != BayouBonanza::GameResult::IN_PROGRESS) { // Check again after advanceTurn effects (like draw by turn limit)
                 broadcastGameOver(gameState->getActivePlayer(), gameState->getGameResult()); // Winner might be complex here
                 gameRunning = false;
            } else {
                 broadcastGameStateUpdate();
                 // Optionally send TurnChange if it's distinct from GameStateUpdate
                 // sf::Packet turnChangePacket;
                 // turnChangePacket << static_cast<sf::Uint8>(MessageType::TurnChange) << gameState->getActivePlayer() << gameState->getTurnNumber();
                 // broadcastPacket(turnChangePacket);
            }
        } else { // Game was already over due to the move itself
            broadcastGameOver(gameState->getActivePlayer(), gameState->getGameResult()); // Similar to above, winner logic needed
            gameRunning = false;
        }

    } else {
        sendActionInvalid(senderSide, receivedNetMove, "Illegal move.");
    }
}

void Server::broadcastGameStateUpdate() {
    if (!gameState) return;

    sf::Packet updatePacket;
    updatePacket << static_cast<sf::Uint8>(MessageType::GameStateUpdate);
    // Serialize game state (similar to GameStart)
    updatePacket << gameState->getActivePlayer();
    updatePacket << static_cast<sf::Int32>(gameState->getTurnNumber()); // Add turn number

    const auto& board = gameState->getBoard();
    for (int y = 0; y < BayouBonanza::GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < BayouBonanza::GameBoard::BOARD_SIZE; ++x) {
            const auto& square = board.getSquare(x, y);
            if (square.isEmpty()) {
                updatePacket << static_cast<sf::Uint8>(PieceType::NONE);
            } else {
                auto piece = square.getPiece();
                updatePacket << static_cast<sf::Uint8>(getNetworkPieceType(piece->getTypeName()));
                updatePacket << piece->getSide();
                updatePacket << static_cast<sf::Int8>(piece->getHealth());
            }
        }
    }
    updatePacket << static_cast<sf::Int32>(gameState->getSteam(PlayerSide::PLAYER_ONE));
    updatePacket << static_cast<sf::Int32>(gameState->getSteam(PlayerSide::PLAYER_TWO));
    // (Optional: GamePhase, GameResult if useful for general updates)
    // updatePacket << static_cast<sf::Uint8>(gameState->getGamePhase());
    // updatePacket << static_cast<sf::Uint8>(gameState->getGameResult());


    broadcastPacket(updatePacket);
    std::cout << "Broadcasted GameStateUpdate. Turn: " << gameState->getTurnNumber() 
              << " Active: " << (gameState->getActivePlayer() == PlayerSide::PLAYER_ONE ? "P1" : "P2") << std::endl;
}

void Server::sendActionInvalid(PlayerSide targetSide, const NetworkMove& moveData, const std::string& reason) {
    sf::Packet invalidPacket;
    invalidPacket << static_cast<sf::Uint8>(MessageType::ActionInvalid);
    invalidPacket << moveData; // Send back the move that failed
    invalidPacket << reason;   // Send the reason string
    
    sendPacketToClient(targetSide, invalidPacket);
    std::cout << "Sent ActionInvalid to Player " << (targetSide == PlayerSide::PLAYER_ONE ? "One" : "Two")
              << " for move from (" << moveData.from.x << "," << moveData.from.y 
              << ") to (" << moveData.to.x << "," << moveData.to.y << "). Reason: " << reason << std::endl;
}

void Server::broadcastGameOver(PlayerSide winner, BayouBonanza::GameResult result) {
    sf::Packet gameOverPacket;
    gameOverPacket << static_cast<sf::Uint8>(MessageType::GameOver);
    gameOverPacket << winner; // The player who won, or NEUTRAL for draw
    gameOverPacket << static_cast<sf::Uint8>(result); // GameResult enum
    
    broadcastPacket(gameOverPacket);
    std::cout << "Broadcasted GameOver. Winner: " << (winner == PlayerSide::PLAYER_ONE ? "P1" : (winner == PlayerSide::PLAYER_TWO ? "P2" : "Neutral"))
              << " Result: " << static_cast<int>(result) << std::endl;
}

void Server::handleDisconnect(PlayerSide disconnectedPlayer) {
    std::cout << "Handling disconnect for Player " << (disconnectedPlayer == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
    if (!gameRunning) return; // Already handled or game ended

    gameRunning = false; // Stop the game loop

    // Determine the winner (the other player)
    PlayerSide winner = (disconnectedPlayer == PlayerSide::PLAYER_ONE) ? PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
    BayouBonanza::GameResult gameResult;
    if (winner == PlayerSide::PLAYER_ONE) {
        gameResult = BayouBonanza::GameResult::PLAYER_ONE_WIN;
    } else {
        gameResult = BayouBonanza::GameResult::PLAYER_TWO_WIN;
    }
    
    // Update game state if it exists
    if (gameState) {
        gameState->setGameResult(gameResult);
        // gameState->setWinner(winner); // If GameState has such a method
    }

    // Notify the other player
    PlayerSide remainingPlayer = (disconnectedPlayer == PlayerSide::PLAYER_ONE) ? PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
    auto it = playerConnections.find(remainingPlayer);
    if (it != playerConnections.end() && it->second) {
        sf::Packet disconnectNoticePacket;
        // Using GameOver, but could be a custom "OpponentDisconnected" message
        disconnectNoticePacket << static_cast<sf::Uint8>(MessageType::GameOver); 
        disconnectNoticePacket << winner;
        disconnectNoticePacket << static_cast<sf::Uint8>(gameResult); // Or a specific "OpponentLeft" result
        it->second->send(disconnectNoticePacket); // Send directly, not using sendPacketToClient to avoid issues if map is modified
        std::cout << "Notified Player " << (remainingPlayer == PlayerSide::PLAYER_ONE ? "One" : "Two") 
                  << " about opponent's disconnect." << std::endl;
    }
    
    // Clean up sockets? SFML sockets close on destruction.
    // playerConnections.clear(); // Or selectively remove
    // socketToPlayerMap.clear();
}

// Ensure main.cpp or where Server is run calls waitForClients, which then calls startGameSession, then gameLoop.
// Destructor for Server could be added if specific cleanup for game logic pointers is needed,
// but unique_ptr should handle them.
Server::~Server() {
    std::cout << "Server shutting down." << std::endl;
    // Ensure gameRunning is false to stop any loops if not already stopped.
    gameRunning = false; 
    // Sockets will close on their own.
    // Game logic components managed by unique_ptr will be deleted.
}
