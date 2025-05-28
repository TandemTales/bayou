#include "Square.h"
#include "PlayerSide.h"   // Explicitly include for packet operators
#include "Piece.h"        // For std::unique_ptr<Piece>, PlayerSide
#include "PieceFactory.h" // For PieceFactory
#include <SFML/Network/Packet.hpp> // For sf::Packet
#include <iostream> // For std::cerr

// Note: Piece.h should already include PlayerSide.h.
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

Piece* Square::getPiece() const { // Changed return type
    return piece.get();
}

void Square::setPiece(std::unique_ptr<Piece> p) { // Changed parameter type
    this->piece = std::move(p);
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
        // const std::unique_ptr<Piece>& currentPiece = sq.getPiece(); // This would be problematic due to unique_ptr move semantics
        const Piece* currentPiece = sq.getPiece(); // Use the raw pointer getter
        // It's crucial that PlayerSide is already handled for sf::Packet
        // and that Piece::operator<< only handles common data (excluding side and typeName).
        packet << currentPiece->getSide();           // Serialize PlayerSide enum
        packet << currentPiece->getTypeName();     // Serialize typeName string
        packet << (*currentPiece);                 // Serialize common Piece data
    }

    packet << static_cast<sf::Int32>(sq.getControlValue(PlayerSide::PLAYER_ONE));
    packet << static_cast<sf::Int32>(sq.getControlValue(PlayerSide::PLAYER_TWO));
    return packet;
}

// Updated operator>> to include PieceFactory reference
sf::Packet& operator>>(sf::Packet& packet, Square& sq, PieceFactory& factory) {
    bool hasPiece;
    packet >> hasPiece;

    if (hasPiece) {
        PlayerSide side;
        std::string typeName; 
        packet >> side;      // Deserialize PlayerSide enum
        packet >> typeName;  // Deserialize typeName string

        std::unique_ptr<Piece> piece = factory.createPiece(typeName, side);
        if (piece) {
            packet >> (*piece); // Deserialize common Piece data into the created piece
            sq.setPiece(std::move(piece));
        } else {
            std::cerr << "Error: Failed to create piece of type '" << typeName << "' during Square deserialization." << std::endl;
            // If piece creation fails, we can't deserialize the rest of its specific data.
            // This could lead to packet stream misalignment if not handled carefully.
            // For now, we set the piece to nullptr and hope the packet stream can recover
            // for subsequent Square data (control values). This is risky.
            // A more robust error handling strategy would be needed in a real application,
            // potentially by reading a "dummy" piece of expected size or failing the stream.
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
