#pragma once

#include <memory>
#include "Piece.h"

namespace BayouBonanza {

/**
 * @brief Represents a move of a piece from one position to another
 */
class Move {
public:
    /**
     * @brief Constructor
     * 
     * @param piece The piece to move
     * @param from Starting position
     * @param to Target position
     */
    Move(std::shared_ptr<Piece> piece, const Position& from, const Position& to);
    
    /**
     * @brief Get the piece being moved
     * 
     * @return Pointer to the piece
     */
    std::shared_ptr<Piece> getPiece() const;
    
    /**
     * @brief Get the starting position
     * 
     * @return Starting position
     */
    const Position& getFrom() const;
    
    /**
     * @brief Get the target position
     * 
     * @return Target position
     */
    const Position& getTo() const;

private:
    std::shared_ptr<Piece> piece;
    Position from;
    Position to;
};

} // namespace BayouBonanza
