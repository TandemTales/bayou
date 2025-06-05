#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>
#include "Card.h"

namespace BayouBonanza {

/**
 * @brief Class for managing collections of cards (decks, hands, etc.)
 * 
 * Provides functionality for storing, manipulating, and serializing
 * collections of cards with validation and utility methods.
 */
class CardCollection {
public:
    /**
     * @brief Default constructor
     */
    CardCollection();
    
    /**
     * @brief Constructor with initial cards
     * 
     * @param cards Vector of cards to initialize the collection with
     */
    CardCollection(std::vector<std::unique_ptr<Card>> cards);
    
    /**
     * @brief Copy constructor (performs deep copy)
     * 
     * @param other The collection to copy from
     */
    CardCollection(const CardCollection& other);
    
    /**
     * @brief Assignment operator (performs deep copy)
     * 
     * @param other The collection to copy from
     * @return Reference to this collection
     */
    CardCollection& operator=(const CardCollection& other);
    
    /**
     * @brief Move constructor
     * 
     * @param other The collection to move from
     */
    CardCollection(CardCollection&& other) noexcept;
    
    /**
     * @brief Move assignment operator
     * 
     * @param other The collection to move from
     * @return Reference to this collection
     */
    CardCollection& operator=(CardCollection&& other) noexcept;
    
    /**
     * @brief Add a card to the collection
     * 
     * @param card The card to add
     */
    void addCard(std::unique_ptr<Card> card);
    
    /**
     * @brief Remove a card at the specified index
     * 
     * @param index The index of the card to remove
     * @return The removed card, or nullptr if index is invalid
     */
    std::unique_ptr<Card> removeCardAt(size_t index);
    
    /**
     * @brief Remove the first card with the specified ID
     * 
     * @param cardId The ID of the card to remove
     * @return The removed card, or nullptr if not found
     */
    std::unique_ptr<Card> removeCardById(int cardId);
    
    /**
     * @brief Get a card at the specified index (const version)
     * 
     * @param index The index of the card to get
     * @return Pointer to the card, or nullptr if index is invalid
     */
    const Card* getCard(size_t index) const;
    
    /**
     * @brief Get a card at the specified index
     * 
     * @param index The index of the card to get
     * @return Pointer to the card, or nullptr if index is invalid
     */
    Card* getCard(size_t index);
    
    /**
     * @brief Find the first card with the specified ID
     * 
     * @param cardId The ID of the card to find
     * @return Pointer to the card, or nullptr if not found
     */
    const Card* findCard(int cardId) const;
    
    /**
     * @brief Get the number of cards in the collection
     * 
     * @return The size of the collection
     */
    size_t size() const;
    
    /**
     * @brief Check if the collection is empty
     * 
     * @return true if the collection is empty, false otherwise
     */
    bool empty() const;
    
    /**
     * @brief Clear all cards from the collection
     */
    void clear();
    
    /**
     * @brief Shuffle the cards in the collection
     */
    void shuffle();
    
    /**
     * @brief Get all card IDs in the collection
     * 
     * @return Vector of card IDs
     */
    std::vector<int> getCardIds() const;
    
    /**
     * @brief Count occurrences of each card ID
     * 
     * @return Map of card ID to count
     */
    std::map<int, int> getCardCounts() const;
    
    /**
     * @brief Validate the collection according to deck rules
     * 
     * @param maxSize Maximum allowed size (0 = no limit)
     * @param maxCopies Maximum copies of each card (0 = no limit)
     * @return true if the collection is valid, false otherwise
     */
    bool validate(size_t maxSize = 0, int maxCopies = 0) const;
    
    /**
     * @brief Serialize the collection to a string format
     * 
     * @return String representation of the collection
     */
    std::string serialize() const;
    
    /**
     * @brief Deserialize a collection from a string format
     * 
     * @param data The serialized data
     * @return true if deserialization was successful, false otherwise
     */
    bool deserialize(const std::string& data);
    
    /**
     * @brief Save the collection to a file
     * 
     * @param filename Path to the file to save to
     * @return true if saved successfully, false otherwise
     */
    bool saveToFile(const std::string& filename) const;
    
    /**
     * @brief Load the collection from a file
     * 
     * @param filename Path to the file to load from
     * @return true if loaded successfully, false otherwise
     */
    bool loadFromFile(const std::string& filename);
    
    /**
     * @brief Create a copy of the collection
     * 
     * @return A new CardCollection with copies of all cards
     */
    CardCollection clone() const;

protected:
    std::vector<std::unique_ptr<Card>> cards;
    
private:
    /**
     * @brief Deep copy cards from another collection
     * 
     * @param other The collection to copy from
     */
    void copyFrom(const CardCollection& other);
};

/**
 * @brief Specialized collection for player hands
 * 
 * Enforces hand size limits and provides hand-specific functionality.
 */
class Hand : public CardCollection {
public:
    static const size_t MAX_HAND_SIZE = 4;
    
    /**
     * @brief Default constructor
     */
    Hand();
    
    /**
     * @brief Add a card to the hand if there's space
     * 
     * @param card The card to add
     * @return true if the card was added, false if hand is full
     */
    bool addCard(std::unique_ptr<Card> card);
    
    /**
     * @brief Check if the hand is full
     * 
     * @return true if the hand has reached maximum capacity
     */
    bool isFull() const;
    
    /**
     * @brief Get the number of available slots in the hand
     * 
     * @return Number of slots available
     */
    size_t getAvailableSlots() const;
};

/**
 * @brief Specialized collection for player decks
 * 
 * Enforces deck composition rules and provides deck-specific functionality.
 */
class Deck : public CardCollection {
public:
    static const size_t DECK_SIZE = 20;
    static const int MAX_COPIES = 2;
    
    /**
     * @brief Default constructor
     */
    Deck();
    
    /**
     * @brief Constructor with initial cards
     * 
     * @param cards Vector of cards to initialize the deck with
     */
    Deck(std::vector<std::unique_ptr<Card>> cards);
    
    /**
     * @brief Draw a card from the top of the deck
     * 
     * @return The drawn card, or nullptr if deck is empty
     */
    std::unique_ptr<Card> drawCard();
    
    /**
     * @brief Peek at the top card without removing it
     * 
     * @return Pointer to the top card, or nullptr if deck is empty
     */
    const Card* peekTop() const;
    
    /**
     * @brief Check if the deck is valid according to game rules
     * 
     * @return true if the deck meets all requirements
     */
    bool isValid() const;
    
    /**
     * @brief Get the number of cards remaining in the deck
     * 
     * @return Number of cards left to draw
     */
    size_t cardsRemaining() const;
};

} // namespace BayouBonanza 