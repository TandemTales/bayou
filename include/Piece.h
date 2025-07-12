#pragma once

#include <vector>
#include <memory>
#include <string>
#include "PlayerSide.h"
#include <SFML/Config.hpp> // For sf::Uint8
#include <SFML/Network/Packet.hpp> // For sf::Packet
#include "PieceData.h" // Added PieceData.h

namespace BayouBonanza {


// Forward declarations
class GameBoard;
class Square;

// Note: Position struct is defined in PieceData.h and imported via include

// SFML Packet operators for Position
inline sf::Packet& operator<<(sf::Packet& packet, const Position& position) {
    return packet << static_cast<sf::Int32>(position.x) << static_cast<sf::Int32>(position.y);
}

inline sf::Packet& operator>>(sf::Packet& packet, Position& position) {
    sf::Int32 x, y;
    packet >> x >> y;
    position.x = static_cast<int>(x);
    position.y = static_cast<int>(y);
    return packet;
}

/**
 * @brief Abstract base class for all game pieces
 * 
 * This class defines the common interface and properties for all piece types.
 */
class Piece {
public:
    /**
     * @brief Constructor
     * 
     * @param side The player that owns this piece
     * @param stats The statistical data for this piece type
     */
    Piece(PlayerSide side, const PieceStats& stats);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Piece() = default;
    
    /**
     * @brief Get the owner of the piece
     * 
     * @return The player side that owns this piece
     */
    PlayerSide getSide() const;
    
    /**
     * @brief Get the attack value of the piece
     * 
     * @return The attack value
     */
    int getAttack() const;
    
    /**
     * @brief Get the health value of the piece
     * 
     * @return The health value
     */
    int getHealth() const;
    
    /**
     * @brief Get the maximum health value of the piece
     * 
     * @return The maximum health value from piece stats
     */
    int getMaxHealth() const;
    
    /**
     * @brief Set the health value of the piece
     * 
     * @param health The new health value
     */
    void setHealth(int health);
    
    /**
     * @brief Apply damage to the piece
     * 
     * @param damage The amount of damage to apply
     * @return true if the piece is destroyed (health <= 0)
     */
    bool takeDamage(int damage);
    
    /**
     * @brief Get the current position of the piece
     * 
     * @return The position on the board
     */
    Position getPosition() const;
    
    /**
     * @brief Set the position of the piece
     * 
     * @param pos The new position
     */
    void setPosition(const Position& pos);
    
    // Modified function declarations (no longer pure virtual)
    virtual bool isValidMove(const GameBoard& board, const Position& target) const;
    virtual std::vector<Position> getValidMoves(const GameBoard& board) const;
    virtual std::vector<Position> getInfluenceArea(const GameBoard& board) const; // Kept virtual, implementation will be in .cpp
    virtual std::string getTypeName() const;
    virtual std::string getSymbol() const;
    bool isVictoryPiece() const;
    bool isRanged() const;
    bool canJump() const;

protected:
    PlayerSide side;
    int attack; // Will be initialized from stats
    int health; // Will be initialized from stats
    Position position;
    bool hasMoved;
    PieceStats stats; // Changed from const PieceStats& to PieceStats (store by value)

public:
    void setHasMoved(bool moved) { hasMoved = moved; }
    bool getHasMoved() const { return hasMoved; }
};

// SFML Packet operators for Piece (common data)
// Piece type and player side are handled externally by Square/Factory
inline sf::Packet& operator<<(sf::Packet& packet, const Piece& piece) {
    packet << piece.getSymbol(); // Using getSymbol()
    packet << piece.getPosition();
    packet << static_cast<sf::Int32>(piece.getHealth());
    packet << static_cast<sf::Int32>(piece.getAttack());
    packet << piece.getHasMoved(); // Use getHasMoved()
    return packet;
}

inline sf::Packet& operator>>(sf::Packet& packet, Piece& piece) {
    // Assumes 'piece' is an already-created concrete object of the correct type and side.
    // PlayerSide and piece type name should have been read by the caller (e.g., Square deserialization)
    // and used with PieceFactory to create 'piece'. The symbol is now derived from stats.
    std::string receivedSymbol; // Renamed to avoid conflict if symbol is a member or for clarity
    Position position;
    sf::Int32 health, attack;
    bool hasMovedFlag;

    packet >> receivedSymbol >> position >> health >> attack >> hasMovedFlag;

    // Verify symbol if necessary, though it's usually fixed by type.
    // if (receivedSymbol != piece.getSymbol()) { /* handle error or warning */ }

    piece.setPosition(position);
    piece.setHealth(static_cast<int>(health));
    // piece.setAttack(static_cast<int>(attack)); // Piece class doesn't have setAttack, it's const after construction.
                                                // This is fine, as attack is set by factory.
    piece.setHasMoved(hasMovedFlag);
    
    return packet;
}

} // namespace BayouBonanza
