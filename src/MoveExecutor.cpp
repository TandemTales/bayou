#include "MoveExecutor.h"
#include "GameBoard.h"
#include "Square.h"
#include "King.h"

namespace BayouBonanza {

bool MoveExecutor::validateMove(const GameState& gameState, const Move& move) const {
    const GameBoard& board = gameState.getBoard();
    std::shared_ptr<Piece> piece = move.getPiece();
    
    // Ensure the piece exists
    if (!piece) {
        return false;
    }
    
    // Check if it's the correct player's turn
    if (piece->getSide() != gameState.getActivePlayer()) {
        return false;
    }
    
    // Check if the piece is at the expected starting position
    if (piece->getPosition() != move.getFrom()) {
        return false;
    }
    
    // Check if the move is valid for this piece type
    if (!piece->isValidMove(board, move.getTo())) {
        return false;
    }
    
    return true;
}

MoveResult MoveExecutor::executeMove(GameState& gameState, const Move& move) {
    // Validate the move first
    if (!validateMove(gameState, move)) {
        return MoveResult::INVALID_MOVE;
    }
    
    GameBoard& board = gameState.getBoard();
    std::shared_ptr<Piece> piece = move.getPiece();
    const Position& from = move.getFrom();
    const Position& to = move.getTo();
    
    // Get the target square
    Square& fromSquare = board.getSquare(from.x, from.y);
    Square& toSquare = board.getSquare(to.x, to.y);
    
    // If the target square has a piece, resolve combat
    if (!toSquare.isEmpty()) {
        std::shared_ptr<Piece> targetPiece = toSquare.getPiece();
        
        // Check if the target piece belongs to the opponent
        if (targetPiece->getSide() != piece->getSide()) {
            bool destroyed = resolveCombat(piece, targetPiece, gameState);
            
            // If the opponent's piece is not destroyed, the attacker stays in place
            if (!destroyed) {
                return MoveResult::SUCCESS;
            }
            
            // Check if the destroyed piece was a king
            if (dynamic_cast<King*>(targetPiece.get())) {
                // Set game result based on which king was captured
                if (targetPiece->getSide() == PlayerSide::PLAYER_ONE) {
                    gameState.setGameResult(GameResult::PLAYER_TWO_WIN);
                } else {
                    gameState.setGameResult(GameResult::PLAYER_ONE_WIN);
                }
                
                // Move the attacking piece to the target square
                toSquare.setPiece(piece);
                fromSquare.setPiece(nullptr);
                piece->setPosition(to);
                
                return MoveResult::KING_CAPTURED;
            }
        } else {
            // Cannot move to a square occupied by own piece
            return MoveResult::INVALID_MOVE;
        }
    }
    
    // Move the piece to the target square
    toSquare.setPiece(piece);
    fromSquare.setPiece(nullptr);
    piece->setPosition(to);
    
    // Recalculate board control after the move
    recalculateBoardControl(gameState);
    
    return MoveResult::SUCCESS;
}

std::vector<Move> MoveExecutor::getValidMoves(const GameState& gameState, std::shared_ptr<Piece> piece) const {
    std::vector<Move> validMoves;
    
    if (!piece) {
        return validMoves;
    }
    
    // Get the current position of the piece
    Position from = piece->getPosition();
    
    // Get all valid target positions for this piece
    std::vector<Position> targets = piece->getValidMoves(gameState.getBoard());
    
    // Create Move objects for each valid target
    for (const Position& to : targets) {
        validMoves.emplace_back(piece, from, to);
    }
    
    return validMoves;
}

bool MoveExecutor::resolveCombat(std::shared_ptr<Piece> attacker, std::shared_ptr<Piece> defender, GameState& gameState) {
    // Apply attacker's damage to defender
    bool destroyed = defender->takeDamage(attacker->getAttack());
    
    // Return whether the defender was destroyed
    return destroyed;
}

void MoveExecutor::recalculateBoardControl(GameState& gameState) {
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
    
    // After recalculating control, update steam for each player
    int player1ControlledSquares = 0;
    int player2ControlledSquares = 0;
    
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Square& square = board.getSquare(x, y);
            PlayerSide controller = square.getControlledBy();
            
            if (controller == PlayerSide::PLAYER_ONE) {
                player1ControlledSquares++;
            } else if (controller == PlayerSide::PLAYER_TWO) {
                player2ControlledSquares++;
            }
        }
    }
    
    // Add steam based on controlled squares
    gameState.addSteam(PlayerSide::PLAYER_ONE, player1ControlledSquares);
    gameState.addSteam(PlayerSide::PLAYER_TWO, player2ControlledSquares);
}

} // namespace BayouBonanza
