#include "Move.h"
// Piece.h (for Position) and SFML/Network/Packet.hpp are included via Move.h

namespace BayouBonanza {

// Default constructor (for deserialization)
Move::Move() :
    piece_(nullptr),
    from_(Position{0, 0}),
    to_(Position{0, 0}),
    isPromotion_(false),
    pieceTypePromotedTo_("") {
}

// Standard move constructor
Move::Move(std::shared_ptr<Piece> piece, const Position& from, const Position& to) :
    piece_(piece),
    from_(from),
    to_(to),
    isPromotion_(false),
    pieceTypePromotedTo_("") { // Default, not used if not promotion
}

// Promotion move constructor
Move::Move(std::shared_ptr<Piece> piece, const Position& from, const Position& to, const std::string& promotionType) :
    piece_(piece),
    from_(from),
    to_(to),
    isPromotion_(true),
    pieceTypePromotedTo_(promotionType) {
}

std::shared_ptr<Piece> Move::getPiece() const {
    return piece_;
}

const Position& Move::getFrom() const {
    return from_;
}

const Position& Move::getTo() const {
    return to_;
}

bool Move::isPromotion() const {
    return isPromotion_;
}

const std::string& Move::getPromotionType() const {
    return pieceTypePromotedTo_;
}

// SFML Packet operators for Move
sf::Packet& operator<<(sf::Packet& packet, const Move& mv) {
    packet << mv.getFrom(); // Uses Position's operator<<
    packet << mv.getTo();   // Uses Position's operator<<
    packet << mv.isPromotion();
    if (mv.isPromotion()) {
        packet << mv.getPromotionType();
    }
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, Move& mv) {
    Position from, to;
    bool isPromotion;
    std::string promotionType;

    packet >> from >> to >> isPromotion;
    if (isPromotion) {
        packet >> promotionType;
    }

    // The piece_ member of mv is not set here as it's not transmitted.
    // The server reconstructs the context of the move.
    // For the client, if it were to receive a Move (less common), 
    // it would also need context or this Move object would be partial.
    mv.from_ = from;
    mv.to_ = to;
    mv.isPromotion_ = isPromotion;
    mv.pieceTypePromotedTo_ = promotionType;
    mv.piece_ = nullptr; // Explicitly set piece_ to nullptr as it's not part of serialization

    return packet;
}

} // namespace BayouBonanza
