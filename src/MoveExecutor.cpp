#include "MoveExecutor.h"
#include "GameBoard.h"
#include "InfluenceSystem.h"
// #include "King.h" // Removed - using data-driven approach with PieceFactory
#include "GameState.h"
#include "Square.h"

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
        Piece* targetPiece = toSquare.getPiece();
        
        // Check if the target piece belongs to the opponent
        if (targetPiece->getSide() != piece->getSide()) {
            // Create a shared_ptr wrapper for targetPiece temporarily for combat resolution
            // Note: This is a design issue - we should either use all shared_ptr or all raw pointers
            std::shared_ptr<Piece> targetSharedPtr(targetPiece, [](Piece*){});  // No-op deleter since ownership is managed by Square
            
            bool destroyed = resolveCombat(piece, targetSharedPtr, gameState);
            
            // If the opponent's piece is not destroyed, the attacker stays in place
            if (!destroyed) {
                return MoveResult::SUCCESS;
            }
            
            // Check if the destroyed piece was a king
            if (targetPiece->getPieceType() == PieceType::KING) {
                // Set game result based on which king was captured
                if (targetPiece->getSide() == PlayerSide::PLAYER_ONE) {
                    gameState.setGameResult(GameResult::PLAYER_TWO_WIN);
                } else {
                    gameState.setGameResult(GameResult::PLAYER_ONE_WIN);
                }
                
                // Extract the piece from the source square properly
                std::unique_ptr<Piece> movingPiece = fromSquare.extractPiece();
                if (!movingPiece) {
                    return MoveResult::INVALID_MOVE; // Piece was not at expected location
                }
                
                // Move the attacking piece to the target square
                movingPiece->setPosition(to);
                toSquare.setPiece(std::move(movingPiece));
                
                return MoveResult::KING_CAPTURED;
            }
        } else {
            // Cannot move to a square occupied by own piece
            return MoveResult::INVALID_MOVE;
        }
    }
    
    // Extract the piece from the source square properly
    std::unique_ptr<Piece> movingPiece = fromSquare.extractPiece();
    if (!movingPiece) {
        return MoveResult::INVALID_MOVE; // Piece was not at expected location
    }
    
    // Move the piece to the target square
    movingPiece->setPosition(to);
    toSquare.setPiece(std::move(movingPiece));
    
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
    
    // Use the new InfluenceSystem to calculate board influence and control
    InfluenceSystem::calculateBoardInfluence(board);
    
    // After recalculating control, update steam for each player
    int player1ControlledSquares = 0;
    int player2ControlledSquares = 0;
    
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Square& square = board.getSquare(x, y);
            PlayerSide controller = InfluenceSystem::getControllingPlayer(square);
            
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
