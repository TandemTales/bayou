#include "GameOverDetector.h"
#include "GameBoard.h"
// #include "King.h" // Removed - using data-driven approach with PieceFactory
#include "../include/HealthTracker.h"

namespace BayouBonanza {

// Initialize static member
WinConditionCallback GameOverDetector::winConditionCallback = nullptr;

void GameOverDetector::registerWinConditionCallback(WinConditionCallback callback) {
    winConditionCallback = callback;
}

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
        
        // Fire win condition notification
        std::string description = "Player 2 wins! Player 1's king has been captured.";
        fireWinConditionNotification(PlayerSide::PLAYER_TWO, description);
        
        return true;
    } else if (!hasKing(gameState, PlayerSide::PLAYER_TWO)) {
        gameState.setGameResult(GameResult::PLAYER_ONE_WIN);
        gameState.setGamePhase(GamePhase::GAME_OVER);
        
        // Fire win condition notification
        std::string description = "Player 1 wins! Player 2's king has been captured.";
        fireWinConditionNotification(PlayerSide::PLAYER_ONE, description);
        
        return true;
    }
    
    // No game-ending conditions found
    return false;
}

std::string GameOverDetector::getWinConditionDescription(const GameState& gameState) const {
    // Check current game result
    GameResult result = gameState.getGameResult();
    
    switch (result) {
        case GameResult::PLAYER_ONE_WIN:
            if (!hasKing(gameState, PlayerSide::PLAYER_TWO)) {
                return "Player 1 wins! Player 2's king has been captured.";
            } else {
                return "Player 1 wins!";
            }
            
        case GameResult::PLAYER_TWO_WIN:
            if (!hasKing(gameState, PlayerSide::PLAYER_ONE)) {
                return "Player 2 wins! Player 1's king has been captured.";
            } else {
                return "Player 2 wins!";
            }
            
        case GameResult::DRAW:
            return "Game ended in a draw.";
            
        case GameResult::IN_PROGRESS:
            // Check for potential win conditions
            bool player1HasKing = hasKing(gameState, PlayerSide::PLAYER_ONE);
            bool player2HasKing = hasKing(gameState, PlayerSide::PLAYER_TWO);
            
            if (!player1HasKing && !player2HasKing) {
                return "Both kings are missing! This should not happen in normal gameplay.";
            } else if (!player1HasKing) {
                return "Player 1's king is missing! Player 2 should win.";
            } else if (!player2HasKing) {
                return "Player 2's king is missing! Player 1 should win.";
            } else {
                // Game is still in progress
                PlayerSide activePlayer = gameState.getActivePlayer();
                std::string playerStr = (activePlayer == PlayerSide::PLAYER_ONE) ? "Player 1" : "Player 2";
                
                switch (gameState.getGamePhase()) {
                    case GamePhase::DRAW:
                        return "Game in progress. " + playerStr + "'s turn (Draw Phase).";
                    case GamePhase::PLAY:
                        return "Game in progress. " + playerStr + "'s turn (Play Phase).";
                    case GamePhase::MOVE:
                        return "Game in progress. " + playerStr + "'s turn (Move Phase).";
                    case GamePhase::SETUP:
                        return "Game in setup phase.";
                    case GamePhase::GAME_OVER:
                        return "Game is over.";
                    default:
                        return "Game in progress. " + playerStr + "'s turn.";
                }
            }
    }
    
    return "Unknown game state.";
}

bool GameOverDetector::hasVictoryPieces(const GameState& gameState, PlayerSide side) const {
    return hasKing(gameState, side);
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

void GameOverDetector::fireWinConditionNotification(PlayerSide winner, const std::string& description) {
    if (winConditionCallback) {
        winConditionCallback(winner, description);
    }
}

} // namespace BayouBonanza
