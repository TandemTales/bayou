#include "Bishop.h"
#include "GameBoard.h"

namespace BayouBonanza {

// Bishop has moderate health and attack
Bishop::Bishop(PlayerSide side) : 
    Piece(side, 4, 8) {
}

bool Bishop::isValidMove(const GameBoard& board, const Position& target) const {
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
    int dx = target.x - position.x;
    int dy = target.y - position.y;
    
    // Bishop can only move diagonally
    if (std::abs(dx) == std::abs(dy) && dx != 0) {
        // Determine the direction of movement
        int stepX = (dx > 0) ? 1 : -1;
        int stepY = (dy > 0) ? 1 : -1;
        
        // Check all squares along the path for obstacles
        int x = position.x + stepX;
        int y = position.y + stepY;
        
        while (x != target.x && y != target.y) {
            if (!board.getSquare(x, y).isEmpty()) {
                return false;  // Path is blocked
            }
            x += stepX;
            y += stepY;
        }
        
        return true;  // Path is clear
    }
    
    return false;  // Not a valid direction for Bishop
}

std::vector<Position> Bishop::getValidMoves(const GameBoard& board) const {
    std::vector<Position> validMoves;
    
    // Check all 4 diagonal directions
    const int directions[4][2] = {
        {-1, -1}, {1, -1},  // Diagonally up-left, Diagonally up-right
        {-1, 1},  {1, 1}    // Diagonally down-left, Diagonally down-right
    };
    
    for (const auto& dir : directions) {
        int stepX = dir[0];
        int stepY = dir[1];
        
        // Check all squares in this direction until we hit the edge or a piece
        for (int i = 1; i < GameBoard::BOARD_SIZE; ++i) {  // Maximum possible steps is the board width
            Position target(position.x + (i * stepX), position.y + (i * stepY));
            
            // Check if we're still on the board
            if (!board.isValidPosition(target.x, target.y)) {
                break;  // Off the board, stop checking this direction
            }
            
            const Square& targetSquare = board.getSquare(target.x, target.y);
            if (targetSquare.isEmpty()) {
                // Empty square, we can move here
                validMoves.push_back(target);
            } else {
                // Square has a piece
                if (targetSquare.getPiece()->getSide() != side) {
                    // Enemy piece, we can move here (to capture)
                    validMoves.push_back(target);
                }
                break;  // Can't move past pieces, stop checking this direction
            }
        }
    }
    
    return validMoves;
}

std::vector<Position> Bishop::getInfluenceArea(const GameBoard& board) const {
    // For the Bishop, influence area is the same as valid moves
    return getValidMoves(board);
}

std::string Bishop::getTypeName() const {
    return "Bishop";
}

std::string Bishop::getSymbol() const {
    return "B";
}

PieceType Bishop::getPieceType() const {
    return PieceType::BISHOP;
}

} // namespace BayouBonanza
