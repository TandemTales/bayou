#pragma once

#include <string>
#include <vector>
#include "Piece.h" // Assuming Piece.h is in the include directory

// Forward declaration if Position is defined elsewhere, or define it here
// For now, let's assume Position is defined elsewhere and included where needed,
// or it's simple enough to be defined here if not.
// If it's a common type, it should ideally be in its own header.
struct Position {
    int x;
    int y;
};

struct PieceMovementRule {
    std::vector<Position> relativeMoves;
    bool isPawnForward{false};
    bool isPawnCapture{false};
    bool canJump{false};
    int maxRange{1};
};

struct PieceStats {
    std::string typeName;
    std::string symbol;
    int attack;
    int health;
    std::vector<PieceMovementRule> movementRules;
    std::vector<PieceMovementRule> influenceRules;
    PieceType pieceType; // From Piece.h
};
