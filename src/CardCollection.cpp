#include "CardCollection.h"
#include "CardFactory.h"
#include <algorithm>
#include <random>
#include <fstream>
#include <sstream>
#include <set>
#include <iostream>

namespace BayouBonanza {

// CardCollection implementation
CardCollection::CardCollection() = default;

CardCollection::CardCollection(std::vector<std::unique_ptr<Card>> cards) 
    : cards(std::move(cards)) {
}

CardCollection::CardCollection(const CardCollection& other) {
    copyFrom(other);
}

CardCollection& CardCollection::operator=(const CardCollection& other) {
    if (this != &other) {
        copyFrom(other);
    }
    return *this;
}

CardCollection::CardCollection(CardCollection&& other) noexcept 
    : cards(std::move(other.cards)) {
}

CardCollection& CardCollection::operator=(CardCollection&& other) noexcept {
    if (this != &other) {
        cards = std::move(other.cards);
    }
    return *this;
}

void CardCollection::addCard(std::unique_ptr<Card> card) {
    if (card) {
        cards.push_back(std::move(card));
    }
}

std::unique_ptr<Card> CardCollection::removeCardAt(size_t index) {
    if (index >= cards.size()) {
        return nullptr;
    }
    
    auto card = std::move(cards[index]);
    cards.erase(cards.begin() + index);
    return card;
}

std::unique_ptr<Card> CardCollection::removeCardById(int cardId) {
    for (auto it = cards.begin(); it != cards.end(); ++it) {
        if ((*it)->getId() == cardId) {
            auto card = std::move(*it);
            cards.erase(it);
            return card;
        }
    }
    return nullptr;
}

const Card* CardCollection::getCard(size_t index) const {
    if (index >= cards.size()) {
        return nullptr;
    }
    return cards[index].get();
}

Card* CardCollection::getCard(size_t index) {
    if (index >= cards.size()) {
        return nullptr;
    }
    return cards[index].get();
}

const Card* CardCollection::findCard(int cardId) const {
    for (const auto& card : cards) {
        if (card->getId() == cardId) {
            return card.get();
        }
    }
    return nullptr;
}

size_t CardCollection::size() const {
    return cards.size();
}

bool CardCollection::empty() const {
    return cards.empty();
}

void CardCollection::clear() {
    cards.clear();
}

void CardCollection::shuffle() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(cards.begin(), cards.end(), gen);
}

std::vector<int> CardCollection::getCardIds() const {
    std::vector<int> ids;
    ids.reserve(cards.size());
    
    for (const auto& card : cards) {
        ids.push_back(card->getId());
    }
    
    return ids;
}

std::map<int, int> CardCollection::getCardCounts() const {
    std::map<int, int> counts;
    
    for (const auto& card : cards) {
        counts[card->getId()]++;
    }
    
    return counts;
}

bool CardCollection::validate(size_t maxSize, int maxCopies) const {
    // Check size limit
    if (maxSize > 0 && cards.size() > maxSize) {
        return false;
    }
    
    // Check copy limits
    if (maxCopies > 0) {
        auto counts = getCardCounts();
        for (const auto& pair : counts) {
            if (pair.second > maxCopies) {
                return false;
            }
        }
    }
    
    return true;
}

std::string CardCollection::serialize() const {
    std::ostringstream oss;
    
    // Simple format: "cardId1,cardId2,cardId3,..."
    for (size_t i = 0; i < cards.size(); ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << cards[i]->getId();
    }
    
    return oss.str();
}

bool CardCollection::deserialize(const std::string& data) {
    cards.clear();
    
    if (data.empty()) {
        return true; // Empty collection is valid
    }
    
    std::istringstream iss(data);
    std::string token;
    
    while (std::getline(iss, token, ',')) {
        try {
            int cardId = std::stoi(token);
            auto card = CardFactory::createCard(cardId);
            if (card) {
                cards.push_back(std::move(card));
            } else {
                // Invalid card ID found
                cards.clear();
                return false;
            }
        } catch (const std::exception&) {
            // Invalid number format
            cards.clear();
            return false;
        }
    }
    
    return true;
}

bool CardCollection::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    file << serialize();
    return file.good();
}

bool CardCollection::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string data;
    std::getline(file, data);
    
    return deserialize(data);
}

CardCollection CardCollection::clone() const {
    std::vector<std::unique_ptr<Card>> clonedCards;
    clonedCards.reserve(cards.size());
    
    for (const auto& card : cards) {
        clonedCards.push_back(card->clone());
    }
    
    return CardCollection(std::move(clonedCards));
}

void CardCollection::copyFrom(const CardCollection& other) {
    cards.clear();
    cards.reserve(other.cards.size());
    
    for (const auto& card : other.cards) {
        cards.push_back(card->clone());
    }
}

// Hand implementation
Hand::Hand() = default;

bool Hand::addCard(std::unique_ptr<Card> card) {
    if (isFull()) {
        return false;
    }
    
    CardCollection::addCard(std::move(card));
    return true;
}

bool Hand::isFull() const {
    return size() >= MAX_HAND_SIZE;
}

size_t Hand::getAvailableSlots() const {
    return MAX_HAND_SIZE - size();
}

// Deck implementation
Deck::Deck() = default;

Deck::Deck(std::vector<std::unique_ptr<Card>> cards,
           std::vector<std::unique_ptr<Card>> victory)
    : CardCollection(std::move(cards)) {
    for (auto& slot : victoryCards) slot = nullptr;
    for (size_t i = 0; i < std::min(victory.size(), VICTORY_SIZE); ++i) {
        victoryCards[i] = std::move(victory[i]);
    }
}

Deck::Deck(const Deck& other) : CardCollection(other) {
    for (auto& slot : victoryCards) slot = nullptr;
    for (size_t i = 0; i < VICTORY_SIZE; ++i) {
        if (other.victoryCards[i]) {
            victoryCards[i] = other.victoryCards[i]->clone();
        }
    }
}

Deck& Deck::operator=(const Deck& other) {
    if (this != &other) {
        CardCollection::operator=(other);
        for (auto& slot : victoryCards) slot = nullptr;
        for (size_t i = 0; i < VICTORY_SIZE; ++i) {
            if (other.victoryCards[i]) {
                victoryCards[i] = other.victoryCards[i]->clone();
            }
        }
    }
    return *this;
}

Deck::Deck(Deck&& other) noexcept = default;
Deck& Deck::operator=(Deck&& other) noexcept = default;

std::unique_ptr<Card> Deck::drawCard() {
    if (empty()) {
        return nullptr;
    }
    
    // Draw from the top (last element for efficiency)
    auto card = std::move(cards.back());
    cards.pop_back();
    return card;
}

const Card* Deck::peekTop() const {
    if (empty()) {
        return nullptr;
    }
    
    return cards.back().get();
}

bool Deck::isValid() const {
    if (!validate(DECK_SIZE, MAX_COPIES) || size() != DECK_SIZE) {
        return false;
    }

    std::set<int> ids;
    for (const auto& c : cards) {
        ids.insert(c->getId());
    }
    std::set<int> vicIds;
    for (const auto& c : victoryCards) {
        if (!c) continue;
        int id = c->getId();
        if (vicIds.count(id) || ids.count(id)) return false;
        vicIds.insert(id);
    }
    return true;
}

bool Deck::isValidForEditing() const {
    if (!validate(0, MAX_COPIES)) {
        return false;
    }

    std::set<int> ids;
    for (const auto& c : cards) {
        ids.insert(c->getId());
    }
    std::set<int> vicIds;
    for (const auto& c : victoryCards) {
        if (!c) continue;
        int id = c->getId();
        if (vicIds.count(id) || ids.count(id)) return false;
        vicIds.insert(id);
    }
    return true;
}

size_t Deck::cardsRemaining() const {
    return size();
}

bool Deck::setVictoryCard(size_t index, std::unique_ptr<Card> card) {
    if (index >= VICTORY_SIZE) {
        return false;
    }
    victoryCards[index] = std::move(card);
    return true;
}

void Deck::swapVictoryCards(size_t index1, size_t index2) {
    if (index1 >= VICTORY_SIZE || index2 >= VICTORY_SIZE) return;
    std::swap(victoryCards[index1], victoryCards[index2]);
}

std::unique_ptr<Card> Deck::removeVictoryCardAt(size_t index) {
    if (index >= VICTORY_SIZE) {
        return nullptr;
    }
    std::unique_ptr<Card> card = std::move(victoryCards[index]);
    victoryCards[index] = nullptr;
    return card;
}

const Card* Deck::getVictoryCard(size_t index) const {
    if (index >= VICTORY_SIZE) return nullptr;
    return victoryCards[index].get();
}

Card* Deck::getVictoryCard(size_t index) {
    if (index >= VICTORY_SIZE) return nullptr;
    return victoryCards[index].get();
}

size_t Deck::victoryCount() const {
    size_t count = 0;
    for (const auto& c : victoryCards) {
        if (c) ++count;
    }
    return count;
}

std::string Deck::serialize() const {
    std::ostringstream oss;
    for (size_t i = 0; i < cards.size(); ++i) {
        if (i > 0) oss << ",";
        oss << cards[i]->getId();
    }
    oss << "|";
    for (size_t i = 0; i < VICTORY_SIZE; ++i) {
        if (i > 0) oss << ",";
        if (victoryCards[i])
            oss << victoryCards[i]->getId();
        else
            oss << 0;
    }
    return oss.str();
}

bool Deck::deserialize(const std::string& data) {
    cards.clear();
    for (auto& slot : victoryCards) slot = nullptr;
    std::string mainPart = data;
    std::string victoryPart;
    size_t sep = data.find('|');
    if (sep != std::string::npos) {
        mainPart = data.substr(0, sep);
        victoryPart = data.substr(sep + 1);
    }
    if (!CardCollection::deserialize(mainPart)) {
        return false;
    }
    if (!victoryPart.empty()) {
        std::istringstream iss(victoryPart);
        std::string token;
        size_t idx = 0;
        while (idx < VICTORY_SIZE && std::getline(iss, token, ',')) {
            try {
                int id = std::stoi(token);
                if (id != 0) {
                    auto card = CardFactory::createCard(id);
                    if (card) {
                        victoryCards[idx] = std::move(card);
                    } else {
                        cards.clear();
                        for (auto& slot : victoryCards) slot = nullptr;
                        return false;
                    }
                } else {
                    victoryCards[idx] = nullptr;
                }
            } catch (...) {
                cards.clear();
                for (auto& slot : victoryCards) slot = nullptr;
                return false;
            }
            ++idx;
        }
    }
    return true;
}

} // namespace BayouBonanza 