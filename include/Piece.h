#pragma once

#include <vector>
#include <memory>
#include <string>
#include "PlayerSide.h"

namespace BayouBonanza {

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
     * @param attack The attack value of the piece
     * @param health The health value of the piece
     */
    Piece(PlayerSide side, int attack, int health);
    
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
    
    /**
     * @brief Check if a move to the target position is valid
     * 
     * @param board The game board
     * @param target The target position
     * @return true if the move is valid
     */
    virtual bool isValidMove(const GameBoard& board, const Position& target) const = 0;
    
    /**
     * @brief Get all valid moves for this piece
     * 
     * @param board The game board
     * @return Vector of valid positions this piece can move to
     */
    virtual std::vector<Position> getValidMoves(const GameBoard& board) const = 0;
    
    /**
     * @brief Get the area of influence for this piece
     * 
     * This returns all squares that this piece exerts control over.
     * 
     * @param board The game board
     * @return Vector of positions that this piece influences
     */
    virtual std::vector<Position> getInfluenceArea(const GameBoard& board) const;
    
    /**
     * @brief Get the type name of the piece
     * 
     * @return String representation of the piece type
     */
    virtual std::string getTypeName() const = 0;

    // Pure virtual function for getting the piece symbol
    virtual std::string getSymbol() const = 0;

protected:
    PlayerSide side;
    int attack;
    int health;
    Position position;
    bool hasMoved; // Added for serialization

public:
    // Add virtual getPieceType method
    virtual PieceType getPieceType() const = 0;
    void setHasMoved(bool moved) { hasMoved = moved; } // Setter for hasMoved
    bool getHasMoved() const { return hasMoved; }      // Getter for hasMoved
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
    // and used with PieceFactory to create 'piece'.
    std::string symbol; // Read for verification or if symbols can change dynamically.
    Position position;
    sf::Int32 health, attack;
    bool hasMovedFlag;

    packet >> symbol >> position >> health >> attack >> hasMovedFlag;

    // Verify symbol if necessary, though it's usually fixed by type.
    // if (symbol != piece.getSymbol()) { /* handle error or warning */ }

    piece.setPosition(position);
    piece.setHealth(static_cast<int>(health));
    // piece.setAttack(static_cast<int>(attack)); // Piece class doesn't have setAttack, it's const after construction.
                                                // This is fine, as attack is set by factory.
    piece.setHasMoved(hasMovedFlag);
    
    return packet;
}

} // namespace BayouBonanza
