#include "GameInitializer.h"
#include "GameBoard.h"
#include "Square.h"
#include "InfluenceSystem.h"
#include <iostream> // For std::cerr

namespace BayouBonanza {

// Constructor
GameInitializer::GameInitializer() {
    if (!pieceDefManager.loadDefinitions("assets/data/pieces.json")) {
        // Handle error: Log and maybe throw or exit
        std::cerr << "FATAL: Could not load piece definitions from assets/data/pieces.json" << std::endl;
        // Consider throwing an exception or setting an error state that can be checked.
        // For now, proceeding will likely lead to issues if pieceFactory is used without definitions.
    }
    pieceFactory = std::make_unique<BayouBonanza::PieceFactory>(pieceDefManager);
}

void GameInitializer::initializeNewGame(GameState& gameState) {
    // Reset the game state to default values
    resetGameState(gameState);
    
    // Set up the board with initial pieces
    setupBoard(gameState);
    
    // Initialize the card system for both players
    gameState.initializeCardSystem();
    
    // Calculate initial control values
    calculateInitialControl(gameState);
}

void GameInitializer::initializeNewGame(GameState& gameState, const Deck& deck1, const Deck& deck2) {
    resetGameState(gameState);
    setupBoard(gameState);
    gameState.initializeCardSystem(deck1, deck2);
    calculateInitialControl(gameState);
}

void GameInitializer::setupBoard(GameState& gameState) {
    // Clear the board
    gameState.getBoard().resetBoard();
    
    // Set up Player One pieces (bottom of board)
    PlayerSide playerOne = PlayerSide::PLAYER_ONE;
    // Back row
    createAndPlacePiece(gameState, "Sweetykins", playerOne, 0, 7);
    createAndPlacePiece(gameState, "Automatick", playerOne, 1, 7);
    createAndPlacePiece(gameState, "Sidewinder", playerOne, 2, 7);
    createAndPlacePiece(gameState, "ScarlettGlumpkin", playerOne, 3, 7);
    createAndPlacePiece(gameState, "TinkeringTom", playerOne, 4, 7);
    createAndPlacePiece(gameState, "Rustbucket", playerOne, 5, 7);
    createAndPlacePiece(gameState, "Automatick", playerOne, 6, 7);
    createAndPlacePiece(gameState, "Sweetykins", playerOne, 7, 7);
    
    // Pawn row
    for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
        createAndPlacePiece(gameState, "Sentroid", playerOne, x, 6);
    }
    
    // Set up Player Two pieces (top of board)
    PlayerSide playerTwo = PlayerSide::PLAYER_TWO;
    // Back row
    createAndPlacePiece(gameState, "Sweetykins", playerTwo, 0, 0);
    createAndPlacePiece(gameState, "Automatick", playerTwo, 1, 0);
    createAndPlacePiece(gameState, "Rustbucket", playerTwo, 2, 0);
    createAndPlacePiece(gameState, "ScarlettGlumpkin", playerTwo, 3, 0);
    createAndPlacePiece(gameState, "TinkeringTom", playerTwo, 4, 0);
    createAndPlacePiece(gameState, "Sidewinder", playerTwo, 5, 0);
    createAndPlacePiece(gameState, "Automatick", playerTwo, 6, 0);
    createAndPlacePiece(gameState, "Sweetykins", playerTwo, 7, 0);
    
    // Pawn row
    for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
        createAndPlacePiece(gameState, "Sentroid", playerTwo, x, 1);
    }
}

Piece* GameInitializer::createAndPlacePiece(GameState& gameState, const std::string& pieceType, PlayerSide side, int x, int y) {
    // Create the piece using the factory
    std::unique_ptr<Piece> piece = pieceFactory->createPiece(pieceType, side);
    
    if (!piece) {
        std::cerr << "Failed to create piece: " << pieceType << std::endl;
        return nullptr; // Invalid piece type or factory error
    }
    
    // Set the piece's position
    Position pos(x, y);
    piece->setPosition(pos);
    
    // Get raw pointer before moving ownership to the square
    Piece* rawPiecePtr = piece.get(); 
    
    // Place the piece on the board
    Square& square = gameState.getBoard().getSquare(x, y);
    square.setPiece(std::move(piece)); // Square now owns the piece
    
    return rawPiecePtr;
}

void GameInitializer::resetGameState(GameState& gameState) {
    // Set Player 1 as the starting player
    if (gameState.getActivePlayer() != PlayerSide::PLAYER_ONE) {
        gameState.switchActivePlayer();
    }
    
    // Set game phase to draw phase
    gameState.setGamePhase(GamePhase::DRAW);
    
    // Set game result to in progress
    gameState.setGameResult(GameResult::IN_PROGRESS);
    
    // Reset turn number to 1
    gameState.setTurnNumber(1);
    
    // Reset steam for both players
    gameState.setSteam(PlayerSide::PLAYER_ONE, 0);
    gameState.setSteam(PlayerSide::PLAYER_TWO, 0);
    
    // Auto-advance from draw phase to action phase
    gameState.nextPhase();
}

void GameInitializer::calculateInitialControl(GameState& gameState) {
    GameBoard& board = gameState.getBoard();
    
    // Use the new InfluenceSystem to calculate board influence and control
    InfluenceSystem::calculateBoardInfluence(board);
}

} // namespace BayouBonanza
