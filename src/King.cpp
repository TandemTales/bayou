#include "King.h"
#include "GameBoard.h"

namespace BayouBonanza {

// King has high health but moderate attack
King::King(PlayerSide side) : 
    Piece(side, 3, 10) {
}

bool King::isValidMove(const GameBoard& board, const Position& target) const {
    // Check if the target position is valid
    if (!board.isValidPosition(target.x, target.y)) {
        return false;
    }
    
    // Calculate the distance from current position to target
    int dx = std::abs(target.x - position.x);
    int dy = std::abs(target.y - position.y);
    
    // King can move one square in any direction (including diagonals)
    if (dx <= 1 && dy <= 1 && (dx > 0 || dy > 0)) {
        // Check if the target square is empty or has an enemy piece
        const Square& targetSquare = board.getSquare(target.x, target.y);
        return targetSquare.isEmpty() || 
               (targetSquare.getPiece() && targetSquare.getPiece()->getSide() != side);
    }
    
    return false;
}

std::vector<Position> King::getValidMoves(const GameBoard& board) const {
    std::vector<Position> validMoves;
    
    // Check all 8 directions around the king
    const int directions[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1, 0},           {1, 0},
        {-1, 1},  {0, 1},  {1, 1}
    };
    
    for (const auto& dir : directions) {
        Position target(position.x + dir[0], position.y + dir[1]);
        
        if (isValidMove(board, target)) {
            validMoves.push_back(target);
        }
    }
    
    return validMoves;
}

std::string King::getTypeName() const {
    return "King";
}

} // namespace BayouBonanza
