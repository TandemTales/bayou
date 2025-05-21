#include "GameInitializer.h"
#include "GameBoard.h"
#include "Square.h"

namespace BayouBonanza {

void GameInitializer::initializeNewGame(GameState& gameState) {
    // Reset the game state to default values
    resetGameState(gameState);
    
    // Set up the board with initial pieces
    setupBoard(gameState);
    
    // Calculate initial control values
    calculateInitialControl(gameState);
}

void GameInitializer::setupBoard(GameState& gameState) {
    // Clear the board
    gameState.getBoard().resetBoard();
    
    // Set up Player One pieces (bottom of board)
    PlayerSide playerOne = PlayerSide::PLAYER_ONE;
    // Back row
    createAndPlacePiece(gameState, "Rook", playerOne, 0, 7);
    createAndPlacePiece(gameState, "Knight", playerOne, 1, 7);
    createAndPlacePiece(gameState, "Bishop", playerOne, 2, 7);
    createAndPlacePiece(gameState, "Queen", playerOne, 3, 7);
    createAndPlacePiece(gameState, "King", playerOne, 4, 7);
    createAndPlacePiece(gameState, "Bishop", playerOne, 5, 7);
    createAndPlacePiece(gameState, "Knight", playerOne, 6, 7);
    createAndPlacePiece(gameState, "Rook", playerOne, 7, 7);
    
    // Pawn row
    for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
        createAndPlacePiece(gameState, "Pawn", playerOne, x, 6);
    }
    
    // Set up Player Two pieces (top of board)
    PlayerSide playerTwo = PlayerSide::PLAYER_TWO;
    // Back row
    createAndPlacePiece(gameState, "Rook", playerTwo, 0, 0);
    createAndPlacePiece(gameState, "Knight", playerTwo, 1, 0);
    createAndPlacePiece(gameState, "Bishop", playerTwo, 2, 0);
    createAndPlacePiece(gameState, "Queen", playerTwo, 3, 0);
    createAndPlacePiece(gameState, "King", playerTwo, 4, 0);
    createAndPlacePiece(gameState, "Bishop", playerTwo, 5, 0);
    createAndPlacePiece(gameState, "Knight", playerTwo, 6, 0);
    createAndPlacePiece(gameState, "Rook", playerTwo, 7, 0);
    
    // Pawn row
    for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
        createAndPlacePiece(gameState, "Pawn", playerTwo, x, 1);
    }
}

std::shared_ptr<Piece> GameInitializer::createAndPlacePiece(GameState& gameState, const std::string& pieceType, PlayerSide side, int x, int y) {
    // Create the piece using the factory
    std::shared_ptr<Piece> piece = PieceFactory::createPieceByType(pieceType, side);
    
    if (!piece) {
        return nullptr; // Invalid piece type
    }
    
    // Set the piece's position
    Position pos(x, y);
    piece->setPosition(pos);
    
    // Place the piece on the board
    Square& square = gameState.getBoard().getSquare(x, y);
    square.setPiece(piece);
    
    return piece;
}

void GameInitializer::resetGameState(GameState& gameState) {
    // Set Player 1 as the starting player
    if (gameState.getActivePlayer() != PlayerSide::PLAYER_ONE) {
        gameState.switchActivePlayer();
    }
    
    // Set game phase to main game
    gameState.setGamePhase(GamePhase::MAIN_GAME);
    
    // Set game result to in progress
    gameState.setGameResult(GameResult::IN_PROGRESS);
    
    // Reset turn number to 1
    while (gameState.getTurnNumber() > 1) {
        gameState.incrementTurnNumber();
    }
    
    // Reset steam for both players
    gameState.setSteam(PlayerSide::PLAYER_ONE, 0);
    gameState.setSteam(PlayerSide::PLAYER_TWO, 0);
}

void GameInitializer::calculateInitialControl(GameState& gameState) {
    GameBoard& board = gameState.getBoard();
    
    // Reset all control values
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Square& square = board.getSquare(x, y);
            square.setControlValue(PlayerSide::PLAYER_ONE, 0);
            square.setControlValue(PlayerSide::PLAYER_TWO, 0);
        }
    }
    
    // Calculate control for each piece on the board
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Square& square = board.getSquare(x, y);
            
            if (!square.isEmpty()) {
                std::shared_ptr<Piece> piece = square.getPiece();
                PlayerSide side = piece->getSide();
                
                // A piece always controls its own square
                square.setControlValue(side, square.getControlValue(side) + 1);
                
                // Get the influence area of this piece
                std::vector<Position> influenceArea = piece->getInfluenceArea(board);
                
                // Apply influence to each square in the area
                for (const Position& pos : influenceArea) {
                    if (pos.x != x || pos.y != y) { // Skip the piece's own square
                        Square& influencedSquare = board.getSquare(pos.x, pos.y);
                        influencedSquare.setControlValue(side, 
                            influencedSquare.getControlValue(side) + 1);
                    }
                }
            }
        }
    }
}

} // namespace BayouBonanza
