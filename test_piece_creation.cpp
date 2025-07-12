#include <iostream>
#include "GameInitializer.h"
#include "GameState.h"
#include "PieceDefinitionManager.h"
#include "PieceFactory.h"
#include "Square.h"

using namespace BayouBonanza;

int main() {
    std::cout << "Testing piece creation and game initialization..." << std::endl;
    
    // Load piece definitions
    PieceDefinitionManager pieceDefManager;
    if (!pieceDefManager.loadDefinitions("assets/data/pieces.json")) {
        std::cerr << "FATAL: Could not load piece definitions" << std::endl;
        return -1;
    }
    std::cout << "✓ Piece definitions loaded successfully" << std::endl;
    
    // Create PieceFactory
    PieceFactory pieceFactory(pieceDefManager);
    std::cout << "✓ PieceFactory created successfully" << std::endl;
    
    // Test individual piece creation
    auto testPiece = pieceFactory.createPiece("TinkeringTom", PlayerSide::PLAYER_ONE);
    if (!testPiece) {
        std::cerr << "✗ Failed to create TinkeringTom piece" << std::endl;
        return -1;
    }
    std::cout << "✓ TinkeringTom piece created: " << testPiece->getTypeName() 
              << " (symbol: " << testPiece->getSymbol() << ")" << std::endl;
    
    // Test GameInitializer with external references
    GameInitializer gameInitializer(pieceDefManager, pieceFactory);
    std::cout << "✓ GameInitializer created with external references" << std::endl;
    
    // Initialize a game state
    GameState gameState;
    gameInitializer.initializeNewGame(gameState);
    std::cout << "✓ Game state initialized" << std::endl;
    
    // Check if pieces were placed correctly
    const GameBoard& board = gameState.getBoard();
    
    // Check a few key positions
    const Square& tomSquare = board.getSquare(4, 7); // TinkeringTom position for Player 1
    if (tomSquare.isEmpty()) {
        std::cerr << "✗ TinkeringTom not found at expected position (4,7)" << std::endl;
        return -1;
    }
    
    Piece* tom = tomSquare.getPiece();
    if (tom->getTypeName().empty()) {
        std::cerr << "✗ TinkeringTom has empty type name!" << std::endl;
        return -1;
    }
    
    std::cout << "✓ TinkeringTom found at (4,7): " << tom->getTypeName() 
              << " (symbol: " << tom->getSymbol() << ")" << std::endl;
    
    // Check a Sentroid position
    const Square& sentroidSquare = board.getSquare(0, 6); // Sentroid position for Player 1
    if (sentroidSquare.isEmpty()) {
        std::cerr << "✗ Sentroid not found at expected position (0,6)" << std::endl;
        return -1;
    }
    
    Piece* sentroid = sentroidSquare.getPiece();
    if (sentroid->getTypeName().empty()) {
        std::cerr << "✗ Sentroid has empty type name!" << std::endl;
        return -1;
    }
    
    std::cout << "✓ Sentroid found at (0,6): " << sentroid->getTypeName() 
              << " (symbol: " << sentroid->getSymbol() << ")" << std::endl;
    
    std::cout << "\n✅ All tests passed! Piece creation and game initialization working correctly." << std::endl;
    
    return 0;
} 