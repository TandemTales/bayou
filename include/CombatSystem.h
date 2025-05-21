#ifndef BAYOU_BONANZA_COMBAT_SYSTEM_H
#define BAYOU_BONANZA_COMBAT_SYSTEM_H

#include "GameBoard.h"
#include "PlayerSide.h"
#include "Piece.h"

namespace BayouBonanza {

struct CombatResult {
    int damageDealt;
    bool targetDefeated;
};

class CombatSystem {
public:
    // Resolves combat between pieces at the attacker and defender positions
    static bool resolveCombat(GameBoard& board, const Position& attacker, const Position& defender);
    
    // Applies damage from attacker to defender
    static void applyDamage(std::shared_ptr<Piece> attacker, std::shared_ptr<Piece> defender);
    
    // Checks entire board for dead pieces and removes them
    static void checkAndRemoveDeadPieces(GameBoard& board);
    
    // Checks if a specific square has a dead piece and removes it if necessary
    static bool checkAndRemoveDeadPiece(GameBoard& board, const Position& position);
    
    // Determines if two pieces can engage in combat
    static bool canEngageInCombat(const GameBoard& board, const Position& attacker, const Position& defender);
};

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_COMBAT_SYSTEM_H
