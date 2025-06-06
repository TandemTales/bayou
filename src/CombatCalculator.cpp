#include "CombatCalculator.h"
// #include "../include/Queen.h" // Removed - using data-driven approach
// #include "../include/Rook.h" // Removed - using data-driven approach
// #include "../include/Bishop.h" // Removed - using data-driven approach
// #include "../include/Knight.h" // Removed - using data-driven approach
// #include "../include/Pawn.h" // Removed - using data-driven approach
// #include "../include/King.h" // Removed - using data-driven approach
#include "Piece.h"
#include "GameBoard.h"
#include "Square.h"
#include <algorithm>

namespace BayouBonanza {

CombatResult CombatCalculator::calculateCombat(Piece* attacker, Piece* defender) {
    CombatResult result;
    
    // Calculate base damage
    result.damageDealt = calculateDamage(attacker, defender);
    
    // Apply damage to defender and check if defeated
    bool originallyAlive = defender->getHealth() > 0;
    defender->takeDamage(result.damageDealt);
    result.targetDefeated = originallyAlive && defender->getHealth() <= 0;
    
    // No counter-attack functionality as per user request
    
    return result;
}

int CombatCalculator::calculateDamage(Piece* attacker, Piece* defender) {
    if (!attacker || !defender) {
        return 0;
    }
    
    int baseDamage = attacker->getAttack();
    int finalDamage = baseDamage;

    // Type-based bonuses removed for simplicity

    // Ensure damage is at least 1 if an attack happens
    return std::max(1, finalDamage);
}

bool CombatCalculator::isDefeated(Piece* piece) {
    return !piece || piece->getHealth() <= 0;
}

// Method removed - no counter-attacks in the game

} // namespace BayouBonanza
