#include "Move.h"

namespace BayouBonanza {

Move::Move(std::shared_ptr<Piece> piece, const Position& from, const Position& to) :
    piece(piece),
    from(from),
    to(to) {
}

std::shared_ptr<Piece> Move::getPiece() const {
    return piece;
}

const Position& Move::getFrom() const {
    return from;
}

const Position& Move::getTo() const {
    return to;
}

} // namespace BayouBonanza
