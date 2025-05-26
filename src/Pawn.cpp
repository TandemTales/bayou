#include "Pawn.h"
#include "GameBoard.h"

namespace BayouBonanza {

// Pawn has low health but can be effective in numbers
Pawn::Pawn(PlayerSide side) : 
    Piece(side, 2, 5) {
}

bool Pawn::isValidMove(const GameBoard& board, const Position& target) const {
    // Check if the target position is valid
    if (!board.isValidPosition(target.x, target.y)) {
        return false;
    }
    
    // Direction depends on the player side (North/South orientation)
    int forwardDirection = (side == PlayerSide::PLAYER_ONE) ? -1 : 1;
    
    // Calculate the distance from current position to target
    int dx = target.x - position.x;
    int dy = target.y - position.y;
    
    // Check for forward move (non-capturing)
    if (dx == 0 && dy == forwardDirection) {
        // Forward moves must be to empty squares
        return board.getSquare(target.x, target.y).isEmpty();
    }
    
    // Check for diagonal capture
    if (std::abs(dx) == 1 && dy == forwardDirection) {
        const Square& targetSquare = board.getSquare(target.x, target.y);
        // Diagonal moves must capture an enemy piece
        return !targetSquare.isEmpty() && targetSquare.getPiece()->getSide() != side;
    }
    
    return false;  // Not a valid move for Pawn
}

std::vector<Position> Pawn::getValidMoves(const GameBoard& board) const {
    std::vector<Position> validMoves;
    
    // Direction depends on the player side (North/South orientation)
    int forwardDirection = (side == PlayerSide::PLAYER_ONE) ? -1 : 1;
    
    // Check forward move
    Position forwardPos(position.x, position.y + forwardDirection);
    if (board.isValidPosition(forwardPos.x, forwardPos.y) && 
        board.getSquare(forwardPos.x, forwardPos.y).isEmpty()) {
        validMoves.push_back(forwardPos);
    }
    
    // Check diagonal captures
    for (int dx : {-1, 1}) {
        Position capturePos(position.x + dx, position.y + forwardDirection);
        if (board.isValidPosition(capturePos.x, capturePos.y)) {
            const Square& square = board.getSquare(capturePos.x, capturePos.y);
            if (!square.isEmpty() && square.getPiece()->getSide() != side) {
                validMoves.push_back(capturePos);
            }
        }
    }
    
    return validMoves;
}

std::vector<Position> Pawn::getInfluenceArea(const GameBoard& board) const {
    std::vector<Position> influenceArea;
    
    // Pawns only influence the diagonal squares they can capture on
    int forwardDirection = (side == PlayerSide::PLAYER_ONE) ? -1 : 1;
    
    // Check diagonal influence
    for (int dx : {-1, 1}) {
        Position influencePos(position.x + dx, position.y + forwardDirection);
        if (board.isValidPosition(influencePos.x, influencePos.y)) {
            influenceArea.push_back(influencePos);
        }
    }
    
    return influenceArea;
}

std::string Pawn::getTypeName() const {
    return "Pawn";
}

std::string Pawn::getSymbol() const {
    return "P";
}

PieceType Pawn::getPieceType() const {
    return PieceType::PAWN;
}

} // namespace BayouBonanza
