#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <memory>
#include "GameState.h"
#include "Piece.h"
#include "Move.h"
#include "PlayerSide.h"
#include "NetworkProtocol.h"

// Forward declaration
namespace BayouBonanza {
    class GraphicsManager;
}

namespace BayouBonanza {

/**
 * @brief Manages all input handling for the game client
 * 
 * This class handles mouse events, piece selection, drag and drop,
 * move validation, and communication with the server.
 */
class InputManager {
public:
    /**
     * @brief Constructor
     * 
     * @param window Reference to the SFML render window
     * @param socket Reference to the network socket for server communication
     * @param gameState Reference to the current game state
     * @param gameHasStarted Reference to game start flag
     * @param myPlayerSide Reference to the player's assigned side
     * @param graphicsManager Reference to the graphics manager for coordinate conversion
     */
    InputManager(sf::RenderWindow& window, 
                 sf::TcpSocket& socket,
                 GameState& gameState,
                 bool& gameHasStarted,
                 PlayerSide& myPlayerSide,
                 GraphicsManager& graphicsManager);

    /**
     * @brief Process a single SFML event
     * 
     * @param event The SFML event to process
     * @return true if the event was handled, false otherwise
     */
    bool handleEvent(const sf::Event& event);

    /**
     * @brief Get the currently selected piece
     * 
     * @return Pointer to the selected piece, or nullptr if none selected
     */
    Piece* getSelectedPiece() const;

    /**
     * @brief Check if a piece is currently selected
     * 
     * @return true if a piece is selected
     */
    bool isPieceSelected() const;

    /**
     * @brief Get the original coordinates of the selected piece
     * 
     * @return Board coordinates of the selected piece
     */
    sf::Vector2i getOriginalSquareCoords() const;

    /**
     * @brief Get the current mouse position for dragging (in game coordinates)
     * 
     * @return Current mouse position in game coordinates
     */
    sf::Vector2f getCurrentMousePosition() const;

    /**
     * @brief Get the mouse offset for proper piece dragging
     * 
     * @return Mouse offset from piece top-left corner
     */
    sf::Vector2f getMouseOffset() const;

    /**
     * @brief Reset the input state (deselect piece, clear selection)
     */
    void resetInputState();

private:
    // References to external objects
    sf::RenderWindow& window;
    sf::TcpSocket& socket;
    GameState& gameState;
    bool& gameHasStarted;
    PlayerSide& myPlayerSide;
    GraphicsManager& graphicsManager;

    // Input state
    Piece* selectedPiece;
    sf::Vector2i originalSquareCoords;
    sf::Vector2f mouseOffset;
    bool pieceSelected;
    sf::Vector2f currentMousePosition; // In game coordinates

    // Helper methods
    /**
     * @brief Convert game coordinates to board coordinates
     * 
     * @param gamePos Game position
     * @return Board coordinates, or (-1, -1) if outside board
     */
    sf::Vector2i gamePosToBoard(const sf::Vector2f& gamePos) const;

    /**
     * @brief Handle mouse button press events
     * 
     * @param event The mouse button press event
     */
    void handleMouseButtonPressed(const sf::Event& event);

    /**
     * @brief Handle mouse movement events
     * 
     * @param event The mouse move event
     */
    void handleMouseMoved(const sf::Event& event);

    /**
     * @brief Handle mouse button release events
     * 
     * @param event The mouse button release event
     */
    void handleMouseButtonReleased(const sf::Event& event);

    /**
     * @brief Handle piece selection
     * 
     * @param boardX Board X coordinate
     * @param boardY Board Y coordinate
     * @param gameMousePos Game mouse position
     */
    void selectPiece(int boardX, int boardY, const sf::Vector2f& gameMousePos);

    /**
     * @brief Attempt to move the selected piece
     * 
     * @param targetX Target board X coordinate
     * @param targetY Target board Y coordinate
     */
    void attemptMove(int targetX, int targetY);

    /**
     * @brief Send a move to the server
     * 
     * @param move The move to send
     */
    void sendMoveToServer(const Move& move);
};

} // namespace BayouBonanza 