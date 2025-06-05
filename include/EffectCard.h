#pragma once

#include "Card.h"
#include "PieceData.h"

namespace BayouBonanza {

/**
 * @brief Enum representing different target types for effect cards
 */
enum class TargetType {
    SINGLE_PIECE,       // Target a single piece
    ALL_FRIENDLY,       // Target all friendly pieces
    ALL_ENEMY,          // Target all enemy pieces
    ALL_PIECES,         // Target all pieces on the board
    BOARD_AREA,         // Target a specific area of the board
    SELF_PLAYER,        // Target the player themselves (for resource effects)
    ENEMY_PLAYER        // Target the enemy player
};

/**
 * @brief Structure representing an effect that can be applied
 */
struct Effect {
    EffectType type;
    int magnitude;          // Strength of the effect
    int duration;           // Duration in turns (0 = instant, -1 = permanent)
    TargetType targetType;
    
    Effect(EffectType t, int mag, int dur = 0, TargetType target = TargetType::SINGLE_PIECE)
        : type(t), magnitude(mag), duration(dur), targetType(target) {}
};

/**
 * @brief Card that applies effects to pieces or the game state
 * 
 * EffectCards allow players to modify pieces, apply buffs/debuffs,
 * heal damage, or create other temporary or permanent effects.
 */
class EffectCard : public Card {
public:
    /**
     * @brief Constructor
     * 
     * @param id Unique identifier for this card
     * @param name Display name of the card
     * @param description Text description of the card's effect
     * @param steamCost Amount of steam required to play this card
     * @param effect The effect this card applies
     * @param rarity The rarity level of this card
     */
    EffectCard(int id, const std::string& name, const std::string& description,
               int steamCost, const Effect& effect, CardRarity rarity = CardRarity::COMMON);
    
    /**
     * @brief Get the effect this card applies
     * 
     * @return The effect structure
     */
    const Effect& getEffect() const;
    
    /**
     * @brief Check if this card can be played in the current game state
     * 
     * Validates that:
     * - Player has enough steam
     * - There are valid targets for the effect
     * - Game state allows the effect
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @return true if the card can be played, false otherwise
     */
    bool canPlay(const GameState& gameState, PlayerSide player) const override;
    
    /**
     * @brief Execute the card's effect
     * 
     * This method will:
     * - Deduct steam cost from player
     * - Apply the effect to appropriate targets
     * - Handle both instant and ongoing effects
     * 
     * @param gameState The game state to modify
     * @param player The player playing the card
     * @return true if the card was played successfully, false otherwise
     */
    bool play(GameState& gameState, PlayerSide player) const override;
    
    /**
     * @brief Check if a specific piece is a valid target for this effect
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @param position The position of the piece to check
     * @return true if the piece is a valid target
     */
    bool isValidTarget(const GameState& gameState, PlayerSide player, const Position& position) const;
    
    /**
     * @brief Get all valid target positions for this effect
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @return Vector of valid positions that can be targeted
     */
    std::vector<Position> getValidTargets(const GameState& gameState, PlayerSide player) const;
    
    /**
     * @brief Play the card targeting a specific position
     * 
     * @param gameState The game state to modify
     * @param player The player playing the card
     * @param position The position to target (if applicable)
     * @return true if the card was played successfully, false otherwise
     */
    bool playAtTarget(GameState& gameState, PlayerSide player, const Position& position) const;
    
    /**
     * @brief Apply the effect to a specific piece
     * 
     * @param piece The piece to apply the effect to
     * @param player The player playing the card
     * @return true if the effect was applied successfully
     */
    bool applyEffectToPiece(Piece* piece, PlayerSide player) const;
    
    /**
     * @brief Apply the effect to the player (for resource effects)
     * 
     * @param gameState The game state to modify
     * @param targetPlayer The player to apply the effect to
     * @param castingPlayer The player casting the effect
     * @return true if the effect was applied successfully
     */
    bool applyEffectToPlayer(GameState& gameState, PlayerSide targetPlayer, PlayerSide castingPlayer) const;
    
    /**
     * @brief Get a detailed description of what this card does
     * 
     * @return Extended description including effect details
     */
    std::string getDetailedDescription() const override;
    
    /**
     * @brief Create a copy of this card
     * 
     * @return A unique pointer to a copy of this card
     */
    std::unique_ptr<Card> clone() const override;

private:
    Effect effect;
    
    /**
     * @brief Check if the effect can target friendly pieces
     * 
     * @return true if the effect can target friendly pieces
     */
    bool canTargetFriendly() const;
    
    /**
     * @brief Check if the effect can target enemy pieces
     * 
     * @return true if the effect can target enemy pieces
     */
    bool canTargetEnemy() const;
    
    /**
     * @brief Get the effect type name as a string
     * 
     * @return String representation of the effect type
     */
    std::string getEffectTypeName() const;
};

} // namespace BayouBonanza 