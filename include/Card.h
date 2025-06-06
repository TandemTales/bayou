#pragma once

#include <string>
#include <memory>
#include <vector>
#include "PlayerSide.h"
#include <SFML/Network/Packet.hpp>

namespace BayouBonanza {

// Forward declarations
class GameState;
class GameBoard;

/**
 * @brief Enum representing different card types
 */
enum class CardType {
    PIECE_CARD,     // Cards that spawn pieces on the board
    EFFECT_CARD,    // Cards that apply effects to pieces or board
    SPELL_CARD,     // Instant effect cards
    BUFF_CARD,      // Cards that provide ongoing buffs
    DEBUFF_CARD     // Cards that apply debuffs
};

/**
 * @brief Enum representing different effect types for cards
 */
enum class EffectType {
    HEAL,           // Restore health to a piece
    DAMAGE,         // Deal damage to a piece
    BUFF_ATTACK,    // Increase attack value
    BUFF_HEALTH,    // Increase health value
    DEBUFF_ATTACK,  // Decrease attack value
    DEBUFF_HEALTH,  // Decrease health value
    MOVE_BOOST,     // Allow extra movement
    SHIELD,         // Provide damage protection
    POISON,         // Deal damage over time
    STUN           // Prevent piece from moving
};

/**
 * @brief Enum representing card rarity levels
 */
enum class CardRarity {
    COMMON,         // Basic cards, low steam cost
    UNCOMMON,       // Moderate power, medium steam cost
    RARE,           // Powerful effects, high steam cost
    LEGENDARY       // Game-changing effects, very high steam cost
};

// SFML Packet operators for CardType
inline sf::Packet& operator<<(sf::Packet& packet, const CardType& type) {
    return packet << static_cast<sf::Uint8>(type);
}

inline sf::Packet& operator>>(sf::Packet& packet, CardType& type) {
    sf::Uint8 type_uint8;
    packet >> type_uint8;
    type = static_cast<CardType>(type_uint8);
    return packet;
}

// SFML Packet operators for EffectType
inline sf::Packet& operator<<(sf::Packet& packet, const EffectType& type) {
    return packet << static_cast<sf::Uint8>(type);
}

inline sf::Packet& operator>>(sf::Packet& packet, EffectType& type) {
    sf::Uint8 type_uint8;
    packet >> type_uint8;
    type = static_cast<EffectType>(type_uint8);
    return packet;
}

// SFML Packet operators for CardRarity
inline sf::Packet& operator<<(sf::Packet& packet, const CardRarity& rarity) {
    return packet << static_cast<sf::Uint8>(rarity);
}

inline sf::Packet& operator>>(sf::Packet& packet, CardRarity& rarity) {
    sf::Uint8 rarity_uint8;
    packet >> rarity_uint8;
    rarity = static_cast<CardRarity>(rarity_uint8);
    return packet;
}

/**
 * @brief Abstract base class for all game cards
 * 
 * This class defines the common interface and properties for all card types.
 * Cards represent actions that players can take by spending steam resources.
 */
class Card {
public:
    /**
     * @brief Constructor
     * 
     * @param id Unique identifier for this card
     * @param name Display name of the card
     * @param description Text description of the card's effect
     * @param steamCost Amount of steam required to play this card
     * @param cardType The type of card this is
     * @param rarity The rarity level of this card
     */
    Card(int id, const std::string& name, const std::string& description, 
         int steamCost, CardType cardType, CardRarity rarity = CardRarity::COMMON);
    
    /**
     * @brief Virtual destructor
     */
    virtual ~Card() = default;
    
    /**
     * @brief Get the unique ID of this card
     * 
     * @return The card's unique identifier
     */
    int getId() const;
    
    /**
     * @brief Get the name of the card
     * 
     * @return The card's display name
     */
    const std::string& getName() const;
    
    /**
     * @brief Get the description of the card
     * 
     * @return The card's description text
     */
    const std::string& getDescription() const;
    
    /**
     * @brief Get the steam cost to play this card
     * 
     * @return The amount of steam required
     */
    int getSteamCost() const;
    
    /**
     * @brief Get the type of this card
     * 
     * @return The card type
     */
    CardType getCardType() const;
    
    /**
     * @brief Get the rarity of this card
     * 
     * @return The card rarity
     */
    CardRarity getRarity() const;
    
    /**
     * @brief Check if this card can be played in the current game state
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @return true if the card can be played, false otherwise
     */
    virtual bool canPlay(const GameState& gameState, PlayerSide player) const = 0;
    
    /**
     * @brief Execute the card's effect
     * 
     * @param gameState The game state to modify
     * @param player The player playing the card
     * @return true if the card was played successfully, false otherwise
     */
    virtual bool play(GameState& gameState, PlayerSide player) const = 0;
    
    /**
     * @brief Get a detailed description of what this card does
     * 
     * @return Extended description including mechanics
     */
    virtual std::string getDetailedDescription() const;
    
    /**
     * @brief Create a copy of this card
     * 
     * @return A unique pointer to a copy of this card
     */
    virtual std::unique_ptr<Card> clone() const = 0;

protected:
    int id;
    std::string name;
    std::string description;
    int steamCost;
    CardType cardType;
    CardRarity rarity;
};

// SFML Packet operators for Card base data
sf::Packet& operator<<(sf::Packet& packet, const Card& card);
sf::Packet& operator>>(sf::Packet& packet, Card& card);

} // namespace BayouBonanza 