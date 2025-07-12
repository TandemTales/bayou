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
     * @brief Constructor that creates its own PieceDefinitionManager and PieceFactory
     */
    GameInitializer();
    
    /**
     * @brief Constructor that uses external PieceDefinitionManager and PieceFactory
     * 
     * @param pieceDefManager Reference to an already loaded PieceDefinitionManager
     * @param pieceFactory Reference to an already created PieceFactory
     */
    GameInitializer(const PieceDefinitionManager& pieceDefManager, PieceFactory& pieceFactory);
    
    /**
     * @brief Initialize a new game state
     * 
     * @param gameState Game state to initialize
     */
    void initializeNewGame(GameState& gameState);

    /**
     * @brief Initialize a new game state with custom decks
     */
    void initializeNewGame(GameState& gameState, const Deck& deck1, const Deck& deck2);
    
    /**
     * @brief Set up the initial board configuration
     * 
     * Places the starting pieces on the board.
     * 
     * @param gameState Game state to set up
     */
    void setupBoard(GameState& gameState);

private:
    // For constructor without parameters - owns the instances
    std::unique_ptr<BayouBonanza::PieceDefinitionManager> ownedPieceDefManager;
    std::unique_ptr<BayouBonanza::PieceFactory> ownedPieceFactory;
    
    // References to the actual instances to use (either owned or external)
    const BayouBonanza::PieceDefinitionManager* pieceDefManager;
    BayouBonanza::PieceFactory* pieceFactory;

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
