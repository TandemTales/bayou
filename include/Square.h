#pragma once

#include <memory>
#include "PlayerSide.h" // PlayerSide enum
#include "Piece.h"      // PieceType enum, Piece class
#include "PieceFactory.h" // For PieceFactory
#include <SFML/Network/Packet.hpp> // For sf::Packet

namespace BayouBonanza {

// Forward declaration to avoid circular includes
class Piece;
class PieceFactory;

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
    Piece* getPiece() const; // Return raw pointer for observation
    
    /**
     * @brief Set a piece on this square
     * 
     * @param piece Unique pointer to the piece to place (Square takes ownership)
     */
    void setPiece(std::unique_ptr<Piece> piece);
    
    /**
     * @brief Extract the piece from this square, transferring ownership
     * 
     * @return Unique pointer to the piece (caller takes ownership), nullptr if empty
     */
    std::unique_ptr<Piece> extractPiece();
    
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
     * @brief Get the player that controls this square
     * 
     * @return PlayerSide that controls this square (NEUTRAL if tied)
     */
    PlayerSide getControlledBy() const;
    
    /**
     * @brief Set the global PieceFactory for deserialization
     * 
     * @param factory Pointer to the PieceFactory to use
     */
    static void setGlobalPieceFactory(PieceFactory* factory);

    // PieceFactory needs to be accessible for deserialization.
    // This is a design challenge. For now, we assume it can be accessed.
    // One common way is to pass it to the deserialization operator,
    // e.g., operator>>(sf::Packet& packet, Square& sq, PieceFactory& factory);
    // Or, GameState could hold the factory.

    // Static PieceFactory pointer for deserialization
    static PieceFactory* globalPieceFactory;

private:
    std::unique_ptr<Piece> piece; // nullptr if empty, Square owns the piece
    int controlValuePlayer1;      // Control value for player 1
    int controlValuePlayer2;      // Control value for player 2
};

// SFML Packet operators for Square
sf::Packet& operator<<(sf::Packet& packet, const Square& sq);
sf::Packet& operator>>(sf::Packet& packet, Square& sq);


} // namespace BayouBonanza
