#pragma once

#include <memory>
#include "PlayerSide.h" // PlayerSide enum
#include "Piece.h"      // PieceType enum, Piece class
#include "PieceFactory.h" // For PieceFactory
#include <SFML/Network/Packet.hpp> // For sf::Packet

namespace BayouBonanza {

// Forward declaration to avoid circular includes
class Piece;

/**
 * @brief Represents a single square on the game board
 * 
 * Each square may contain a piece and has control values for both players.
 * Control values determine which player has influence over this square.
 */
class Square {
public:
    /**
     * @brief Default constructor, initializes an empty square with no control
     */
    Square();
    
    /**
     * @brief Check if the square is empty (has no piece)
     * 
     * @return true if square has no piece
     */
    bool isEmpty() const;
    
    /**
     * @brief Get the piece on this square
     * 
     * @return Pointer to the piece, or nullptr if empty
     */
    std::shared_ptr<Piece> getPiece() const;
    
    /**
     * @brief Set a piece on this square
     * 
     * @param piece Pointer to the piece to place
     */
    void setPiece(std::shared_ptr<Piece> piece);
    
    /**
     * @brief Get the control value for a specific player
     * 
     * @param side The player side to check
     * @return Control value (higher means more control)
     */
    int getControlValue(PlayerSide side) const;
    
    /**
     * @brief Set the control value for a specific player
     * 
     * @param side The player side to set
     * @param value The control value to set
     */
    void setControlValue(PlayerSide side, int value);
    
    /**
     * @brief Determine which player has control of this square
     * 
     * @return The player side that controls this square, or NEUTRAL if tied
     */
    PlayerSide getControlledBy() const;

private:
    std::shared_ptr<Piece> piece; // nullptr if empty
    int controlValuePlayer1;      // Control value for player 1
    int controlValuePlayer2;      // Control value for player 2
};

// SFML Packet operators for Square
sf::Packet& operator<<(sf::Packet& packet, const Square& sq);
sf::Packet& operator>>(sf::Packet& packet, Square& sq);

} // namespace BayouBonanza
