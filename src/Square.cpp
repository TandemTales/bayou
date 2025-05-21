#include "Square.h"

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

} // namespace BayouBonanza
