#pragma once

#include <string>
#include "Piece.h"

namespace BayouBonanza {

/**
 * @brief Represents the Pawn piece
 * 
 * The Pawn can move one square forward but captures diagonally.
 */
class Pawn : public Piece {
public:
    /**
     * @brief Constructor
     * 
     * @param side The player that owns this piece
     */
    Pawn(PlayerSide side);
    
    /**
     * @brief Check if a move to the target position is valid
     * 
     * The Pawn can move one square forward (relative to its side),
     * or capture one square diagonally forward.
     * 
     * @param board The game board
     * @param target The target position
     * @return true if the move is valid
     */
    bool isValidMove(const GameBoard& board, const Position& target) const override;
    
    /**
     * @brief Get all valid moves for this piece
     * 
     * @param board The game board
     * @return Vector of valid positions this piece can move to
     */
    std::vector<Position> getValidMoves(const GameBoard& board) const override;
    
    /**
     * @brief Get the area of influence for this piece
     * 
     * For the Pawn, this includes the diagonal squares it can capture on.
     * 
     * @param board The game board
     * @return Vector of positions that this piece influences
     */
    std::vector<Position> getInfluenceArea(const GameBoard& board) const override;
    
    /**
     * @brief Get the type name of the piece
     * 
     * @return String representation of the piece type
     */
    std::string getTypeName() const override;
};

} // namespace BayouBonanza
