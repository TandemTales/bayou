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

protected:
    PlayerSide side;
    int attack;
    int health;
    Position position;
};

} // namespace BayouBonanza
