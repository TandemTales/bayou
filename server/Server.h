#ifndef SERVER_H
#define SERVER_H

#include <SFML/Network.hpp>
#include <array>
// Removed <vector> as it's not strictly needed if listener.accept directly uses the array indices
#include <map>
#include <memory>
#include <iostream> 

// Game logic headers
#include "GameState.h"
#include "GameInitializer.h"
#include "TurnManager.h"
#include "GameRules.h"
#include "MoveExecutor.h"
#include "PlayerSide.h" 
#include "NetworkProtocol.h" 
#include "Piece.h"
#include "Move.h" // Required for constructing Move objects

class Server {
public:
    Server(unsigned short port);
    void waitForClients();

private:
    void startGameSession();
    void gameLoop(); // Replaces placeholder while loop, handles packet processing and game logic
    void handlePlayerAction(BayouBonanza::PlayerSide senderSide, sf::Packet& packet);
    void broadcastGameStateUpdate();
    void sendActionInvalid(BayouBonanza::PlayerSide targetSide, const BayouBonanza::Network::NetworkMove& moveData, const std::string& reason);
    void handleDisconnect(BayouBonanza::PlayerSide disconnectedPlayer);
    void broadcastGameOver(BayouBonanza::PlayerSide winner, BayouBonanza::GameResult result);


    void sendPacketToClient(BayouBonanza::PlayerSide player, sf::Packet& packet);
    void broadcastPacket(sf::Packet& packet);
    BayouBonanza::Network::PieceType getNetworkPieceType(const std::string& typeName) const;

    sf::TcpListener listener;
    std::array<sf::TcpSocket, 2> clientSockets;
    std::map<BayouBonanza::PlayerSide, sf::TcpSocket*> playerConnections;
    std::map<sf::TcpSocket*, BayouBonanza::PlayerSide> socketToPlayerMap; // Reverse mapping for convenience
    
    bool listening;
    int connectedClientsCount;
    bool gameRunning; // To control the main game loop

    // Game logic components
    std::unique_ptr<BayouBonanza::GameState> gameState;
    std::unique_ptr<BayouBonanza::GameInitializer> gameInitializer;
    std::unique_ptr<BayouBonanza::TurnManager> turnManager;
    std::unique_ptr<BayouBonanza::GameRules> gameRules;
    std::unique_ptr<BayouBonanza::MoveExecutor> moveExecutor;
    // std::unique_ptr<BayouBonanza::GameOverDetector> gameOverDetector; // If we had one
};

#endif // SERVER_H
