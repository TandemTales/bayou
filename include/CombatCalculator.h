#ifndef BAYOU_BONANZA_COMBAT_CALCULATOR_H
#define BAYOU_BONANZA_COMBAT_CALCULATOR_H

#include "Piece.h"
#include <memory>

namespace BayouBonanza {

struct CombatResult {
    int damageDealt;
    bool targetDefeated;
    // Counter-attack functionality removed per user request
};

class CombatCalculator {
public:
    // Calculate damage from attacker to defender (one-directional)
    static CombatResult calculateCombat(Piece* attacker, Piece* defender);
    
    // Calculate damage based on piece types, can include bonuses/penalties
    static int calculateDamage(Piece* attacker, Piece* defender);
    
    // Check if a piece is defeated (health <= 0)
    static bool isDefeated(Piece* piece);
    
    // Method removed - no counter-attacks in the game
};

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_COMBAT_CALCULATOR_H
