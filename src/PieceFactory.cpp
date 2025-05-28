#include "PieceFactory.h"
#include "PieceDefinitionManager.h" // Include the manager's header
#include "Piece.h"                  // For Piece implementation
#include <iostream>                 // For error messages

namespace BayouBonanza {

PieceFactory::PieceFactory(const PieceDefinitionManager& manager) : definitionManager(manager) {}

std::unique_ptr<Piece> PieceFactory::createPiece(const std::string& typeName, PlayerSide side) {
    const PieceStats* stats = definitionManager.getPieceStats(typeName);

    if (!stats) {
        std::cerr << "Error: PieceFactory could not create piece of type '" << typeName << "'. Stats not found." << std::endl;
        return nullptr; // Or throw an exception
    }

    // Create a new generic Piece using the stats
    // The Piece constructor now takes PieceStats: Piece(PlayerSide side, const PieceStats& stats)
    std::unique_ptr<Piece> newPiece = std::make_unique<Piece>(side, *stats);
    
    // newPiece->setPosition(...) will likely be set by GameBoard or GameInitializer after creation.
    // newPiece->setHasMoved(false); // This is handled by Piece constructor default

    return newPiece;
}

} // namespace BayouBonanza
