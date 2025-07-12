#include "GameInitializer.h"
#include "GameBoard.h"
#include "Square.h"
#include "InfluenceSystem.h"
#include <iostream> // For std::cerr

namespace BayouBonanza {

// Constructor
GameInitializer::GameInitializer() {
    ownedPieceDefManager = std::make_unique<PieceDefinitionManager>();
    if (!ownedPieceDefManager->loadDefinitions("assets/data/pieces.json")) {
        // Handle error: Log and maybe throw or exit
        std::cerr << "FATAL: Could not load piece definitions from assets/data/pieces.json" << std::endl;
        // Consider throwing an exception or setting an error state that can be checked.
        // For now, proceeding will likely lead to issues if pieceFactory is used without definitions.
    }
    ownedPieceFactory = std::make_unique<BayouBonanza::PieceFactory>(*ownedPieceDefManager);
    
    // Set references to the owned instances
    pieceDefManager = ownedPieceDefManager.get();
    pieceFactory = ownedPieceFactory.get();
}

// Constructor with external references
GameInitializer::GameInitializer(const PieceDefinitionManager& pieceDefManager, PieceFactory& pieceFactory)
    : ownedPieceDefManager(nullptr), ownedPieceFactory(nullptr),
      pieceDefManager(&pieceDefManager), pieceFactory(&pieceFactory) {
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
    // Place single TinkeringTom piece in the center of the back row
    createAndPlacePiece(gameState, "TinkeringTom", playerOne, 4, 7);
    
    // Set up Player Two pieces (top of board)
    PlayerSide playerTwo = PlayerSide::PLAYER_TWO;
    // Place single TinkeringTom piece in the center of the back row
    createAndPlacePiece(gameState, "TinkeringTom", playerTwo, 4, 0);
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
    
    // Start directly in PLAY phase so the first player can make moves
    // The DRAW phase auto-advancement was causing issues with turn switching
    gameState.setGamePhase(GamePhase::PLAY);
    
    // Set game result to in progress
    gameState.setGameResult(GameResult::IN_PROGRESS);
    
    // Reset turn number to 1
    gameState.setTurnNumber(1);
    
    // Reset steam for both players
    gameState.setSteam(PlayerSide::PLAYER_ONE, 0);
    gameState.setSteam(PlayerSide::PLAYER_TWO, 0);
}

void GameInitializer::calculateInitialControl(GameState& gameState) {
    GameBoard& board = gameState.getBoard();
    
    // Use the new InfluenceSystem to calculate board influence and control
    InfluenceSystem::calculateBoardInfluence(board);
}

} // namespace BayouBonanza
