#ifndef BAYOU_BONANZA_COMBAT_SYSTEM_H
#define BAYOU_BONANZA_COMBAT_SYSTEM_H

#include "GameBoard.h"
#include "Position.h"
#include "Piece.h"
#include "PieceRemovalHandler.h"
#include "HealthTracker.h"

namespace BayouBonanza {

class CombatSystem {
public:
    // Constructor that takes callbacks for combat events
    CombatSystem();
    
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
    
    // Initialize combat-related systems and register callbacks
    static void initialize();
    
    // Check if a king has been defeated, indicating game over
    static bool checkForDefeatedKings(const GameBoard& board, PlayerSide& winningSide);
};

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_COMBAT_SYSTEM_H
