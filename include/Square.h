#pragma once

#include <memory>
#include "PlayerSide.h" // PlayerSide enum
#include "Piece.h"      // Piece class
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
 * Control is persistent - once gained, it's only lost when another player gains MORE influence.
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
     * @brief Get the current influence value for a specific player
     * 
     * @param side The player side to check
     * @return Current influence value (reset each turn)
     */
    int getControlValue(PlayerSide side) const;
    
    /**
     * @brief Set the current influence value for a specific player
     * 
     * @param side The player side to set
     * @param value The influence value to set
     */
    void setControlValue(PlayerSide side, int value);
    
    /**
     * @brief Get the player that currently controls this square (persistent)
     * 
     * @return PlayerSide that controls this square (NEUTRAL if no one has ever controlled it)
     */
    PlayerSide getControlledBy() const;
    
    /**
     * @brief Set the controlling player for this square (used by InfluenceSystem)
     * 
     * @param controller The player that should control this square
     */
    void setControlledBy(PlayerSide controller);
    
    /**
     * @brief Update control based on current influence values (implements sticky control logic)
     * 
     * Control only changes when another player has MORE influence than the current controller.
     * If no one has ever controlled this square, the player with more influence gains control.
     */
    void updateControlFromInfluence();
    
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
    int controlValuePlayer1;      // Current influence value for player 1 (reset each turn)
    int controlValuePlayer2;      // Current influence value for player 2 (reset each turn)
    PlayerSide currentController; // Persistent control - who actually controls this square
};

// SFML Packet operators for Square
sf::Packet& operator<<(sf::Packet& packet, const Square& sq);
sf::Packet& operator>>(sf::Packet& packet, Square& sq);


} // namespace BayouBonanza
