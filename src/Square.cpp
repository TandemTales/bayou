#include "Square.h"
#include "PlayerSide.h"   // Explicitly include for packet operators
#include "Piece.h"        // For std::unique_ptr<Piece>, PlayerSide
#include "PieceFactory.h" // For PieceFactory
#include <SFML/Network/Packet.hpp> // For sf::Packet
#include <iostream> // For std::cerr

namespace BayouBonanza {

// Static variable definition
PieceFactory* Square::globalPieceFactory = nullptr;

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

std::unique_ptr<Piece> Square::extractPiece() {
    return std::move(piece); // Transfers ownership, automatically sets piece to nullptr
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

void Square::setGlobalPieceFactory(PieceFactory* factory) {
    globalPieceFactory = factory;
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

// Updated operator>> 
sf::Packet& operator>>(sf::Packet& packet, Square& sq) {
    bool hasPiece;
    packet >> hasPiece;

    if (hasPiece) {
        PlayerSide side;
        std::string typeName; 
        packet >> side;      // Deserialize PlayerSide enum
        packet >> typeName;  // Deserialize typeName string

        // Use static globalPieceFactory to recreate the piece
        if (Square::globalPieceFactory) {
            std::unique_ptr<Piece> piece = Square::globalPieceFactory->createPiece(typeName, side);
            if (piece) {
                // Deserialize the piece data
                packet >> (*piece);
                sq.setPiece(std::move(piece));
            } else {
                std::cerr << "Error: Failed to create piece of type '" << typeName << "'" << std::endl;
                sq.setPiece(nullptr);
                // Skip the piece data in the packet to avoid misalignment
                std::string dummySymbol;
                Position dummyPosition;
                sf::Int32 dummyHealth, dummyAttack;
                bool dummyHasMoved;
                packet >> dummySymbol >> dummyPosition >> dummyHealth >> dummyAttack >> dummyHasMoved;
            }
        } else {
            std::cerr << "Error: Global PieceFactory not available for deserialization" << std::endl;
            sq.setPiece(nullptr);
            // Skip the piece data in the packet to avoid misalignment
            std::string dummySymbol;
            Position dummyPosition;
            sf::Int32 dummyHealth, dummyAttack;
            bool dummyHasMoved;
            packet >> dummySymbol >> dummyPosition >> dummyHealth >> dummyAttack >> dummyHasMoved;
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
