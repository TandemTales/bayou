#include "Knight.h"
#include "GameBoard.h"

namespace BayouBonanza {

// Knight has moderate health and attack with unique movement
Knight::Knight(PlayerSide side) : 
    Piece(side, 3, 7) {
}

bool Knight::isValidMove(const GameBoard& board, const Position& target) const {
    // Check if the target position is valid
    if (!board.isValidPosition(target.x, target.y)) {
        return false;
    }
    
    // Check if the target has our own piece (can't capture our own pieces)
    const Square& targetSquare = board.getSquare(target.x, target.y);
    if (!targetSquare.isEmpty() && targetSquare.getPiece()->getSide() == side) {
        return false;
    }
    
    // Calculate the distance from current position to target
    int dx = std::abs(target.x - position.x);
    int dy = std::abs(target.y - position.y);
    
    // Knight moves in an L-shape: 2 squares in one direction and 1 square in the perpendicular direction
    return (dx == 1 && dy == 2) || (dx == 2 && dy == 1);
}

std::vector<Position> Knight::getValidMoves(const GameBoard& board) const {
    std::vector<Position> validMoves;
    
    // All possible L-shaped Knight moves
    const int moves[8][2] = {
        {-2, -1}, {-1, -2}, {1, -2}, {2, -1},  // Two left or top, one up or right
        {2, 1},   {1, 2},   {-1, 2}, {-2, 1}   // Two right or bottom, one down or left
    };
    
    for (const auto& move : moves) {
        Position target(position.x + move[0], position.y + move[1]);
        
        if (isValidMove(board, target)) {
            validMoves.push_back(target);
        }
    }
    
    return validMoves;
}

std::vector<Position> Knight::getInfluenceArea(const GameBoard& board) const {
    // For the Knight, influence area is the same as valid moves
    return getValidMoves(board);
}

std::string Knight::getTypeName() const {
    return "Knight";
}

std::string Knight::getSymbol() const {
    return "N";
}

PieceType Knight::getPieceType() const {
    return PieceType::KNIGHT;
}

} // namespace BayouBonanza
