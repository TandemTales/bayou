#include "GameInitializer.h"
#include "GameBoard.h"
#include "Square.h"
#include "King.h"

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
    
    // Place kings for both players
    // Player 1's king at the bottom center of the board
    createAndPlaceKing(gameState, PlayerSide::PLAYER_ONE, 4, 7);
    
    // Player 2's king at the top center of the board
    createAndPlaceKing(gameState, PlayerSide::PLAYER_TWO, 4, 0);
    
    // TODO: In future tasks, add more initial pieces as needed
}

std::shared_ptr<Piece> GameInitializer::createAndPlaceKing(GameState& gameState, PlayerSide side, int x, int y) {
    // Create a new king
    std::shared_ptr<King> king = std::make_shared<King>(side);
    
    // Set the king's position
    Position pos(x, y);
    king->setPosition(pos);
    
    // Place the king on the board
    Square& square = gameState.getBoard().getSquare(x, y);
    square.setPiece(king);
    
    return king;
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
