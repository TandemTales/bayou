#include "Queen.h"
#include "GameBoard.h"

namespace BayouBonanza {

// Queen has moderate health but high attack
Queen::Queen(PlayerSide side) : 
    Piece(side, 5, 8) {
}

bool Queen::isValidMove(const GameBoard& board, const Position& target) const {
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
    
    // Queen can move horizontally, vertically, or diagonally
    // Check if the move is along a valid direction
    if (dx == 0 || dy == 0 || std::abs(dx) == std::abs(dy)) {
        // Determine the direction of movement
        int stepX = (dx == 0) ? 0 : (dx > 0 ? 1 : -1);
        int stepY = (dy == 0) ? 0 : (dy > 0 ? 1 : -1);
        
        // Check all squares along the path for obstacles
        int x = position.x + stepX;
        int y = position.y + stepY;
        
        while (x != target.x || y != target.y) {
            if (!board.getSquare(x, y).isEmpty()) {
                return false;  // Path is blocked
            }
            x += stepX;
            y += stepY;
        }
        
        return true;  // Path is clear
    }
    
    return false;  // Not a valid direction for Queen
}

std::vector<Position> Queen::getValidMoves(const GameBoard& board) const {
    std::vector<Position> validMoves;
    
    // Check all 8 directions (horizontal, vertical, diagonal)
    const int directions[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},  // Diagonally up-left, Up, Diagonally up-right
        {-1, 0},           {1, 0},   // Left, Right
        {-1, 1},  {0, 1},  {1, 1}    // Diagonally down-left, Down, Diagonally down-right
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

std::vector<Position> Queen::getInfluenceArea(const GameBoard& board) const {
    // For the Queen, influence area is the same as valid moves
    return getValidMoves(board);
}

std::string Queen::getTypeName() const {
    return "Queen";
}

std::string Queen::getSymbol() const {
    return "Q";
}

PieceType Queen::getPieceType() const {
    return PieceType::QUEEN;
}

} // namespace BayouBonanza
