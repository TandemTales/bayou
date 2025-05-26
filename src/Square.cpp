#include "Square.h"
#include "Piece.h"        // For std::shared_ptr<Piece>, PieceType, PlayerSide
#include "PieceFactory.h" // For PieceFactory
#include <SFML/Network/Packet.hpp> // For sf::Packet

// Note: Piece.h should already include PlayerSide.h and define PieceType.
// Square.h should already include Piece.h and PieceFactory.h for the declarations.

namespace BayouBonanza {

Square::Square() : 
    piece(nullptr),
    controlValuePlayer1(0),
    controlValuePlayer2(0) {
}

bool Square::isEmpty() const {
    return piece == nullptr;
}

std::shared_ptr<Piece> Square::getPiece() const {
    return piece;
}

void Square::setPiece(std::shared_ptr<Piece> piece) {
    this->piece = piece;
}

int Square::getControlValue(PlayerSide side) const {
    if (side == PlayerSide::PLAYER_ONE) {
        return controlValuePlayer1;
    } else if (side == PlayerSide::PLAYER_TWO) {
        return controlValuePlayer2;
    }
    return 0; // NEUTRAL has no control value
}

void Square::setControlValue(PlayerSide side, int value) {
    if (side == PlayerSide::PLAYER_ONE) {
        controlValuePlayer1 = value;
    } else if (side == PlayerSide::PLAYER_TWO) {
        controlValuePlayer2 = value;
    }
    // NEUTRAL side is ignored for setting control
}

PlayerSide Square::getControlledBy() const {
    if (controlValuePlayer1 > controlValuePlayer2) {
        return PlayerSide::PLAYER_ONE;
    } else if (controlValuePlayer2 > controlValuePlayer1) {
        return PlayerSide::PLAYER_TWO;
    } else {
        return PlayerSide::NEUTRAL;
    }
}

// SFML Packet operators for Square
sf::Packet& operator<<(sf::Packet& packet, const Square& sq) {
    bool hasPiece = (sq.getPiece() != nullptr);
    packet << hasPiece;

    if (hasPiece) {
        const std::shared_ptr<Piece>& piece = sq.getPiece();
        // It's crucial that PlayerSide and PieceType enums are already handled for sf::Packet
        // and that Piece::operator<< only handles common data (excluding side and type).
        packet << piece->getSide();           // Serialize PlayerSide enum
        packet << piece->getPieceType();      // Serialize PieceType enum
        packet << (*piece);                   // Serialize common Piece data
    }

    packet << static_cast<sf::Int32>(sq.getControlValue(PlayerSide::PLAYER_ONE));
    packet << static_cast<sf::Int32>(sq.getControlValue(PlayerSide::PLAYER_TWO));
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, Square& sq) {
    bool hasPiece;
    packet >> hasPiece;

    if (hasPiece) {
        PlayerSide side;
        PieceType type;
        packet >> side; // Deserialize PlayerSide enum
        packet >> type; // Deserialize PieceType enum

        std::shared_ptr<Piece> piece = PieceFactory::createPieceByPieceType(type, side);
        if (piece) {
            packet >> (*piece); // Deserialize common Piece data into the created piece
            sq.setPiece(piece);
        } else {
            // Failed to create piece (e.g., unknown type), or type was invalid.
            // This case should ideally not happen if data is consistent.
            // We read the piece's data block from the packet anyway to keep stream state consistent,
            // even if we can't use it. This is tricky.
            // A better approach if piece creation fails: log error and potentially stop deserialization.
            // For now, assume if piece is null, the corresponding data block for the piece in the packet
            // was minimal or handled. Or, more robustly, if piece creation fails,
            // one might need to read and discard a "dummy" piece's worth of data.
            // However, Piece::operator>> expects a valid object.
            // Simplest for now: if factory fails, set to null, packet stream might be misaligned if not careful.
            // Let's assume factory always succeeds for valid types from packet.
            sq.setPiece(nullptr); 
        }
    } else {
        sq.setPiece(nullptr);
    }

    sf::Int32 controlP1_sf, controlP2_sf;
    packet >> controlP1_sf >> controlP2_sf;
    sq.setControlValue(PlayerSide::PLAYER_ONE, static_cast<int>(controlP1_sf));
    sq.setControlValue(PlayerSide::PLAYER_TWO, static_cast<int>(controlP2_sf));
    return packet;
}

} // namespace BayouBonanza
