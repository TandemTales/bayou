#pragma once

#include <vector>
#include <string>
#include <memory>
#include "Card.h"
#include "PieceCard.h"
#include "EffectCard.h"
#include "GameState.h"
#include "PlayerSide.h"
#include "PieceData.h"

namespace BayouBonanza {

/**
 * @brief Enum representing different validation error types
 */
enum class ValidationError {
    NONE,                    // No error
    INSUFFICIENT_STEAM,      // Player doesn't have enough steam
    INVALID_HAND_INDEX,      // Hand index is out of bounds
    CARD_NOT_FOUND,         // Card doesn't exist at specified index
    NO_VALID_TARGETS,       // No valid targets available for the card
    INVALID_TARGET,         // Specified target is not valid
    INVALID_PLACEMENT,      // Piece placement position is invalid
    GAME_STATE_INVALID,     // Game state doesn't allow card play
    CARD_CANNOT_BE_PLAYED,  // Card's canPlay() method returned false
    UNKNOWN_CARD_TYPE       // Card type is not recognized
};

/**
 * @brief Structure containing validation result information
 */
struct ValidationResult {
    bool isValid;
    ValidationError error;
    std::string errorMessage;
    
    ValidationResult(bool valid = true, ValidationError err = ValidationError::NONE, 
                    const std::string& message = "")
        : isValid(valid), error(err), errorMessage(message) {}
};

/**
 * @brief Structure containing card play execution result
 */
struct PlayResult {
    bool success;
    ValidationError error;
    std::string errorMessage;
    bool steamSpent;        // Whether steam was deducted
    bool cardRemoved;       // Whether card was removed from hand
    
    PlayResult(bool succeeded = false, ValidationError err = ValidationError::NONE,
               const std::string& message = "", bool steamDeducted = false, bool cardTaken = false)
        : success(succeeded), error(err), errorMessage(message), 
          steamSpent(steamDeducted), cardRemoved(cardTaken) {}
};

/**
 * @brief Comprehensive validator for card play mechanics
 * 
 * This class provides validation and execution systems for card play,
 * including pre-play validation, target validation, and rollback mechanisms.
 */
class CardPlayValidator {
public:
    /**
     * @brief Validate if a card can be played from a player's hand
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @param handIndex The index of the card in the player's hand
     * @return ValidationResult containing validation status and error details
     */
    static ValidationResult validateCardPlay(const GameState& gameState, PlayerSide player, size_t handIndex);
    
    /**
     * @brief Validate if a card can be played at a specific target position
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @param handIndex The index of the card in the player's hand
     * @param targetPosition The target position for the card
     * @return ValidationResult containing validation status and error details
     */
    static ValidationResult validateTargetedCardPlay(const GameState& gameState, PlayerSide player, 
                                                    size_t handIndex, const Position& targetPosition);
    
    /**
     * @brief Validate placement for a piece card
     * 
     * @param gameState The current game state
     * @param player The player attempting to place the piece
     * @param pieceCard The piece card being played
     * @param position The position where the piece will be placed
     * @return ValidationResult containing validation status and error details
     */
    static ValidationResult validatePiecePlacement(const GameState& gameState, PlayerSide player,
                                                  const PieceCard* pieceCard, const Position& position);
    
    /**
     * @brief Validate targeting for an effect card
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the effect
     * @param effectCard The effect card being played
     * @param position The target position for the effect
     * @return ValidationResult containing validation status and error details
     */
    static ValidationResult validateEffectTarget(const GameState& gameState, PlayerSide player,
                                                const EffectCard* effectCard, const Position& position);
    
    /**
     * @brief Get all valid placement positions for a piece card
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @param pieceCard The piece card to check
     * @return Vector of valid positions where the piece can be placed
     */
    static std::vector<Position> getValidPlacements(const GameState& gameState, PlayerSide player,
                                                   const PieceCard* pieceCard);
    
    /**
     * @brief Get all valid target positions for an effect card
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @param effectCard The effect card to check
     * @return Vector of valid positions that can be targeted
     */
    static std::vector<Position> getValidTargets(const GameState& gameState, PlayerSide player,
                                                const EffectCard* effectCard);
    
    /**
     * @brief Execute a card play with comprehensive validation and rollback
     * 
     * This method performs the complete card play pipeline:
     * 1. Validates the card play
     * 2. Deducts steam cost
     * 3. Removes card from hand
     * 4. Executes card effect
     * 5. Handles rollback on failure
     * 
     * @param gameState The game state to modify
     * @param player The player playing the card
     * @param handIndex The index of the card in the hand
     * @param targetPosition Optional target position for targeted cards
     * @return PlayResult containing execution status and details
     */
    static PlayResult executeCardPlay(GameState& gameState, PlayerSide player, size_t handIndex,
                                     const Position& targetPosition = {-1, -1});
    
    /**
     * @brief Execute a targeted card play with validation
     * 
     * @param gameState The game state to modify
     * @param player The player playing the card
     * @param handIndex The index of the card in the hand
     * @param targetPosition The target position for the card
     * @return PlayResult containing execution status and details
     */
    static PlayResult executeTargetedCardPlay(GameState& gameState, PlayerSide player, 
                                             size_t handIndex, const Position& targetPosition);
    
    /**
     * @brief Check if the game state allows card play
     * 
     * @param gameState The current game state
     * @param player The player attempting to play a card
     * @return ValidationResult indicating if card play is allowed
     */
    static ValidationResult validateGameState(const GameState& gameState, PlayerSide player);
    
    /**
     * @brief Get a human-readable error message for a validation error
     * 
     * @param error The validation error type
     * @return String description of the error
     */
    static std::string getErrorMessage(ValidationError error);
    
    /**
     * @brief Check if a position is within board bounds
     * 
     * @param position The position to check
     * @return true if the position is valid, false otherwise
     */
    static bool isValidBoardPosition(const Position& position);

private:
    /**
     * @brief Rollback a failed card play attempt
     * 
     * @param gameState The game state to rollback
     * @param player The player whose action failed
     * @param card The card to return to hand
     * @param steamCost The steam cost to refund
     * @param handIndex The original hand index to restore
     */
    static void rollbackCardPlay(GameState& gameState, PlayerSide player, 
                               std::unique_ptr<Card> card, int steamCost, size_t handIndex);
};

} // namespace BayouBonanza 