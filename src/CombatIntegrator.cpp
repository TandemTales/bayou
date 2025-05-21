#include "../include/CombatIntegrator.h"
#include "../include/PieceRemovalHandler.h"
#include "../include/CombatCalculator.h"
#include "../include/HealthTracker.h"
#include "../include/GameBoard.h"
#include "../include/GameState.h"
#include "../include/Move.h"

namespace BayouBonanza {

// Initialize static members
PreCombatCallback CombatIntegrator::preCombatCallback = nullptr;
PostCombatCallback CombatIntegrator::postCombatCallback = nullptr;
GameOverCallback CombatIntegrator::gameOverCallback = nullptr;

void CombatIntegrator::initialize(GameState& gameState) {
    // Initialize the combat system
    CombatSystem::initialize();
    
    // Set up event handling between combat system and game state
    HealthTracker::registerEventCallback([&gameState](std::shared_ptr<Piece> piece, HealthEvent event, int value) {
        // Update game state based on health events
        if (event == HealthEvent::DEFEATED) {
            // Update player statistics, trigger visual effects, etc.
            // This will depend on how the GameState class is implemented
        }
    });
    
    PieceRemovalHandler::registerEventCallback([&gameState](const Position& pos, std::shared_ptr<Piece> piece, RemovalEvent event) {
        // Update game state based on removal events
        if (event == RemovalEvent::KING_DEFEATED) {
            // Handle king defeat - game over condition
            PlayerSide winningSide = (piece->getSide() == PlayerSide::PLAYER_ONE) ? 
                                     PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
                                     
            // Notify game state about game over condition
            // This will depend on how the GameState class is implemented
            
            // Call the game over callback if registered
            if (gameOverCallback) {
                gameOverCallback(gameState.getBoard(), winningSide);
            }
        }
    });
}

void CombatIntegrator::registerPreCombatCallback(PreCombatCallback callback) {
    preCombatCallback = callback;
}

void CombatIntegrator::registerPostCombatCallback(PostCombatCallback callback) {
    postCombatCallback = callback;
}

void CombatIntegrator::registerGameOverCallback(GameOverCallback callback) {
    gameOverCallback = callback;
}

bool CombatIntegrator::handleCombatOnMove(GameBoard& board, Move& move) {
    const Position& to = move.getTo();
    const Position& from = move.getFrom();
    
    // Check if the destination has an enemy piece
    if (!board.isValidPosition(to.x, to.y)) {
        return false;
    }
    
    auto& targetSquare = board.getSquare(to.x, to.y);
    
    if (targetSquare.isEmpty()) {
        // No combat if the target square is empty
        return false;
    }
    
    auto movingPiece = move.getPiece();
    auto targetPiece = targetSquare.getPiece();
    
    // Verify the pieces belong to different players
    if (!movingPiece || !targetPiece || movingPiece->getSide() == targetPiece->getSide()) {
        return false; // Cannot combat own pieces or if pieces are null
    }
    
    // Notify before combat occurs
    if (preCombatCallback) {
        preCombatCallback(board, from, to);
    }
    
    // Resolve combat
    bool success = CombatSystem::resolveCombat(board, from, to);
    
    // Update the board after combat
    updateBoardPostCombat(board);
    
    // Check for game over
    PlayerSide winningSide;
    bool gameOver = checkGameOver(board, winningSide);
    
    // Notify after combat is complete
    if (postCombatCallback) {
        postCombatCallback(board, from, to, success);
    }
    
    return success;
}

bool CombatIntegrator::handleDirectCombat(GameBoard& board, const Position& attacker, const Position& defender) {
    // Verify that both positions are valid
    if (!board.isValidPosition(attacker.x, attacker.y) || !board.isValidPosition(defender.x, defender.y)) {
        return false;
    }
    
    // Check if pieces can engage in combat
    if (!CombatSystem::canEngageInCombat(board, attacker, defender)) {
        return false;
    }
    
    // Notify before combat occurs
    if (preCombatCallback) {
        preCombatCallback(board, attacker, defender);
    }
    
    // Resolve combat
    // One-way combat only - attacker damages defender with no counter-attacks
    bool success = CombatSystem::resolveCombat(board, attacker, defender);
    
    // Update the board after combat
    updateBoardPostCombat(board);
    
    // Check for game over
    PlayerSide winningSide;
    bool gameOver = checkGameOver(board, winningSide);
    
    // Notify after combat is complete
    if (postCombatCallback) {
        postCombatCallback(board, attacker, defender, success);
    }
    
    return success;
}

bool CombatIntegrator::processTurnEndCombatEffects(GameBoard& board, PlayerSide currentPlayer) {
    // Handle any end-of-turn combat effects like poison, etc.
    
    // Remove any defeated pieces that might have been damaged during the turn
    CombatSystem::checkAndRemoveDeadPieces(board);
    
    // Update influence areas and control values
    updateBoardPostCombat(board);
    
    // Check for game over
    PlayerSide winningSide;
    return checkGameOver(board, winningSide);
}

void CombatIntegrator::updateBoardPostCombat(GameBoard& board) {
    // Recalculate influence areas and control values
    board.recalculateControlValues();
}

bool CombatIntegrator::checkGameOver(GameBoard& board, PlayerSide& winningSide) {
    // Check for defeated kings
    bool gameOver = CombatSystem::checkForDefeatedKings(board, winningSide);
    
    if (gameOver && gameOverCallback) {
        gameOverCallback(board, winningSide);
    }
    
    return gameOver;
}

} // namespace BayouBonanza
