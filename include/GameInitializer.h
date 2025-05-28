#pragma once

#include "GameState.h"
#include "PlayerSide.h"
#include "Piece.h"
// #include "PieceFactory.h" // Forward-declared or included below if unique_ptr needs full def
#include "PieceDefinitionManager.h" // Added
#include "PieceFactory.h"           // Added (ensuring full definition for unique_ptr)


namespace BayouBonanza {
// Forward declare PieceFactory if only pointers/references are used in header
// class PieceFactory; // Not needed if included above
// class PieceDefinitionManager; // Already included

/**
 * @brief Responsible for initializing a new game
 * 
 * This class handles setting up the initial game state, placing starting
 * pieces, and configuring default values.
 */
class GameInitializer {
public:
    /**
     * @brief Constructor
     */
    GameInitializer() = default;
    
    /**
     * @brief Initialize a new game state
     * 
     * @param gameState Game state to initialize
     */
    void initializeNewGame(GameState& gameState);
    
    /**
     * @brief Set up the initial board configuration
     * 
     * Places the starting pieces on the board.
     * 
     * @param gameState Game state to set up
     */
    void setupBoard(GameState& gameState);

private:
    BayouBonanza::PieceDefinitionManager pieceDefManager;
    std::unique_ptr<BayouBonanza::PieceFactory> pieceFactory;

    /**
     * @brief Create a piece and place it on the board
     * 
     * @param gameState Game state to modify
     * @param pieceType Type of piece to create ("King", "Queen", etc.)
     * @param side Player side
     * @param x X-coordinate
     * @param y Y-coordinate
     * @return Raw pointer to the placed piece (ownership managed by Square via unique_ptr)
     */
    Piece* createAndPlacePiece(GameState& gameState, const std::string& pieceType, PlayerSide side, int x, int y);
    
    /**
     * @brief Reset the game state to default values
     * 
     * @param gameState Game state to reset
     */
    void resetGameState(GameState& gameState);
    
    /**
     * @brief Calculate initial control values for the board
     * 
     * @param gameState Game state to update
     */
    void calculateInitialControl(GameState& gameState);
};

} // namespace BayouBonanza
