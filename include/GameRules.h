#pragma once

#include <memory>
#include "GameState.h"
#include "Move.h"
#include "MoveExecutor.h"

namespace BayouBonanza {

/**
 * @brief Encapsulates the game rules and logic
 * 
 * This class is responsible for enforcing game rules, checking win conditions,
 * and managing game flow.
 */
class GameRules {
public:
    /**
     * @brief Default constructor
     */
    GameRules();
    
    /**
     * @brief Initialize a new game
     * 
     * Sets up the game board with initial pieces and state.
     * 
     * @param gameState The game state to initialize
     */
    void initializeGame(GameState& gameState);
    
    /**
     * @brief Process a player's move
     * 
     * Validates and executes a move, updating the game state accordingly.
     * 
     * @param gameState Current game state
     * @param move The move to process
     * @return Result of the move execution
     */
    MoveResult processMove(GameState& gameState, const Move& move);
    
    /**
     * @brief Check if the game is over
     * 
     * @param gameState Current game state
     * @return true if the game is over
     */
    bool isGameOver(const GameState& gameState) const;
    
    /**
     * @brief Process end of turn actions
     * 
     * This includes switching active player, incrementing turn number,
     * and applying any end-of-turn effects.
     * 
     * @param gameState Game state to update
     */
    void endTurn(GameState& gameState);
    
    /**
     * @brief Get all valid moves for the active player
     * 
     * @param gameState Current game state
     * @return Vector of valid moves
     */
    std::vector<Move> getValidMovesForActivePlayer(const GameState& gameState) const;
    
    /**
     * @brief Check if a player has won the game
     * 
     * @param gameState Current game state
     * @param side Player side to check
     * @return true if the specified player has won
     */
    bool hasPlayerWon(const GameState& gameState, PlayerSide side) const;

private:
    MoveExecutor moveExecutor;
    
    /**
     * @brief Check if a player's king is still on the board
     * 
     * @param gameState Current game state
     * @param side Player side to check
     * @return true if the player's king is alive
     */
    bool hasKing(const GameState& gameState, PlayerSide side) const;
};

} // namespace BayouBonanza
