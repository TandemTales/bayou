#pragma once

#include "GameState.h"
#include "PlayerSide.h"
#include <functional>

namespace BayouBonanza {

/**
 * @brief Callback type for win condition notifications
 */
using WinConditionCallback = std::function<void(PlayerSide winner, const std::string& description)>;

/**
 * @brief Detects game over conditions and determines winners
 */
class GameOverDetector {
public:
    /**
     * @brief Default constructor
     */
    GameOverDetector() = default;
    
    /**
     * @brief Register a callback for win condition notifications
     * 
     * @param callback Function to call when a win condition is detected
     */
    static void registerWinConditionCallback(WinConditionCallback callback);
    
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
     * @return The winning player, or NEUTRAL if no winner/draw
     */
    PlayerSide getWinner(const GameState& gameState) const;
    
    /**
     * @brief Check for game over conditions and update game state if needed
     * 
     * @param gameState Game state to check and potentially update
     * @return true if the game is over
     */
    bool checkAndUpdateGameOver(GameState& gameState);

    /**
     * @brief Get detailed win condition information
     * 
     * @param gameState Current game state
     * @return String describing the win condition or game status
     */
    std::string getWinConditionDescription(const GameState& gameState) const;

    /**
     * @brief Check if a specific player has any victory pieces remaining
     * 
     * @param gameState Current game state
     * @param side Player side to check
     * @return true if the player has at least one victory piece (king)
     */
    bool hasVictoryPieces(const GameState& gameState, PlayerSide side) const;

private:
    static WinConditionCallback winConditionCallback;
    
    /**
     * @brief Check if a player has a king on the board
     * 
     * @param gameState Current game state
     * @param side Player side to check
     * @return true if the player has a king
     */
    bool hasKing(const GameState& gameState, PlayerSide side) const;
    
    /**
     * @brief Fire win condition notification
     * 
     * @param winner The winning player
     * @param description Description of the win condition
     */
    static void fireWinConditionNotification(PlayerSide winner, const std::string& description);
};

} // namespace BayouBonanza
