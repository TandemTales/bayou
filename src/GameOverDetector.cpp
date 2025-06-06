#include "GameOverDetector.h"
#include "GameBoard.h"
// #include "King.h" // Removed - using data-driven approach with PieceFactory
#include "../include/HealthTracker.h"

namespace BayouBonanza {

bool GameOverDetector::isGameOver(const GameState& gameState) const {
    // If game result is already set, the game is over
    if (gameState.getGameResult() != GameResult::IN_PROGRESS) {
        return true;
    }
    
    // Check if either player has lost their king
    if (!hasKing(gameState, PlayerSide::PLAYER_ONE) || 
        !hasKing(gameState, PlayerSide::PLAYER_TWO)) {
        return true;
    }
    
    // No game-ending conditions found
    return false;
}

PlayerSide GameOverDetector::getWinner(const GameState& gameState) const {
    // Check if the game result is already set
    if (gameState.getGameResult() == GameResult::PLAYER_ONE_WIN) {
        return PlayerSide::PLAYER_ONE;
    } else if (gameState.getGameResult() == GameResult::PLAYER_TWO_WIN) {
        return PlayerSide::PLAYER_TWO;
    } else if (gameState.getGameResult() == GameResult::DRAW) {
        return PlayerSide::NEUTRAL;
    }
    
    // Check if either player has lost their king
    if (!hasKing(gameState, PlayerSide::PLAYER_ONE)) {
        return PlayerSide::PLAYER_TWO;
    } else if (!hasKing(gameState, PlayerSide::PLAYER_TWO)) {
        return PlayerSide::PLAYER_ONE;
    }
    
    // No winner yet
    return PlayerSide::NEUTRAL;
}

bool GameOverDetector::checkAndUpdateGameOver(GameState& gameState) {
    // If the game is already over, don't update anything
    if (gameState.getGameResult() != GameResult::IN_PROGRESS) {
        return true;
    }
    
    // Check for game-ending conditions
    if (!hasKing(gameState, PlayerSide::PLAYER_ONE)) {
        gameState.setGameResult(GameResult::PLAYER_TWO_WIN);
        gameState.setGamePhase(GamePhase::GAME_OVER);
        return true;
    } else if (!hasKing(gameState, PlayerSide::PLAYER_TWO)) {
        gameState.setGameResult(GameResult::PLAYER_ONE_WIN);
        gameState.setGamePhase(GamePhase::GAME_OVER);
        return true;
    }
    
    // No game-ending conditions found
    return false;
}

bool GameOverDetector::hasKing(const GameState& gameState, PlayerSide side) const {
    const GameBoard& board = gameState.getBoard();
    
    // Iterate through each square on the board
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            const Square& square = board.getSquare(x, y);
            
            // Check if the square has a piece
            if (!square.isEmpty()) {
                Piece* piece = square.getPiece();
                
                // Check if the piece is a king of the specified side
                if (piece->getSide() == side && piece->isVictoryPiece()) {
                    return true;
                }
            }
        }
    }
    
    // No king found for this player
    return false;
}

} // namespace BayouBonanza
