#ifndef CLIENT_H
#define CLIENT_H

#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <string>
#include <iostream> // For logging
#include <optional> // For std::optional

// Game logic and protocol headers
#include "NetworkProtocol.h"
#include "GameState.h"
#include "GameBoard.h"
#include "Piece.h"  // For PieceType for textures and piece rendering, and Position
#include "Square.h"
#include "PlayerSide.h"

// Using directives for convenience within the class, if desired, or fully qualify.
// For headers, it's best to use fully qualified names or forward declarations if possible.

class Client {
public:
    Client(const std::string& serverIp, unsigned short port);
    void run();

private:
    void connectToServer(const std::string& serverIp, unsigned short port);
    void processServerPacket(sf::Packet& packet);
    void render();
    // Modified to take the specific mouse event
    void handlePlayerInput(const sf::Event::MouseButtonEvent& event); 
    void loadPieceTextures();
    void setupWindow(); // Helper for window initialization

    // Helper to convert screen coordinates to board coordinates
    BayouBonanza::Position screenToBoardCoords(int mouseX, int mouseY) const;

    // Network
    sf::TcpSocket socket;
    bool connected;

    // SFML Window & Rendering
    sf::RenderWindow window;
    sf::Font font; // For UI text
    std::map<BayouBonanza::Network::PieceType, sf::Texture> pieceTextures;
    // Add textures for board, UI elements if necessary

    // Game State & Input Handling
    std::unique_ptr<BayouBonanza::GameState> gameState; 
    BayouBonanza::PlayerSide myPlayerSide;
    bool isMyTurn;
    bool gameActive; 

    std::optional<BayouBonanza::Position> selectedSquare; // Stores the currently selected square
    // std::vector<BayouBonanza::Position> possibleMovesForSelectedPiece; // Optional for highlighting

    // Helper to convert PieceType from NetworkProtocol to a string for texture loading, if needed
    std::string pieceTypeToString(BayouBonanza::Network::PieceType type); 
    BayouBonanza::Network::PieceType getNetworkPieceTypeFromString(const std::string& typeName);


    // Drawing helpers
    void drawBoard();
    void drawPieces();
    void drawUI();
    void drawHighlights(); // For selected square and possible moves
};

#endif // CLIENT_H
