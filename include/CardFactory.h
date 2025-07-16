#pragma once

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include "Card.h"
#include "PieceCard.h"
#include "EffectCard.h"

namespace BayouBonanza {

/**
 * @brief Structure representing a card definition for factory creation
 */
struct CardDefinition {
    int id;
    std::string name;
    std::string description;
    int steamCost;
    CardType cardType;
    CardRarity rarity;
    
    // For PieceCards
    std::string pieceType;
    
    // For EffectCards
    Effect effect;
    
    // Default constructor
    CardDefinition() 
        : id(0), name(""), description(""), steamCost(0),
          cardType(CardType::PIECE_CARD), rarity(CardRarity::COMMON),
          pieceType("Pawn"), effect(EffectType::HEAL, 0) {}
    
    CardDefinition(int id, const std::string& name, const std::string& description,
                   int steamCost, CardType cardType, CardRarity rarity = CardRarity::COMMON)
        : id(id), name(name), description(description), steamCost(steamCost),
          cardType(cardType), rarity(rarity), pieceType("Pawn"),
          effect(EffectType::HEAL, 0) {}
};

/**
 * @brief Factory class for creating cards and managing card definitions
 * 
 * The CardFactory provides a centralized way to create cards, manage
 * card definitions, and generate starter decks for players.
 */
class CardFactory {
public:
    /**
     * @brief Initialize the factory with default card definitions
     */
    static void initialize();
    
    /**
     * @brief Create a card by its unique ID
     * 
     * @param cardId The unique identifier of the card to create
     * @return A unique pointer to the created card, or nullptr if ID not found
     */
    static std::unique_ptr<Card> createCard(int cardId);
    
    /**
     * @brief Create a card by its name
     * 
     * @param cardName The name of the card to create
     * @return A unique pointer to the created card, or nullptr if name not found
     */
    static std::unique_ptr<Card> createCard(const std::string& cardName);
    
    /**
     * @brief Create a piece card for a specific piece type
     * 
     * @param pieceType The type of piece to create a card for
     * @return A unique pointer to the created piece card
     */
    static std::unique_ptr<PieceCard> createPieceCard(const std::string& pieceType);
    
    /**
     * @brief Create an effect card with specific parameters
     * 
     * @param effectType The type of effect
     * @param magnitude The strength of the effect
     * @param targetType The target type for the effect
     * @param steamCost The steam cost for the card
     * @param rarity The rarity of the card
     * @return A unique pointer to the created effect card
     */
    static std::unique_ptr<EffectCard> createEffectCard(EffectType effectType, int magnitude,
                                                        TargetType targetType, int steamCost,
                                                        CardRarity rarity = CardRarity::COMMON);
    
    /**
     * @brief Create a starter deck for a player
     * 
     * @return A vector of cards representing a balanced starter deck
     */
    static std::vector<std::unique_ptr<Card>> createStarterDeck();
    
    /**
     * @brief Create starter victory cards for a player
     * 
     * @return A vector of victory cards for the starter deck
     */
    static std::vector<std::unique_ptr<Card>> createStarterVictoryCards();
    
    /**
     * @brief Create a custom deck with specific card IDs
     * 
     * @param cardIds Vector of card IDs to include in the deck
     * @return A vector of cards for the custom deck
     */
    static std::vector<std::unique_ptr<Card>> createCustomDeck(const std::vector<int>& cardIds);
    
    /**
     * @brief Get all available card definitions
     * 
     * @return A map of card ID to card definition
     */
    static const std::map<int, CardDefinition>& getCardDefinitions();
    
    /**
     * @brief Get a card definition by ID
     * 
     * @param cardId The ID of the card definition to retrieve
     * @return Pointer to the card definition, or nullptr if not found
     */
    static const CardDefinition* getCardDefinition(int cardId);
    
    /**
     * @brief Get a card definition by name
     * 
     * @param cardName The name of the card definition to retrieve
     * @return Pointer to the card definition, or nullptr if not found
     */
    static const CardDefinition* getCardDefinition(const std::string& cardName);
    
    /**
     * @brief Add a new card definition to the factory
     * 
     * @param definition The card definition to add
     * @return true if added successfully, false if ID already exists
     */
    static bool addCardDefinition(const CardDefinition& definition);
    
    /**
     * @brief Validate a deck composition
     * 
     * Checks that the deck meets game rules:
     * - Exactly 20 cards
     * - Maximum 2 copies of each card
     * 
     * @param cardIds Vector of card IDs in the deck
     * @return true if the deck is valid, false otherwise
     */
    static bool validateDeck(const std::vector<int>& cardIds);
    
    /**
     * @brief Get all cards of a specific type
     * 
     * @param cardType The type of cards to retrieve
     * @return Vector of card IDs matching the type
     */
    static std::vector<int> getCardsByType(CardType cardType);
    
    /**
     * @brief Get all cards of a specific rarity
     * 
     * @param rarity The rarity level to filter by
     * @return Vector of card IDs matching the rarity
     */
    static std::vector<int> getCardsByRarity(CardRarity rarity);
    
    /**
     * @brief Load card definitions from a file
     * 
     * @param filename Path to the card definitions file
     * @return true if loaded successfully, false otherwise
     */
    static bool loadCardDefinitions(const std::string& filename);
    
    /**
     * @brief Save card definitions to a file
     * 
     * @param filename Path to save the card definitions
     * @return true if saved successfully, false otherwise
     */
    static bool saveCardDefinitions(const std::string& filename);

private:
    static std::map<int, CardDefinition> cardDefinitions;
    static std::map<std::string, int> nameToIdMap;
    static bool initialized;
    
    /**
     * @brief Create default card definitions
     */
    static void createDefaultDefinitions();
    
    /**
     * @brief Update the name-to-ID mapping
     */
    static void updateNameMapping();
    
    /**
     * @brief Get the next available card ID
     * 
     * @return The next available unique card ID
     */
    static int getNextCardId();
};

} // namespace BayouBonanza 