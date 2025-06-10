#pragma once

#include <memory>
#include <functional>
#include "GameState.h"
#include "GameRules.h"
#include "GameOverDetector.h"
#include "Move.h"

namespace BayouBonanza {

/**
 * @brief Type of action a player can take during their turn
 */
enum class ActionType {
    MOVE_PIECE,    // Move a piece on the board
    PLAY_CARD,     // Play a card from hand
    END_TURN       // End the current turn
};

/**
 * @brief Result of an action performed by the TurnManager
 */
struct ActionResult {
    bool success;
    std::string message;
    
    ActionResult() : success(false), message("") {}
    ActionResult(bool s, const std::string& m) : success(s), message(m) {}
};

/**
 * @brief Callback function type for action results
 */
using ActionCallback = std::function<void(const ActionResult&)>;

/**
 * @brief Manages player turns and game flow
 */
class TurnManager {
public:
    /**
     * @brief Constructor
     * 
     * @param gameState Reference to the game state
     * @param gameRules Reference to the game rules
     */
    TurnManager(GameState& gameState, GameRules& gameRules);
    
    /**
     * @brief Start a new game
     */
    void startNewGame();
    
    /**
     * @brief Process a move piece action
     * 
     * @param move The move to execute
     * @param callback Callback to receive the result
     */
    void processMoveAction(const Move& move, ActionCallback callback = nullptr);
    
    /**
     * @brief Process a play card action
     * 
     * @param cardIndex Index of the card in the player's hand
     * @param position Position to place the new piece
     * @param callback Callback to receive the result
     */
    void processPlayCardAction(int cardIndex, const Position& position, ActionCallback callback = nullptr);
    
    /**
     * @brief End the current player's turn
     * 
     * @param callback Callback to receive the result
     */
    void endCurrentTurn(ActionCallback callback = nullptr);
    
    /**
     * @brief Advance to the next phase
     * 
     * @param callback Callback to receive the result
     */
    void nextPhase(ActionCallback callback = nullptr);
    
    /**
     * @brief Check if the game is over
     * 
     * @return true if the game is over
     */
    bool isGameOver() const;
    
    /**
     * @brief Get the current game state
     * 
     * @return Reference to the game state
     */
    const GameState& getGameState() const;
    
    /**
     * @brief Get the current active player
     * 
     * @return The active player side
     */
    PlayerSide getActivePlayer() const;
    
    /**
     * @brief Get the current turn number
     * 
     * @return The turn number
     */
    int getTurnNumber() const;

    /**
     * @brief Get detailed win condition information
     * @return String describing the current win condition or game status
     */
    std::string getWinConditionDescription() const;

private:
    GameState& gameState;
    GameRules gameRules;
    GameOverDetector gameOverDetector;
    
    /**
     * @brief Update game state after an action
     * 
     * @param actionResult The result of the action
     * @return true if the game continues
     */
    bool updateGameState(const ActionResult& actionResult);
    
    /**
     * @brief Check if it's game over and update result
     * 
     * @return true if the game is over
     */
    bool checkGameOver();
};

} // namespace BayouBonanza
