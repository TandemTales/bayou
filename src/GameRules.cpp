// #include "King.h" // Removed - using data-driven approach
#include "GameRules.h"
#include "GameBoard.h"
#include "Square.h"
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"
#include "GameInitializer.h"

namespace BayouBonanza {

GameRules::GameRules() = default;

void GameRules::initializeGame(GameState& gameState) {
    // Use GameInitializer to properly set up the game with pieces
    GameInitializer initializer;
    initializer.initializeNewGame(gameState);
    
    // Recalculate board control after placing pieces
    moveExecutor.recalculateBoardControl(gameState);
}

MoveResult GameRules::processMove(GameState& gameState, const Move& move) {
    // Execute the move and get the result
    MoveResult result = moveExecutor.executeMove(gameState, move);
    
    // MoveExecutor already handles victory piece destruction and sets game result
    // No need for additional checks here
    return result;
}

bool GameRules::isGameOver(const GameState& gameState) const {
    return gameState.getGameResult() != GameResult::IN_PROGRESS;
}

void GameRules::endTurn(GameState& gameState) {
    // Switch to the other player
    gameState.switchActivePlayer();
    
    // Increment the turn number after each move
    gameState.incrementTurnNumber();
    
    // Process turn start for the new active player (calculate and add steam generation)
    gameState.processTurnStart();
    
    // Recalculate board control at the end of each turn
    moveExecutor.recalculateBoardControl(gameState);
}

std::vector<Move> GameRules::getValidMovesForActivePlayer(const GameState& gameState) const {
    std::vector<Move> validMoves;
    PlayerSide activeSide = gameState.getActivePlayer();
    const GameBoard& board = gameState.getBoard();
    
    // Iterate through each square on the board
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            const Square& square = board.getSquare(x, y);
            
            // If square has a piece belonging to the active player
            if (!square.isEmpty() && square.getPiece()->getSide() == activeSide) {
                if (square.getPiece()->isStunned()) {
                    continue;
                }
                // Create a temporary shared_ptr wrapper for the piece
                Piece* rawPiece = square.getPiece();
                std::shared_ptr<Piece> piecePtr(rawPiece, [](Piece*){});  // No-op deleter

                // Get valid moves for this piece
                std::vector<Move> pieceMoves = moveExecutor.getValidMoves(gameState, piecePtr);

                // Add them to the list of all valid moves
                validMoves.insert(validMoves.end(), pieceMoves.begin(), pieceMoves.end());
            }
        }
    }
    
    return validMoves;
}

bool GameRules::hasPlayerWon(const GameState& gameState, PlayerSide side) const {
    // A player wins if the opponent's king is captured
    PlayerSide opponent = (side == PlayerSide::PLAYER_ONE) ? 
                         PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
    
    return !hasKing(gameState, opponent);
}

bool GameRules::hasKing(const GameState& gameState, PlayerSide side) const {
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
    
    return false;
}

} // namespace BayouBonanza
