#include "../include/CombatCalculator.h"
#include "../include/Queen.h"
#include "../include/Rook.h"
#include "../include/Bishop.h"
#include "../include/Knight.h"
#include "../include/Pawn.h"
#include "../include/King.h"
#include "../include/Piece.h"
#include <algorithm>

namespace BayouBonanza {

CombatResult CombatCalculator::calculateCombat(std::shared_ptr<Piece> attacker, std::shared_ptr<Piece> defender) {
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

int CombatCalculator::calculateDamage(std::shared_ptr<Piece> attacker, std::shared_ptr<Piece> defender) {
    if (!attacker || !defender) {
        return 0;
    }
    
    int baseDamage = attacker->getAttack();
    int finalDamage = baseDamage;
    
    // Implement piece-specific damage bonuses/penalties
    // For example, certain pieces might have advantages against other piece types
    
    // Example of type-based damage modifiers
    if (dynamic_cast<Queen*>(attacker.get())) {
        // Queen gets bonus damage against Pawns
        if (dynamic_cast<Pawn*>(defender.get())) {
            finalDamage = static_cast<int>(baseDamage * 1.5);
        }
    } else if (dynamic_cast<Knight*>(attacker.get())) {
        // Knights get bonus damage against Bishop and Rook
        if (dynamic_cast<Bishop*>(defender.get()) || dynamic_cast<Rook*>(defender.get())) {
            finalDamage = static_cast<int>(baseDamage * 1.25);
        }
    } else if (dynamic_cast<Pawn*>(attacker.get())) {
        // Pawns get bonus damage against other Pawns
        if (dynamic_cast<Pawn*>(defender.get())) {
            finalDamage = static_cast<int>(baseDamage * 1.2);
        }
    }
    
    // Ensure damage is at least 1 if an attack happens
    return std::max(1, finalDamage);
}

bool CombatCalculator::isDefeated(std::shared_ptr<Piece> piece) {
    return piece && piece->getHealth() <= 0;
}

// Method removed - no counter-attacks in the game

} // namespace BayouBonanza
