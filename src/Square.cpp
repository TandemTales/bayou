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
    controlValuePlayer2(0),
    currentController(PlayerSide::NEUTRAL) {
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
    return currentController;
}

void Square::setControlledBy(PlayerSide controller) {
    currentController = controller;
}

void Square::updateControlFromInfluence() {
    // Sticky control logic:
    // 1. If no one has ever controlled this square, highest influence wins
    // 2. If someone controls it, they keep it unless another player has MORE influence
    // 3. Ties go to the current controller
    
    if (currentController == PlayerSide::NEUTRAL) {
        // No one has ever controlled this square - highest influence wins
        if (controlValuePlayer1 > controlValuePlayer2) {
            currentController = PlayerSide::PLAYER_ONE;
        } else if (controlValuePlayer2 > controlValuePlayer1) {
            currentController = PlayerSide::PLAYER_TWO;
        }
        // If tied at 0 or equal non-zero values, remains neutral
    } else if (currentController == PlayerSide::PLAYER_ONE) {
        // Player One currently controls - they keep it unless Player Two has MORE influence
        // Player Two needs STRICTLY MORE influence than Player One to take control
        if (controlValuePlayer2 > controlValuePlayer1) {
            currentController = PlayerSide::PLAYER_TWO;
        }
        // Player One retains control in all other cases (including ties)
    } else if (currentController == PlayerSide::PLAYER_TWO) {
        // Player Two currently controls - they keep it unless Player One has MORE influence
        // Player One needs STRICTLY MORE influence than Player Two to take control
        if (controlValuePlayer1 > controlValuePlayer2) {
            currentController = PlayerSide::PLAYER_ONE;
        }
        // Player Two retains control in all other cases (including ties)
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
    packet << static_cast<sf::Uint8>(sq.getControlledBy()); // Serialize persistent controller
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
    
    // Deserialize persistent controller
    sf::Uint8 controller_uint8;
    packet >> controller_uint8;
    sq.setControlledBy(static_cast<PlayerSide>(controller_uint8));
    
    return packet;
}

} // namespace BayouBonanza
