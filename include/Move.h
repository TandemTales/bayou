#pragma once

#include <memory>
#include "Piece.h" // Includes Position
#include <SFML/Network/Packet.hpp> // For sf::Packet

namespace BayouBonanza {

/**
 * @brief Represents a move of a piece from one position to another
 */
class Move {
public:
    /**
     * @brief Default constructor (for deserialization)
     */
    Move();

    /**
     * @brief Constructor
     * 
     * @param piece The piece to move
     * @param from Starting position
     * @param to Target position
     */
    Move(std::shared_ptr<Piece> piece, const Position& from, const Position& to);

    /**
     * @brief Constructor for promotion moves
     * 
     * @param piece The piece being moved (usually a Pawn)
     * @param from Starting position
     * @param to Target position (promotion square)
     * @param promotionType The type of piece to promote to
     */
    Move(std::shared_ptr<Piece> piece, const Position& from, const Position& to, const std::string& promotionType);
    
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

    /**
     * @brief Check if this move is a promotion
     * @return true if it's a promotion move
     */
    bool isPromotion() const;

    /**
     * @brief Get the type of piece to promote to (if isPromotion is true)
     * @return name of piece type to promote to
     */
    const std::string& getPromotionType() const;

    // Friend functions for SFML packet operators
    friend sf::Packet& operator<<(sf::Packet& packet, const Move& mv);
    friend sf::Packet& operator>>(sf::Packet& packet, Move& mv);

private:
    std::shared_ptr<Piece> piece_; // The actual piece object, not serialized directly by Move's operators
    Position from_;
    Position to_;
    bool isPromotion_;
    std::string pieceTypePromotedTo_; // Type to promote to by name
};

// SFML Packet operators for Move
sf::Packet& operator<<(sf::Packet& packet, const Move& mv);
sf::Packet& operator>>(sf::Packet& packet, Move& mv);

} // namespace BayouBonanza
