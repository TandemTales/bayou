#include "CardFactory.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <set>
#include "nlohmann/json.hpp"
#include <stdexcept>  // Add for std::invalid_argument

namespace BayouBonanza {

// Static member definitions
std::map<int, CardDefinition> CardFactory::cardDefinitions;
std::map<std::string, int> CardFactory::nameToIdMap;
bool CardFactory::initialized = false;

void CardFactory::initialize() {
    if (initialized) {
        return;
    }
    
    createDefaultDefinitions();
    updateNameMapping();
    initialized = true;
}

std::unique_ptr<Card> CardFactory::createCard(int cardId) {
    if (!initialized) {
        initialize();
    }
    
    auto it = cardDefinitions.find(cardId);
    if (it == cardDefinitions.end()) {
        return nullptr;
    }
    
    const CardDefinition& def = it->second;
    
    switch (def.cardType) {
        case CardType::PIECE_CARD:
            return std::make_unique<PieceCard>(def.id, def.name, def.description,
                                               def.steamCost, def.pieceType, def.rarity);
        case CardType::EFFECT_CARD:
            return std::make_unique<EffectCard>(def.id, def.name, def.description,
                                                def.steamCost, def.effect, def.rarity);
        default:
            // For other card types, we'd need to implement them
            return nullptr;
    }
}

std::unique_ptr<Card> CardFactory::createCard(const std::string& cardName) {
    if (!initialized) {
        initialize();
    }
    
    auto it = nameToIdMap.find(cardName);
    if (it == nameToIdMap.end()) {
        return nullptr;
    }
    
    return createCard(it->second);
}

std::unique_ptr<PieceCard> CardFactory::createPieceCard(const std::string& pieceType) {
    if (!initialized) {
        initialize();
    }
    
    // Find a piece card definition for this piece type
    for (const auto& pair : cardDefinitions) {
        const CardDefinition& def = pair.second;
        if (def.cardType == CardType::PIECE_CARD && def.pieceType == pieceType) {
            return std::make_unique<PieceCard>(def.id, def.name, def.description,
                                               def.steamCost, def.pieceType, def.rarity);
        }
    }

    // If no definition found, create a basic one
    int id = getNextCardId();
    std::string name = "Summon " + pieceType;
    std::string description = "Summon a " + pieceType + " piece to the battlefield";
    int steamCost = 3;

    return std::make_unique<PieceCard>(id, name, description, steamCost, pieceType);
}

std::unique_ptr<EffectCard> CardFactory::createEffectCard(EffectType effectType, int magnitude,
                                                          TargetType targetType, int steamCost,
                                                          CardRarity rarity) {
    int id = getNextCardId();
    std::string name;
    std::string description;
    
    switch (effectType) {
        case EffectType::HEAL:
            name = "Healing Light";
            description = "Restore " + std::to_string(magnitude) + " health to target";
            break;
        case EffectType::DAMAGE:
            name = "Lightning Bolt";
            description = "Deal " + std::to_string(magnitude) + " damage to target";
            break;
        case EffectType::BUFF_ATTACK:
            name = "Battle Fury";
            description = "Increase attack by " + std::to_string(magnitude);
            break;
        case EffectType::BUFF_HEALTH:
            name = "Fortify";
            description = "Increase health by " + std::to_string(magnitude);
            break;
        default:
            name = "Unknown Effect";
            description = "Apply unknown effect";
            break;
    }
    
    Effect effect(effectType, magnitude, 0, targetType);
    return std::make_unique<EffectCard>(id, name, description, steamCost, effect, rarity);
}

std::vector<std::unique_ptr<Card>> CardFactory::createStarterDeck() {
    if (!initialized) {
        initialize();
    }
    
    std::vector<std::unique_ptr<Card>> deck;
    
    // 6 Sentroid cards
    for (int i = 0; i < 6; i++) {
        deck.push_back(createCard("Summon Sentroid"));
    }
    
    // 3 Rustbucket cards
    for (int i = 0; i < 3; i++) {
        deck.push_back(createCard("Summon Rustbucket"));
    }
    
    // 2 Sweetykins cards
    for (int i = 0; i < 2; i++) {
        deck.push_back(createCard("Summon Sweetykins"));
    }
    
    // 2 Automatick cards
    for (int i = 0; i < 2; i++) {
        deck.push_back(createCard("Summon Automatick"));
    }
    
    // 2 Sidewinder cards
    for (int i = 0; i < 2; i++) {
        deck.push_back(createCard("Summon Sidewinder"));
    }
    
    // 1 ScarlettGlumpkin card
    deck.push_back(createCard("Summon ScarlettGlumpkin"));
    
    // 1 TinkeringTom card
    deck.push_back(createCard("Summon TinkeringTom"));
    
    // 3 Effect cards (Healing Light)
    for (int i = 0; i < 3; i++) {
        deck.push_back(createCard("Healing Light"));
    }
    
    return deck;
}

std::vector<std::unique_ptr<Card>> CardFactory::createCustomDeck(const std::vector<int>& cardIds) {
    if (!initialized) {
        initialize();
    }
    
    std::vector<std::unique_ptr<Card>> deck;
    
    for (int cardId : cardIds) {
        auto card = createCard(cardId);
        if (card) {
            deck.push_back(std::move(card));
        }
    }
    
    return deck;
}

const std::map<int, CardDefinition>& CardFactory::getCardDefinitions() {
    if (!initialized) {
        initialize();
    }
    return cardDefinitions;
}

const CardDefinition* CardFactory::getCardDefinition(int cardId) {
    if (!initialized) {
        initialize();
    }
    
    auto it = cardDefinitions.find(cardId);
    return (it != cardDefinitions.end()) ? &it->second : nullptr;
}

const CardDefinition* CardFactory::getCardDefinition(const std::string& cardName) {
    if (!initialized) {
        initialize();
    }
    
    auto it = nameToIdMap.find(cardName);
    if (it != nameToIdMap.end()) {
        return getCardDefinition(it->second);
    }
    return nullptr;
}

bool CardFactory::addCardDefinition(const CardDefinition& definition) {
    if (!initialized) {
        initialize();
    }
    
    // Check if ID already exists
    if (cardDefinitions.find(definition.id) != cardDefinitions.end()) {
        return false;
    }
    
    cardDefinitions[definition.id] = definition;
    nameToIdMap[definition.name] = definition.id;
    return true;
}

bool CardFactory::validateDeck(const std::vector<int>& cardIds) {
    // Check deck size
    if (cardIds.size() != 20) {
        return false;
    }
    
    // Count occurrences of each card
    std::map<int, int> cardCounts;
    for (int cardId : cardIds) {
        cardCounts[cardId]++;
        
        // Check if any card appears more than 2 times
        if (cardCounts[cardId] > 2) {
            return false;
        }
    }
    
    return true;
}

std::vector<int> CardFactory::getCardsByType(CardType cardType) {
    if (!initialized) {
        initialize();
    }
    
    std::vector<int> result;
    for (const auto& pair : cardDefinitions) {
        if (pair.second.cardType == cardType) {
            result.push_back(pair.first);
        }
    }
    return result;
}

std::vector<int> CardFactory::getCardsByRarity(CardRarity rarity) {
    if (!initialized) {
        initialize();
    }
    
    std::vector<int> result;
    for (const auto& pair : cardDefinitions) {
        if (pair.second.rarity == rarity) {
            result.push_back(pair.first);
        }
    }
    return result;
}

bool CardFactory::loadCardDefinitions(const std::string& filename) {
    // TODO: Implement JSON loading
    // For now, just return false to indicate not implemented
    return false;
}

bool CardFactory::saveCardDefinitions(const std::string& filename) {
    // TODO: Implement JSON saving
    // For now, just return false to indicate not implemented
    return false;
}

// Helper functions for parsing enums from strings
EffectType getEffectType(const std::string& str) {
    if (str == "HEAL") return EffectType::HEAL;
    if (str == "DAMAGE") return EffectType::DAMAGE;
    if (str == "BUFF_ATTACK") return EffectType::BUFF_ATTACK;
    if (str == "BUFF_HEALTH") return EffectType::BUFF_HEALTH;
    throw std::invalid_argument("Unknown effect type: " + str);
}

TargetType getTargetType(const std::string& str) {
    if (str == "SINGLE_PIECE") return TargetType::SINGLE_PIECE;
    if (str == "ALL_FRIENDLY") return TargetType::ALL_FRIENDLY;
    throw std::invalid_argument("Unknown target type: " + str);
}

void CardFactory::createDefaultDefinitions() {
    cardDefinitions.clear();
    
    std::ifstream file("assets/data/cards.json");
    if (!file.is_open()) {
        std::cerr << "Failed to open cards.json" << std::endl;
        return;
    }
    
    nlohmann::json cardsData;
    file >> cardsData;
    
    int id = 1;
    for (const auto& cardJson : cardsData) {
        std::string cardTypeStr = cardJson.value("cardType", "PIECE_CARD");
        CardType cardType;
        if (cardTypeStr == "PIECE_CARD") {
            cardType = CardType::PIECE_CARD;
        } else if (cardTypeStr == "EFFECT_CARD") {
            cardType = CardType::EFFECT_CARD;
        } else {
            std::cerr << "Unknown card type: " << cardTypeStr << std::endl;
            continue;
        }
        
        std::string rarityStr = cardJson.value("rarity", "COMMON");
        CardRarity rarity;
        if (rarityStr == "COMMON") rarity = CardRarity::COMMON;
        else if (rarityStr == "UNCOMMON") rarity = CardRarity::UNCOMMON;
        else if (rarityStr == "RARE") rarity = CardRarity::RARE;
        else rarity = CardRarity::COMMON;
        
        int steamCost = cardJson["steamCost"].get<int>();
        
        CardDefinition def(id, "", "", steamCost, cardType, rarity);
        
        if (cardType == CardType::PIECE_CARD) {
            std::string typeName = cardJson["typeName"].get<std::string>();
            def.name = "Summon " + typeName;
            def.description = "Summon a " + typeName + " piece to the battlefield";
            def.pieceType = typeName;
        } else if (cardType == CardType::EFFECT_CARD) {
            def.name = cardJson["name"].get<std::string>();
            def.description = cardJson["description"].get<std::string>();
            
            std::string effectTypeStr = cardJson["effectType"].get<std::string>();
            EffectType effectType = getEffectType(effectTypeStr);
            
            int magnitude = cardJson["magnitude"].get<int>();
            int duration = cardJson.value("duration", 0);
            
            std::string targetTypeStr = cardJson["targetType"].get<std::string>();
            TargetType targetType = getTargetType(targetTypeStr);
            
            def.effect = Effect(effectType, magnitude, duration, targetType);
        }
        
        cardDefinitions[id] = def;
        id++;
    }
}

void CardFactory::updateNameMapping() {
    nameToIdMap.clear();
    for (const auto& pair : cardDefinitions) {
        nameToIdMap[pair.second.name] = pair.first;
    }
}

int CardFactory::getNextCardId() {
    if (cardDefinitions.empty()) {
        return 1;
    }
    
    int maxId = 0;
    for (const auto& pair : cardDefinitions) {
        maxId = std::max(maxId, pair.first);
    }
    return maxId + 1;
}

} // namespace BayouBonanza 