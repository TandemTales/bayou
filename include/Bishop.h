#pragma once

#include <string>
#include "Piece.h"

namespace BayouBonanza {

/**
 * @brief Represents the Bishop piece
 * 
 * The Bishop can move any number of squares diagonally.
 */
class Bishop : public Piece {
public:
    /**
     * @brief Constructor
     * 
     * @param side The player that owns this piece
     */
    Bishop(PlayerSide side);
    
    /**
     * @brief Check if a move to the target position is valid
     * 
     * The Bishop can move any number of squares diagonally,
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
     * For the Bishop, this includes all squares it can move to.
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

    PieceType getPieceType() const override;
};

} // namespace BayouBonanza
