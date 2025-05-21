#pragma once

#include <string>
#include "Piece.h"

namespace BayouBonanza {

/**
 * @brief Represents the Queen piece
 * 
 * The Queen is a powerful piece that can move in any direction
 * (horizontally, vertically, or diagonally) for any number of squares.
 */
class Queen : public Piece {
public:
    /**
     * @brief Constructor
     * 
     * @param side The player that owns this piece
     */
    Queen(PlayerSide side);
    
    /**
     * @brief Check if a move to the target position is valid
     * 
     * The Queen can move any number of squares horizontally, vertically, or diagonally,
     * but cannot jump over other pieces.
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
     * For the Queen, this includes all squares it can move to.
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

    std::string getSymbol() const override;
};

} // namespace BayouBonanza
