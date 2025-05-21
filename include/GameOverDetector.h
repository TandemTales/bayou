#pragma once

#include "GameState.h"
#include "PlayerSide.h"

namespace BayouBonanza {

/**
 * @brief Responsible for detecting game over conditions
 * 
 * This class handles the logic to determine if the game has ended
 * and which player has won.
 */
class GameOverDetector {
public:
    /**
     * @brief Default constructor
     */
    GameOverDetector() = default;
    
    /**
     * @brief Check if the game is over
     * 
     * @param gameState Current game state
     * @return true if the game is over
     */
    bool isGameOver(const GameState& gameState) const;
    
    /**
     * @brief Get the winner of the game
     * 
     * @param gameState Current game state
     * @return The winning player side, or NEUTRAL if no winner
     */
    PlayerSide getWinner(const GameState& gameState) const;
    
    /**
     * @brief Update the game state with game over information
     * 
     * Checks if the game is over and updates the game state accordingly.
     * 
     * @param gameState Game state to update
     * @return true if the game is now over
     */
    bool checkAndUpdateGameOver(GameState& gameState);

private:
    /**
     * @brief Check if a king is still on the board
     * 
     * @param gameState Current game state
     * @param side Player side to check
     * @return true if the king is still alive
     */
    bool hasKing(const GameState& gameState, PlayerSide side) const;
};

} // namespace BayouBonanza
