#pragma once

#include <vector>
#include <memory>
#include <string>
#include "PlayerSide.h"
#include <SFML/Config.hpp> // For sf::Uint8
#include <SFML/Network/Packet.hpp> // For sf::Packet
#include "PieceData.h" // Added PieceData.h

namespace BayouBonanza {

/**
 * @brief Enum representing different piece types
 */
enum class PieceType {
    KING,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT,
    PAWN
};

// SFML Packet operators for PieceType
inline sf::Packet& operator<<(sf::Packet& packet, const PieceType& type) {
    return packet << static_cast<sf::Uint8>(type);
}

inline sf::Packet& operator>>(sf::Packet& packet, PieceType& type) {
    sf::Uint8 type_uint8;
    packet >> type_uint8;
    type = static_cast<PieceType>(type_uint8);
    return packet;
}

// Forward declarations
class GameBoard;
class Square;

/**
 * @brief Structure representing a position on the board
 */
struct Position {
    int x;
    int y;
    
    Position(int x = 0, int y = 0) : x(x), y(y) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
};

// Position struct is now also in PieceData.h, ensure consistency or remove one.
// For now, assuming they are compatible or one will be removed later.

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
    virtual PieceType getPieceType() const; // Added this from previous pure virtual

protected:
    PlayerSide side;
    int attack; // Will be initialized from stats
    int health; // Will be initialized from stats
    Position position;
    bool hasMoved;
    const PieceStats& stats; // Added PieceStats member

public:
    // virtual PieceType getPieceType() const = 0; // This was already listed above, removed from here
    void setHasMoved(bool moved) { hasMoved = moved; }
    bool getHasMoved() const { return hasMoved; }
};

// SFML Packet operators for Piece (common data)
// Note: When serializing a Piece directly (e.g., from Square), PieceType should be written first.
// When deserializing, PieceType should be read first, then an appropriate Piece object
// created (e.g., via PieceFactory), and then this operator called on that object.

// SFML Packet operators for Piece (common data, excluding PieceType and PlayerSide, which are handled by Square/Factory)
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
    // PlayerSide and PieceType should have been read by the caller (e.g., Square deserialization)
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
