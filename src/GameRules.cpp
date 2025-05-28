#include "GameRules.h"
#include "King.h"
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"
#include "Knight.h"
#include "Pawn.h"
#include "Square.h"

namespace BayouBonanza {

GameRules::GameRules() = default;

void GameRules::initializeGame(GameState& gameState) {
    // Reset the game state
    gameState.initializeNewGame();
    
    // Place pieces for both players in a chess-like starting formation
    
    // Player 1 (bottom of board) - rows 6 and 7
    // Place pawns on row 6
    for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
        placePiece<Pawn>(gameState, PlayerSide::PLAYER_ONE, x, 6);
    }
    
    // Place major pieces on row 7
    placePiece<Rook>(gameState, PlayerSide::PLAYER_ONE, 0, 7);
    placePiece<Knight>(gameState, PlayerSide::PLAYER_ONE, 1, 7);
    placePiece<Bishop>(gameState, PlayerSide::PLAYER_ONE, 2, 7);
    placePiece<Queen>(gameState, PlayerSide::PLAYER_ONE, 3, 7);
    placePiece<King>(gameState, PlayerSide::PLAYER_ONE, 4, 7);
    placePiece<Bishop>(gameState, PlayerSide::PLAYER_ONE, 5, 7);
    placePiece<Knight>(gameState, PlayerSide::PLAYER_ONE, 6, 7);
    placePiece<Rook>(gameState, PlayerSide::PLAYER_ONE, 7, 7);
    
    // Player 2 (top of board) - rows 0 and 1
    // Place pawns on row 1
    for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
        placePiece<Pawn>(gameState, PlayerSide::PLAYER_TWO, x, 1);
    }
    
    // Place major pieces on row 0
    placePiece<Rook>(gameState, PlayerSide::PLAYER_TWO, 0, 0);
    placePiece<Knight>(gameState, PlayerSide::PLAYER_TWO, 1, 0);
    placePiece<Bishop>(gameState, PlayerSide::PLAYER_TWO, 2, 0);
    placePiece<Queen>(gameState, PlayerSide::PLAYER_TWO, 3, 0);
    placePiece<King>(gameState, PlayerSide::PLAYER_TWO, 4, 0);
    placePiece<Bishop>(gameState, PlayerSide::PLAYER_TWO, 5, 0);
    placePiece<Knight>(gameState, PlayerSide::PLAYER_TWO, 6, 0);
    placePiece<Rook>(gameState, PlayerSide::PLAYER_TWO, 7, 0);
    
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
                // Get valid moves for this piece
                std::vector<Move> pieceMoves = moveExecutor.getValidMoves(
                    gameState, square.getPiece());
                
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

template<typename PieceType>
void GameRules::placePiece(GameState& gameState, PlayerSide side, int x, int y) {
    GameBoard& board = gameState.getBoard();
    
    // Create a new piece of the specified type
    std::shared_ptr<PieceType> piece = std::make_shared<PieceType>(side);
    
    // Set the piece's position
    Position pos(x, y);
    piece->setPosition(pos);
    
    // Place the piece on the board
    Square& square = board.getSquare(x, y);
    square.setPiece(piece);
}

void GameRules::placeKing(GameState& gameState, PlayerSide side, int x, int y) {
    placePiece<King>(gameState, side, x, y);
}

bool GameRules::hasKing(const GameState& gameState, PlayerSide side) const {
    const GameBoard& board = gameState.getBoard();
    
    // Iterate through each square on the board
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            const Square& square = board.getSquare(x, y);
            
            // Check if the square has a piece
            if (!square.isEmpty()) {
                std::shared_ptr<Piece> piece = square.getPiece();
                
                // Check if the piece is a king of the specified side
                if (piece->getSide() == side && dynamic_cast<King*>(piece.get())) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

} // namespace BayouBonanza
