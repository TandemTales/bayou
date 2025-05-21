#pragma once

#include <memory>
#include "GameBoard.h"
#include "PlayerSide.h"

namespace BayouBonanza {

/**
 * @brief Enum representing different phases of the game
 */
enum class GamePhase {
    SETUP,      // Initial setup phase
    MAIN_GAME,  // Main gameplay phase
    GAME_OVER   // Game has ended
};

/**
 * @brief Enum representing the result of the game
 */
enum class GameResult {
    IN_PROGRESS,     // Game is still ongoing
    PLAYER_ONE_WIN,  // Player one has won
    PLAYER_TWO_WIN,  // Player two has won
    DRAW            // Game ended in a draw
};

/**
 * @brief Manages the state of the game, including board, active player, and game phase
 * 
 * This class is responsible for tracking all aspects of the game state and
 * provides methods for state transitions and game management.
 */
class GameState {
public:
    /**
     * @brief Default constructor, initializes a new game state
     */
    GameState();
    
    /**
     * @brief Get the current game board
     * 
     * @return Reference to the game board
     */
    GameBoard& getBoard();
    
    /**
     * @brief Get the current game board (const version)
     * 
     * @return Const reference to the game board
     */
    const GameBoard& getBoard() const;
    
    /**
     * @brief Get the current active player
     * 
     * @return The player side whose turn it is
     */
    PlayerSide getActivePlayer() const;
    
    /**
     * @brief Switch to the next player's turn
     */
    void switchActivePlayer();
    
    /**
     * @brief Get the current game phase
     * 
     * @return The current phase of the game
     */
    GamePhase getGamePhase() const;
    
    /**
     * @brief Set the game phase
     * 
     * @param phase The new game phase
     */
    void setGamePhase(GamePhase phase);
    
    /**
     * @brief Get the current game result
     * 
     * @return The result of the game
     */
    GameResult getGameResult() const;
    
    /**
     * @brief Set the game result
     * 
     * @param result The result of the game
     */
    void setGameResult(GameResult result);
    
    /**
     * @brief Initialize a new game
     * 
     * Resets the game board and sets up initial game state.
     */
    void initializeNewGame();

    /**
     * @brief Get the current turn number
     * 
     * @return The current turn number (starts at 1)
     */
    int getTurnNumber() const;
    
    /**
     * @brief Increment the turn number
     */
    void incrementTurnNumber();
    
    /**
     * @brief Get the amount of steam for a player
     * 
     * @param side The player side
     * @return The amount of steam
     */
    int getSteam(PlayerSide side) const;
    
    /**
     * @brief Set the amount of steam for a player
     * 
     * @param side The player side
     * @param amount The new amount of steam
     */
    void setSteam(PlayerSide side, int amount);
    
    /**
     * @brief Add steam to a player's total
     * 
     * @param side The player side
     * @param amount The amount of steam to add
     */
    void addSteam(PlayerSide side, int amount);

private:
    GameBoard board;
    PlayerSide activePlayer;
    GamePhase phase;
    GameResult result;
    int turnNumber;
    int steamPlayer1;
    int steamPlayer2;
};

} // namespace BayouBonanza
