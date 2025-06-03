#include "Piece.h"
#include "GameBoard.h"
#include "PieceData.h" // Added
#include "Square.h"    // Added

namespace BayouBonanza {

// Updated constructor
Piece::Piece(PlayerSide side, const PieceStats& stats) :
    side(side),
    stats(stats),
    attack(stats.attack),
    health(stats.health),
    position(-1, -1),
    hasMoved(false) {
}

PlayerSide Piece::getSide() const {
    return side;
}

int Piece::getAttack() const {
    return attack;
}

int Piece::getHealth() const {
    return health;
}

int Piece::getMaxHealth() const {
    return stats.health;
}

void Piece::setHealth(int health) {
    this->health = health;
}

bool Piece::takeDamage(int damage) {
    health -= damage;
    return health <= 0;
}

Position Piece::getPosition() const {
    return position;
}

void Piece::setPosition(const Position& pos) {
    position = pos;
}

// New implementations using PieceStats

std::string Piece::getTypeName() const {
    return stats.typeName;
}

std::string Piece::getSymbol() const {
    return stats.symbol;
}

PieceType Piece::getPieceType() const {
    return stats.pieceType;
}

bool Piece::isValidMove(const GameBoard& board, const Position& target) const {
    for (const auto& rule : stats.movementRules) {
        for (auto baseMove : rule.relativeMoves) { // Make a copy to potentially modify y
            
            // Adjust y for PLAYER_TWO if pawn-like rule
            if ((rule.isPawnForward || rule.isPawnCapture) && side == PlayerSide::PLAYER_TWO) {
                baseMove.y *= -1;
            }

            if (rule.maxRange == 1) { // Non-sliding moves
                Position actualTarget = {position.x + baseMove.x, position.y + baseMove.y};

                if (actualTarget.x == target.x && actualTarget.y == target.y) {
                    if (!board.isValidPosition(target.x, target.y)) {
                        continue; // Should not happen if target is from board iteration, but good check
                    }

                    const Square& targetSquare = board.getSquare(target.x, target.y);

                    if (rule.canJump) {
                        if (targetSquare.isEmpty() || targetSquare.getPiece()->getSide() != side) {
                            return true;
                        }
                    } else {
                        if (rule.isPawnForward) {
                            if (targetSquare.isEmpty()) {
                                return true;
                            }
                        } else if (rule.isPawnCapture) {
                            if (!targetSquare.isEmpty() && targetSquare.getPiece()->getSide() != side) {
                                return true;
                            }
                        } else { // Standard move
                            if (targetSquare.isEmpty() || targetSquare.getPiece()->getSide() != side) {
                                return true;
                            }
                        }
                    }
                }
            } else { // Sliding moves (maxRange > 1)
                for (int d = 1; d <= rule.maxRange; ++d) {
                    Position currentPos = {position.x + baseMove.x * d, position.y + baseMove.y * d};

                    if (!board.isValidPosition(currentPos.x, currentPos.y)) {
                        break; // Path blocked by edge of board
                    }

                    const Square& currentSquare = board.getSquare(currentPos.x, currentPos.y);

                    if (currentPos.x == target.x && currentPos.y == target.y) { // Is this the target square?
                        if (currentSquare.isEmpty() || currentSquare.getPiece()->getSide() != side) {
                            return true; // Valid move if target is empty or enemy
                        } else {
                            break; // Path blocked by friendly piece at the target square
                        }
                    } else { // Intermediate square on the sliding path
                        if (!currentSquare.isEmpty()) {
                            break; // Path blocked by an intermediate piece
                        }
                    }
                }
            }
        }
    }
    return false; // No rule matched
}

std::vector<Position> Piece::getValidMoves(const GameBoard& board) const {
    std::vector<Position> validMoves;
    for (const auto& rule : stats.movementRules) {
        for (auto baseMove : rule.relativeMoves) { // Make a copy to potentially modify y
            
            // Adjust y for PLAYER_TWO if pawn-like rule
            if ((rule.isPawnForward || rule.isPawnCapture) && side == PlayerSide::PLAYER_TWO) {
                baseMove.y *= -1;
            }

            if (rule.maxRange == 1) {
                Position targetPos = {position.x + baseMove.x, position.y + baseMove.y};
                if (isValidMove(board, targetPos)) { // Use the comprehensive isValidMove
                    validMoves.push_back(targetPos);
                }
            } else { // Sliding moves
                for (int d = 1; d <= rule.maxRange; ++d) {
                    Position targetPos = {position.x + baseMove.x * d, position.y + baseMove.y * d};
                    if (!board.isValidPosition(targetPos.x, targetPos.y)) {
                        break; // Off board
                    }
                    // We need to check if the path to targetPos is clear *before* calling isValidMove,
                    // or ensure isValidMove itself handles intermediate blockers correctly for sliding.
                    // The current isValidMove does check intermediate blockers for sliding moves.
                    // However, for generating moves, we need to stop sliding if a piece is hit.
                    
                    if (isValidMove(board, targetPos)) {
                         validMoves.push_back(targetPos);
                    }

                    // If the current targetPos is occupied, subsequent moves in this direction are blocked
                    // unless the piece can jump (which isn't typical for sliding pieces, but check rule).
                    if (!rule.canJump && !board.getSquare(targetPos.x, targetPos.y).isEmpty()) {
                        break; 
                    }
                }
            }
        }
    }
    // Consider removing duplicates if rules could generate the same move, e.g. by using std::sort + std::unique
    return validMoves;
}

std::vector<Position> Piece::getInfluenceArea(const GameBoard& board) const {
    std::vector<Position> influenceArea;
    for (const auto& rule : stats.influenceRules) { // Iterate through influenceRules
        for (auto baseMove : rule.relativeMoves) { // Make a copy for potential pawn y-flip
            
            // Adjust y for PLAYER_TWO if pawn-like rule (though less common for influence)
            // Assuming influence rules for pawns are typically capture-like diagonals.
            if ((rule.isPawnForward || rule.isPawnCapture) && side == PlayerSide::PLAYER_TWO) {
                 baseMove.y *= -1;
            }

            if (rule.maxRange == 1) {
                Position targetPos = {position.x + baseMove.x, position.y + baseMove.y};
                if (board.isValidPosition(targetPos.x, targetPos.y)) {
                    influenceArea.push_back(targetPos);
                }
            } else { // Sliding influence
                for (int d = 1; d <= rule.maxRange; ++d) {
                    Position targetPos = {position.x + baseMove.x * d, position.y + baseMove.y * d};
                    if (!board.isValidPosition(targetPos.x, targetPos.y)) {
                        break; // Off board
                    }
                    influenceArea.push_back(targetPos);
                    // If influence cannot pass through other pieces (unless canJump is true)
                    if (!rule.canJump && !board.getSquare(targetPos.x, targetPos.y).isEmpty()) {
                        break; // Influence blocked by the first piece encountered
                    }
                }
            }
        }
    }
    // Consider removing duplicates, e.g. std::sort + std::unique
    return influenceArea;
}

} // namespace BayouBonanza
