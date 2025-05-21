#ifndef BAYOU_BONANZA_COMBAT_INTEGRATOR_H
#define BAYOU_BONANZA_COMBAT_INTEGRATOR_H

#include "GameBoard.h"
#include "GameState.h"
#include "CombatSystem.h"
#include "Move.h"
#include <functional>

namespace BayouBonanza {

// Callback types for combat events
using PreCombatCallback = std::function<void(const GameBoard&, const Position&, const Position&)>;
using PostCombatCallback = std::function<void(const GameBoard&, const Position&, const Position&, bool)>;
using GameOverCallback = std::function<void(const GameBoard&, PlayerSide)>;

class CombatIntegrator {
public:
    // Initialize the combat integrator with the game systems
    static void initialize(GameState& gameState);
    
    // Register callbacks for combat events
    static void registerPreCombatCallback(PreCombatCallback callback);
    static void registerPostCombatCallback(PostCombatCallback callback);
    static void registerGameOverCallback(GameOverCallback callback);
    
    // Integrate combat with movement - called when a piece moves to a position with an enemy piece
    static bool handleCombatOnMove(GameBoard& board, Move& move);
    
    // Handle direct combat initiated by player action (not movement)
    static bool handleDirectCombat(GameBoard& board, const Position& attacker, const Position& defender);
    
    // Process turn end and check for combat-related game over conditions
    static bool processTurnEndCombatEffects(GameBoard& board, PlayerSide currentPlayer);
    
    // Update influence areas and control values after combat
    static void updateBoardPostCombat(GameBoard& board);

private:
    static PreCombatCallback preCombatCallback;
    static PostCombatCallback postCombatCallback;
    static GameOverCallback gameOverCallback;
    
    // Check for game over conditions (king defeated)
    static bool checkGameOver(GameBoard& board, PlayerSide& winningSide);
};

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_COMBAT_INTEGRATOR_H
