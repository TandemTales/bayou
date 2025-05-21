#include "PieceFactory.h"
#include "King.h"
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"
#include "Knight.h"
#include "Pawn.h"

namespace BayouBonanza {

std::shared_ptr<Piece> PieceFactory::createKing(PlayerSide side) {
    return std::make_shared<King>(side);
}

std::shared_ptr<Piece> PieceFactory::createQueen(PlayerSide side) {
    return std::make_shared<Queen>(side);
}

std::shared_ptr<Piece> PieceFactory::createRook(PlayerSide side) {
    return std::make_shared<Rook>(side);
}

std::shared_ptr<Piece> PieceFactory::createBishop(PlayerSide side) {
    return std::make_shared<Bishop>(side);
}

std::shared_ptr<Piece> PieceFactory::createKnight(PlayerSide side) {
    return std::make_shared<Knight>(side);
}

std::shared_ptr<Piece> PieceFactory::createPawn(PlayerSide side) {
    return std::make_shared<Pawn>(side);
}

std::shared_ptr<Piece> PieceFactory::createPieceByType(const std::string& typeName, PlayerSide side) {
    if (typeName == "King") {
        return createKing(side);
    } else if (typeName == "Queen") {
        return createQueen(side);
    } else if (typeName == "Rook") {
        return createRook(side);
    } else if (typeName == "Bishop") {
        return createBishop(side);
    } else if (typeName == "Knight") {
        return createKnight(side);
    } else if (typeName == "Pawn") {
        return createPawn(side);
    }
    
    // Unknown piece type
    return nullptr;
}

} // namespace BayouBonanza
