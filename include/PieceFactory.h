#pragma once

#include <memory>
#include <string>
#include "Piece.h"        // For std::unique_ptr<Piece> and PlayerSide
// #include "PieceType.h" // This might no longer be needed here if createPiece changes signature

// Forward declare PieceDefinitionManager
namespace BayouBonanza {
class PieceDefinitionManager; // Forward declaration

class PieceFactory {
public:
    // Constructor now takes a reference to the definition manager
    PieceFactory(const PieceDefinitionManager& manager);

    // createPiece takes a type name string allowing for data-driven pieces
    std::unique_ptr<Piece> createPiece(const std::string& typeName, PlayerSide side);

    const PieceDefinitionManager& getDefinitionManager() const { return definitionManager; }

    bool isVictoryPiece(const std::string& typeName) const;

private:
    const PieceDefinitionManager& definitionManager;
};

} // namespace BayouBonanza
