#include "Card.h"
#include "GameState.h"

namespace BayouBonanza {

Card::Card(int id, const std::string& name, const std::string& description, 
           int steamCost, CardType cardType, CardRarity rarity)
    : id(id), name(name), description(description), steamCost(steamCost), 
      cardType(cardType), rarity(rarity) {
}

int Card::getId() const {
    return id;
}

const std::string& Card::getName() const {
    return name;
}

const std::string& Card::getDescription() const {
    return description;
}

int Card::getSteamCost() const {
    return steamCost;
}

CardType Card::getCardType() const {
    return cardType;
}

CardRarity Card::getRarity() const {
    return rarity;
}

std::string Card::getDetailedDescription() const {
    std::string detail = description;
    detail += "\n\nSteam Cost: " + std::to_string(steamCost);
    
    // Add rarity information
    std::string rarityStr;
    switch (rarity) {
        case CardRarity::COMMON: rarityStr = "Common"; break;
        case CardRarity::UNCOMMON: rarityStr = "Uncommon"; break;
        case CardRarity::RARE: rarityStr = "Rare"; break;
        case CardRarity::LEGENDARY: rarityStr = "Legendary"; break;
    }
    detail += "\nRarity: " + rarityStr;
    
    return detail;
}

// SFML Packet operators implementation
sf::Packet& operator<<(sf::Packet& packet, const Card& card) {
    packet << static_cast<sf::Int32>(card.getId());
    packet << card.getName();
    packet << card.getDescription();
    packet << static_cast<sf::Int32>(card.getSteamCost());
    packet << card.getCardType();
    packet << card.getRarity();
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, Card& card) {
    // Note: This is for reading base card data only
    // Actual card deserialization should be handled by CardFactory
    // This operator is mainly for debugging/testing purposes
    sf::Int32 id, steamCost;
    std::string name, description;
    CardType cardType;
    CardRarity rarity;
    
    packet >> id >> name >> description >> steamCost >> cardType >> rarity;
    
    // Note: We can't modify the card's data directly since they're private
    // This operator is mainly for completeness and debugging
    return packet;
}

} // namespace BayouBonanza 