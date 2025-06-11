#pragma once

#include <string>
#include <vector>



// Forward declaration if Position is defined elsewhere, or define it here
// For now, let's assume Position is defined elsewhere and included where needed,
// or it's simple enough to be defined here if not.
// If it's a common type, it should ideally be in its own header.
struct Position {
    int x;
    int y;
    
    Position(int x = 0, int y = 0) : x(x), y(y) {}

    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Position& other) const {
        return !(*this == other);
    }
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
    // Path to static sprite image (optional)
    std::string spritePath;
    // Path to sprite sheet for animations (optional)
    std::string spritesheetPath;
    // Card art used when rendering piece cards (optional)
    std::string cardArtPath;
    int attack;
    int health;
    std::vector<PieceMovementRule> movementRules;
    std::vector<PieceMovementRule> influenceRules;
    bool isRanged{false};
    bool isVictoryPiece{false};
};
