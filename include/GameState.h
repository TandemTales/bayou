#pragma once

#include <memory>
#include "GameBoard.h" // Includes Square.h, Piece.h, etc.
#include "PlayerSide.h"
#include "ResourceSystem.h" // Added ResourceSystem include
#include "CardCollection.h" // Added CardCollection include (includes Deck and Hand)
#include "PieceData.h" // Include PieceData.h for Position struct
#include <SFML/Network/Packet.hpp> // For sf::Packet

// Forward declarations to avoid circular dependencies
namespace BayouBonanza {
    struct ValidationResult;
    struct PlayResult;
    enum class ActionType; // Forward declaration for ActionType from TurnManager
}

// GameBoard.h brings in Square.h which includes Piece.h and PlayerSide.h.

namespace BayouBonanza {

/**
 * @brief Enum representing different phases of the game
 */
enum class GamePhase {
    SETUP,      // Initial setup phase
    DRAW,       // Draw phase - player draws cards
    PLAY,       // Play phase - player can play cards
    MOVE,       // Move phase - player can move pieces
    GAME_OVER   // Game has ended
};

// SFML Packet operators for GamePhase
sf::Packet& operator<<(sf::Packet& packet, const GamePhase& phase);
sf::Packet& operator>>(sf::Packet& packet, GamePhase& phase);

/**
 * @brief Enum representing the result of the game
 */
enum class GameResult {
    IN_PROGRESS,     // Game is still ongoing
    PLAYER_ONE_WIN,  // Player one has won
    PLAYER_TWO_WIN,  // Player two has won
    DRAW            // Game ended in a draw
};

// SFML Packet operators for GameResult
sf::Packet& operator<<(sf::Packet& packet, const GameResult& result);
sf::Packet& operator>>(sf::Packet& packet, GameResult& result);

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
     * @brief Set the active player (primarily for deserialization)
     * @param player The player to set as active
     */
    void setActivePlayer(PlayerSide player);
    
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
     * @brief Advance to the next phase in the turn sequence
     * 
     * Progresses from DRAW -> PLAY -> MOVE -> (next player's DRAW)
     */
    void nextPhase();
    
    /**
     * @brief Check if a specific action is allowed in the current phase
     * 
     * @param actionType The type of action to check
     * @return true if the action is allowed in the current phase
     */
    bool isActionAllowedInPhase(ActionType actionType) const;
    
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
     * @brief Set the turn number (primarily for deserialization)
     * @param turn The turn number to set
     */
    void setTurnNumber(int turn);
    
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
    
    /**
     * @brief Spend steam for a player
     * 
     * @param side The player side
     * @param amount The amount of steam to spend
     * @return true if successful (player had enough steam), false otherwise
     */
    bool spendSteam(PlayerSide side, int amount);
    
    /**
     * @brief Get the ResourceSystem for advanced steam management
     * 
     * @return Reference to the ResourceSystem
     */
    ResourceSystem& getResourceSystem();
    
    /**
     * @brief Get the ResourceSystem for advanced steam management (const version)
     * 
     * @return Const reference to the ResourceSystem
     */
    const ResourceSystem& getResourceSystem() const;
    
    /**
     * @brief Process turn start - calculate and add steam generation
     * 
     * This should be called at the beginning of each player's turn.
     */
    void processTurnStart();

    // Card System Integration
    
    /**
     * @brief Get a player's deck
     * 
     * @param side The player side
     * @return Reference to the player's deck
     */
    Deck& getDeck(PlayerSide side);
    
    /**
     * @brief Get a player's deck (const version)
     * 
     * @param side The player side
     * @return Const reference to the player's deck
     */
    const Deck& getDeck(PlayerSide side) const;
    
    /**
     * @brief Get a player's hand
     * 
     * @param side The player side
     * @return Reference to the player's hand
     */
    Hand& getHand(PlayerSide side);
    
    /**
     * @brief Get a player's hand (const version)
     * 
     * @param side The player side
     * @return Const reference to the player's hand
     */
    const Hand& getHand(PlayerSide side) const;
    
    /**
     * @brief Draw a card from deck to hand for a player
     * 
     * @param side The player side
     * @return true if a card was drawn, false if deck is empty or hand is full
     */
    bool drawCard(PlayerSide side);
    
    /**
     * @brief Play a card from a player's hand
     * 
     * @param side The player side
     * @param handIndex The index of the card in the hand
     * @param targetPosition Optional target position for targeted cards
     * @return true if the card was played successfully, false otherwise
     */
    bool playCard(PlayerSide side, size_t handIndex, const Position& targetPosition = {-1, -1});
    
    /**
     * @brief Play a card with detailed result information
     * 
     * @param side The player side
     * @param handIndex The index of the card in the hand
     * @param targetPosition Optional target position for targeted cards
     * @return PlayResult containing detailed execution status and error information
     */
    PlayResult playCardWithResult(PlayerSide side, size_t handIndex, const Position& targetPosition = {-1, -1});
    
    /**
     * @brief Validate if a card can be played without actually playing it
     * 
     * @param side The player side
     * @param handIndex The index of the card in the hand
     * @param targetPosition Optional target position for targeted cards
     * @return ValidationResult containing validation status and error details
     */
    ValidationResult validateCardPlay(PlayerSide side, size_t handIndex, const Position& targetPosition = {-1, -1}) const;
    
    /**
     * @brief Initialize card system for both players
     * 
     * Creates starter decks and draws initial hands.
     */
    void initializeCardSystem();

    /**
     * @brief Initialize card system with custom decks
     */
    void initializeCardSystem(const Deck& deck1, const Deck& deck2);
    
    /**
     * @brief Process card-related turn start events
     * 
     * Draws cards and handles turn-based card effects.
     */
    void processCardTurnStart();

private:
    GameBoard board;
    PlayerSide activePlayer;
    GamePhase phase;
    GameResult result;
    int turnNumber;
    ResourceSystem resourceSystem; // Added ResourceSystem member
    
    // Legacy steam tracking (kept for backward compatibility)
    int steamPlayer1;
    int steamPlayer2;
    
    // Card System
    Deck deckPlayer1;
    Deck deckPlayer2;
    Hand handPlayer1;
    Hand handPlayer2;
};

// SFML Packet operators for GameState
sf::Packet& operator<<(sf::Packet& packet, const GameState& gs);
sf::Packet& operator>>(sf::Packet& packet, GameState& gs);

} // namespace BayouBonanza
