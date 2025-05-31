#ifndef BAYOU_BONANZA_COMBAT_SYSTEM_H
#define BAYOU_BONANZA_COMBAT_SYSTEM_H

#include "GameBoard.h"
#include "PlayerSide.h"
#include "Piece.h"
#include "CombatCalculator.h"  // Include for CombatResult

namespace BayouBonanza {

class CombatSystem {
public:
    // Initialize the combat system
    static void initialize();
    
    // Resolves combat between pieces at the attacker and defender positions
    static bool resolveCombat(GameBoard& board, const Position& attacker, const Position& defender);
    
    // Applies damage from attacker to defender
    static void applyDamage(Piece* attacker, Piece* defender);
    
    // Checks entire board for dead pieces and removes them
    static void checkAndRemoveDeadPieces(GameBoard& board);
    
    // Checks if a specific square has a dead piece and removes it if necessary
    static bool checkAndRemoveDeadPiece(GameBoard& board, const Position& position);
    
    // Determines if two pieces can engage in combat
    static bool canEngageInCombat(const GameBoard& board, const Position& attacker, const Position& defender);
    
    // Checks for defeated kings and determines the winning side
    static bool checkForDefeatedKings(const GameBoard& board, PlayerSide& winningSide);
};

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_COMBAT_SYSTEM_H
