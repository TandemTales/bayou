#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <memory>
#include <string>
#include <map>

#include "GraphicsManager.h"
#include "GameState.h"
#include "InputManager.h"
#include "PieceDefinitionManager.h"
#include "PieceFactory.h"
#include "CardCollection.h"
#include "Deck.h"
#include "PlayerSide.h"
#include "GameInitializer.h"
#include "GameOverDetector.h"
#include "UIManager.h"
#include "NetworkManager.h" // Include the new NetworkManager header

namespace BayouBonanza {

class Game {
public:
    Game();
    ~Game();
    void run();

private:
    // --- Core Methods ---
    bool initialize();
    void runGameLoop();
    void processEvents();
    void processNetworkMessages();
    void update(float dt);
    void render();
    void cleanup();

    // --- Helper Methods ---
    void handleGameStart(sf::Packet& packet);
    void renderBoard();
    void renderPieces();
    void renderPiece(const Piece* piece, sf::Vector2f position);
    void renderHighlights();
    void renderDraggedCard();
    void onWinCondition(PlayerSide winner, const std::string& description);

    // Static callback setup
    static Game* s_pInstance;
    static void gameOverCallback(PlayerSide winner, const std::string& description);


    // --- Core Components ---
    sf::RenderWindow m_window;
    GraphicsManager m_graphicsManager;
    GameState m_gameState;
    InputManager m_inputManager;
    std::unique_ptr<UIManager> m_uiManager;
    std::unique_ptr<NetworkManager> m_networkManager; // NetworkManager instance

    // --- Game State Variables ---
    bool m_gameHasStarted;
    PlayerSide m_myPlayerSide;
    GameInitializer m_gameInitializer;
    GameOverDetector m_gameOverDetector;

    // --- Asset and Definition Managers ---
    PieceDefinitionManager m_pieceDefManager;
    std::unique_ptr<PieceFactory> m_pieceFactory;
    std::map<std::string, sf::Texture> m_pieceTextures;
    sf::Font m_globalFont;

    // --- Player's Cards ---
    CardCollection m_myCollection;
    Deck m_myDeck;

    // --- State variables ---
    bool m_returnToMenuRequested;
    int m_myCurrentRating;
    std::string m_myUsername;
    std::string m_opponentUsername;

    // These are related to the menu->game transition and need a better home
    bool m_gameStartReceived;
    sf::Packet m_gameStartPacketData;
};

} // namespace BayouBonanza
