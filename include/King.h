#pragma once

#include <string>
#include "Piece.h"

namespace BayouBonanza {

/**
 * @brief Represents the King piece
 * 
 * The King is a special piece that each player starts with.
 * The game ends when a player's King is destroyed.
 */
class King : public Piece {
public:
    /**
     * @brief Constructor
     * 
     * @param side The player that owns this piece
     */
    King(PlayerSide side);
    
    /**
     * @brief Check if a move to the target position is valid
     * 
     * The King can move one square in any direction.
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
     * @brief Get the type name of the piece
     * 
     * @return String representation of the piece type
     */
    std::string getTypeName() const override;

    std::string getSymbol() const override;

    PieceType getPieceType() const override;
};

} // namespace BayouBonanza
