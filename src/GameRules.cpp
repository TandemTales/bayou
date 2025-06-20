// #include "King.h" // Removed - using data-driven approach
#include "GameRules.h"
#include "GameBoard.h"
#include "Square.h"
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"

namespace BayouBonanza {

GameRules::GameRules() = default;

void GameRules::initializeGame(GameState& gameState) {
    // Reset the game state
    gameState.initializeNewGame();
    
    // For now, we'll create a simplified initialization
    // In a full implementation, we would use PieceFactory to create pieces
    // based on a configuration file
    
    // TODO: This should be handled by GameInitializer with PieceFactory
    // For now, just set up an empty board and basic game state
    
    // Set starting player
    gameState.setActivePlayer(PlayerSide::PLAYER_ONE);
    gameState.setGamePhase(GamePhase::DRAW); // Start with draw phase
    gameState.setGameResult(GameResult::IN_PROGRESS);
    
    // Auto-advance from draw phase to action phase
    gameState.nextPhase();
    
    // Recalculate board control after placing pieces
    moveExecutor.recalculateBoardControl(gameState);
}

MoveResult GameRules::processMove(GameState& gameState, const Move& move) {
    // Execute the move and get the result
    MoveResult result = moveExecutor.executeMove(gameState, move);
    
    // If the move was successful, check for game ending conditions
    if (result == MoveResult::SUCCESS || result == MoveResult::PIECE_DESTROYED) {
        // Check if any player has lost their king
        if (!hasKing(gameState, PlayerSide::PLAYER_ONE)) {
            gameState.setGameResult(GameResult::PLAYER_TWO_WIN);
            return MoveResult::KING_CAPTURED;
        }
        
        if (!hasKing(gameState, PlayerSide::PLAYER_TWO)) {
            gameState.setGameResult(GameResult::PLAYER_ONE_WIN);
            return MoveResult::KING_CAPTURED;
        }
    }
    
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
