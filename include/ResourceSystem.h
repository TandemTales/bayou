#pragma once
#include "PlayerSide.h"

namespace BayouBonanza {

// Forward declarations
class GameBoard;

/**
 * @brief Manages steam resources for both players
 * 
 * The ResourceSystem tracks and manages steam generation based on controlled squares,
 * handles steam accumulation per turn, and provides methods for spending steam resources.
 */
class ResourceSystem {
private:
    int player1Steam;
    int player2Steam;
    
    // Store generation values for debugging/UI purposes
    int lastPlayer1Generation;
    int lastPlayer2Generation;
    
public:
    /**
     * @brief Constructor - initializes both players with starting steam
     * @param startingSteam Initial steam amount for both players (default: 0)
     */
    explicit ResourceSystem(int startingSteam = 0);
    
    /**
     * @brief Get current steam amount for a player
     * @param player The player to query
     * @return Current steam amount
     */
    int getSteam(PlayerSide player) const;
    
    /**
     * @brief Set steam amount for a player
     * @param player The player to set steam for
     * @param amount The amount to set (cannot be negative)
     */
    void setSteam(PlayerSide player, int amount);
    
    /**
     * @brief Add steam to a player's total
     * @param player The player to add steam to
     * @param amount The amount to add (cannot be negative)
     */
    void addSteam(PlayerSide player, int amount);
    
    /**
     * @brief Attempt to spend steam for a player
     * @param player The player spending steam
     * @param amount The amount to spend
     * @return true if successful (player had enough steam), false otherwise
     */
    bool spendSteam(PlayerSide player, int amount);
    
    /**
     * @brief Calculate steam generation based on controlled squares
     * @param board The game board to analyze
     * @return Pair of (player1Generation, player2Generation)
     */
    std::pair<int, int> calculateSteamGeneration(const GameBoard& board);
    
    /**
     * @brief Process turn start - add generated steam to active player
     * @param activePlayer The player starting their turn
     * @param board The current game board
     */
    void processTurnStart(PlayerSide activePlayer, const GameBoard& board);
    
    /**
     * @brief Get the last calculated generation values for debugging/UI
     * @return Pair of (player1Generation, player2Generation)
     */
    std::pair<int, int> getLastGenerationValues() const;
    
    /**
     * @brief Reset both players' steam to starting values
     * @param startingSteam Starting steam amount (default: 0)
     */
    void reset(int startingSteam = 0);
};

} // namespace BayouBonanza 