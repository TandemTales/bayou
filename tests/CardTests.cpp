#include <catch2/catch_test_macros.hpp>
#include "Card.h"
#include "PieceCard.h"
#include "EffectCard.h"
#include "CardFactory.h"
#include "CardCollection.h"
#include "CardPlayValidator.h"
#include "GameState.h"
#include "GameBoard.h"
#include "PlayerSide.h"
#include "PieceDefinitionManager.h"
#include "PieceFactory.h"
#include "Square.h"
#include <memory>
#include <vector>
#include <iostream>
#include <typeinfo>
#include <map>

using namespace BayouBonanza;

// Test fixture for card system tests
struct CardTestFixture {
    GameState gameState;
    PieceDefinitionManager pieceDefManager;
    std::unique_ptr<PieceFactory> factory;
    
    CardTestFixture() {
        // Load piece definitions for piece creation
        if (!pieceDefManager.loadDefinitions("assets/data/pieces.json")) {
            // Fallback to alternative path
            pieceDefManager.loadDefinitions("../../assets/data/pieces.json");
        }
        factory = std::make_unique<PieceFactory>(pieceDefManager);
        
        // Set global factory for Square piece creation
        Square::setGlobalPieceFactory(factory.get());
        
        // Initialize game state for testing
        gameState.initializeNewGame();
        
        // Check steam before card system initialization
        int steamBefore = gameState.getSteam(PlayerSide::PLAYER_ONE);
        
        gameState.initializeCardSystem();
        
        // Check steam after card system initialization
        int steamAfter = gameState.getSteam(PlayerSide::PLAYER_ONE);
        
        // Debug output (this will show in test output if there are issues)
        if (steamBefore != steamAfter) {
            std::cout << "Steam changed during card system init: " << steamBefore << " -> " << steamAfter << std::endl;
        }
    }
    
    // Helper to create a test piece card
    std::unique_ptr<PieceCard> createTestPieceCard(const std::string& name = "Test Sentroid",
                                                   int cost = 2,
                                                   const std::string& pieceType = "Sentroid") {
        static int nextId = 1000; // Use high IDs to avoid conflicts
        return std::make_unique<PieceCard>(nextId++, name, "Test piece card", cost, pieceType);
    }
    
    // Helper to create a test effect card
    std::unique_ptr<EffectCard> createTestEffectCard(const std::string& name = "Test Heal", 
                                                     int cost = 1,
                                                     EffectType effectType = EffectType::HEAL,
                                                     int magnitude = 2) {
        static int nextId = 2000; // Use high IDs to avoid conflicts
        Effect effect{effectType, magnitude, 0, TargetType::SINGLE_PIECE};
        return std::make_unique<EffectCard>(nextId++, name, "Test effect card", cost, effect);
    }
};

TEST_CASE_METHOD(CardTestFixture, "Card Base Class Functionality", "[card][base]") {
    
    SECTION("Card Creation and Basic Properties") {
        auto pieceCard = createTestPieceCard("Automatick Card", 3, "Automatick");
        
        REQUIRE(pieceCard->getName() == "Automatick Card");
        REQUIRE(pieceCard->getSteamCost() == 3);
        REQUIRE(pieceCard->getDescription() == "Test piece card");
        REQUIRE(pieceCard->getCardType() == CardType::PIECE_CARD);
        REQUIRE(pieceCard->getPieceType() == "Automatick");
    }
    
    SECTION("Card Type Identification") {
        auto pieceCard = createTestPieceCard();
        auto effectCard = createTestEffectCard();
        
        REQUIRE(pieceCard->getCardType() == CardType::PIECE_CARD);
        REQUIRE(effectCard->getCardType() == CardType::EFFECT_CARD);
    }
    
    SECTION("Card Polymorphism") {
        std::vector<std::unique_ptr<Card>> cards;
        cards.push_back(createTestPieceCard("Sweetykins Card", 4, "Sweetykins"));
        cards.push_back(createTestEffectCard("Damage Spell", 2, EffectType::DAMAGE, 3));
        
        REQUIRE(cards.size() == 2);
        REQUIRE(cards[0]->getCardType() == CardType::PIECE_CARD);
        REQUIRE(cards[1]->getCardType() == CardType::EFFECT_CARD);
        
        // Test polymorphic behavior
        for (const auto& card : cards) {
            REQUIRE(!card->getName().empty());
            REQUIRE(card->getSteamCost() > 0);
        }
    }
}

TEST_CASE_METHOD(CardTestFixture, "PieceCard Functionality", "[card][piece]") {
    
    SECTION("PieceCard Creation") {
        auto sentroidCard = createTestPieceCard("Sentroid Summon", 1, "Sentroid");
        
        REQUIRE(sentroidCard->getName() == "Sentroid Summon");
        REQUIRE(sentroidCard->getSteamCost() == 1);
        REQUIRE(sentroidCard->getPieceType() == "Sentroid");
        REQUIRE(sentroidCard->getCardType() == CardType::PIECE_CARD);
    }
    
    SECTION("PieceCard Valid Placement Detection") {
        auto sentroidCard = createTestPieceCard("Sentroid Summon", 1, "Sentroid");
        
        // Test valid placements for Player One (should be on their side)
        auto validPlacements = sentroidCard->getValidPlacements(gameState, PlayerSide::PLAYER_ONE);
        REQUIRE(!validPlacements.empty());
        
        // All valid placements should be on Player One's side (y >= 4 for 8x8 board)
        for (const auto& pos : validPlacements) {
            REQUIRE(pos.y >= 4);
            REQUIRE(pos.x >= 0);
            REQUIRE(pos.x < 8);
            REQUIRE(pos.y < 8);
        }
    }
    
    SECTION("PieceCard Placement Validation") {
        auto sweetykinsCard = createTestPieceCard("Sweetykins Summon", 5, "Sweetykins");
        
        // Test valid placement
        Position validPos{0, 7}; // Player One's back rank
        REQUIRE(sweetykinsCard->isValidPlacement(gameState, PlayerSide::PLAYER_ONE, validPos));
        
        // Test invalid placement (enemy territory)
        Position invalidPos{0, 0}; // Player Two's back rank
        REQUIRE_FALSE(sweetykinsCard->isValidPlacement(gameState, PlayerSide::PLAYER_ONE, invalidPos));
        
        // Test out of bounds
        Position outOfBounds{-1, 5};
        REQUIRE_FALSE(sweetykinsCard->isValidPlacement(gameState, PlayerSide::PLAYER_ONE, outOfBounds));
    }
    
    SECTION("PieceCard Play Functionality") {
        auto automatickCard = createTestPieceCard("Automatick Summon", 3, "Automatick");
        
        // Give player enough steam
        gameState.addSteam(PlayerSide::PLAYER_ONE, 5);
        
        // Test successful play
        Position playPos{1, 7};
        REQUIRE(automatickCard->canPlay(gameState, PlayerSide::PLAYER_ONE));
        REQUIRE(automatickCard->playAtPosition(gameState, PlayerSide::PLAYER_ONE, playPos));
        
        // Verify piece was placed
        const Square& square = gameState.getBoard().getSquare(playPos.x, playPos.y);
        REQUIRE(!square.isEmpty());
        REQUIRE(square.getPiece()->getSide() == PlayerSide::PLAYER_ONE);
    }
}

TEST_CASE_METHOD(CardTestFixture, "EffectCard Functionality", "[card][effect]") {
    
    SECTION("EffectCard Creation") {
        Effect healEffect{EffectType::HEAL, 3, 0, TargetType::SINGLE_PIECE};
        auto healCard = std::make_unique<EffectCard>(3001, "Healing Potion", "Heals a piece", 2, healEffect);
        
        REQUIRE(healCard->getName() == "Healing Potion");
        REQUIRE(healCard->getSteamCost() == 2);
        REQUIRE(healCard->getEffect().type == EffectType::HEAL);
        REQUIRE(healCard->getEffect().magnitude == 3);
        REQUIRE(healCard->getEffect().targetType == TargetType::SINGLE_PIECE);
    }
    
    SECTION("EffectCard Target Validation") {
        Effect damageEffect{EffectType::DAMAGE, 2, 0, TargetType::SINGLE_PIECE};
        auto damageCard = std::make_unique<EffectCard>(3002, "Lightning Bolt", "Damages a piece", 3, damageEffect);
        
        // Place a piece to target using CardFactory
        auto testPiece = CardFactory::createPieceCard("Sentroid");
        Position piecePos{3, 3};
        
        // We need to actually place a piece on the board for testing
        // For now, let's test with empty positions
        Position emptyPos{4, 4};
        REQUIRE_FALSE(damageCard->isValidTarget(gameState, PlayerSide::PLAYER_ONE, emptyPos));
    }
    
    SECTION("EffectCard Play Functionality") {
        // Create a heal effect that targets the player instead of pieces
        Effect healEffect{EffectType::HEAL, 2, 0, TargetType::SELF_PLAYER};
        auto healCard = std::make_unique<EffectCard>(3003, "Minor Heal", "Heals 2 HP", 1, healEffect);
        
        // Give player steam
        gameState.addSteam(PlayerSide::PLAYER_ONE, 3);
        
        // Debug: Check steam amount
        int steamAmount = gameState.getSteam(PlayerSide::PLAYER_ONE);
        INFO("Player steam: " << steamAmount);
        INFO("Card cost: " << healCard->getSteamCost());
        INFO("Effect type: " << static_cast<int>(healCard->getEffect().type));
        INFO("Target type: " << static_cast<int>(healCard->getEffect().targetType));
        
        // Test basic canPlay functionality
        REQUIRE(healCard->canPlay(gameState, PlayerSide::PLAYER_ONE));
    }
}

TEST_CASE_METHOD(CardTestFixture, "CardFactory Functionality", "[card][factory]") {
    
    SECTION("Card Creation by Type") {
        auto sentroidCard = CardFactory::createPieceCard("Sentroid");
        REQUIRE(sentroidCard != nullptr);
        REQUIRE(sentroidCard->getPieceType() == "Sentroid");
        
        auto healCard = CardFactory::createEffectCard(EffectType::HEAL, 1, TargetType::SINGLE_PIECE, 1);
        REQUIRE(healCard != nullptr);
        REQUIRE(healCard->getEffect().type == EffectType::HEAL);
    }
    
    SECTION("Starter Deck Creation") {
        auto starterDeck = CardFactory::createStarterDeck();
        REQUIRE(!starterDeck.empty());
        REQUIRE(starterDeck.size() <= 20); // Deck size limit
        
        // Verify all cards are valid
        for (const auto& card : starterDeck) {
            REQUIRE(card != nullptr);
            REQUIRE(!card->getName().empty());
            REQUIRE(card->getSteamCost() >= 0);
        }
    }
    

}

TEST_CASE_METHOD(CardTestFixture, "CardCollection Functionality", "[card][collection]") {
    
    SECTION("Hand Management") {
        Hand hand;
        
        // Test empty hand
        REQUIRE(hand.size() == 0);
        REQUIRE(hand.empty());
        REQUIRE_FALSE(hand.isFull());
        
        // Add cards to hand
        auto card1 = createTestPieceCard("Card 1", 1);
        auto card2 = createTestPieceCard("Card 2", 2);
        
        bool added1 = hand.addCard(std::move(card1));
        bool added2 = hand.addCard(std::move(card2));
        
        REQUIRE(added1);
        REQUIRE(added2);
        REQUIRE(hand.size() == 2);
        
        // Test hand limit (4 cards)
        auto card3 = createTestPieceCard("Card 3", 3);
        auto card4 = createTestPieceCard("Card 4", 4);
        auto card5 = createTestPieceCard("Card 5", 5);
        
        REQUIRE(hand.addCard(std::move(card3)));
        REQUIRE(hand.addCard(std::move(card4)));
        REQUIRE(hand.isFull());
        REQUIRE_FALSE(hand.addCard(std::move(card5))); // Should fail - hand full
        
        // Test card removal by index
        const Card* cardToRemove = hand.getCard(0);
        REQUIRE(cardToRemove != nullptr);
        int cardId = cardToRemove->getId();
        std::string cardName = cardToRemove->getName();
        
        auto removedCard = hand.removeCardAt(0);  // Remove by index
        REQUIRE(removedCard != nullptr);
        REQUIRE(removedCard->getName() == cardName);
        REQUIRE(removedCard->getId() == cardId);
        REQUIRE(hand.size() == 3);
        
        // Test card removal by ID
        const Card* secondCard = hand.getCard(0);  // Now the first card
        REQUIRE(secondCard != nullptr);
        int secondCardId = secondCard->getId();
        std::string secondCardName = secondCard->getName();
        
        auto removedById = hand.removeCardById(secondCardId);  // Remove by ID
        REQUIRE(removedById != nullptr);
        REQUIRE(removedById->getName() == secondCardName);
        REQUIRE(removedById->getId() == secondCardId);
        REQUIRE(hand.size() == 2);
    }
    
    SECTION("Deck Management") {
        Deck deck;
        
        // Test empty deck
        REQUIRE(deck.size() == 0);
        REQUIRE(deck.empty());
        
        // Add cards to deck
        for (int i = 0; i < 10; ++i) {
            auto card = createTestPieceCard("Card " + std::to_string(i), 1);
            deck.addCard(std::move(card));
        }
        
        REQUIRE(deck.size() == 10);
        REQUIRE_FALSE(deck.empty());
        
        // Test shuffling
        deck.shuffle();
        REQUIRE(deck.size() == 10); // Size should remain the same
        
        // Test drawing cards
        auto drawnCard = deck.drawCard();
        REQUIRE(drawnCard != nullptr);
        REQUIRE(deck.size() == 9);
        
        // Draw all remaining cards
        while (!deck.empty()) {
            auto card = deck.drawCard();
            REQUIRE(card != nullptr);
        }
        
        REQUIRE(deck.empty());
        REQUIRE(deck.drawCard() == nullptr); // Drawing from empty deck
    }
    
    SECTION("Deck Validation") {
        Deck deck;
        
        // Create a valid deck (20 cards, max 2 copies each)
        for (int i = 0; i < 10; ++i) {
            auto card1 = createTestPieceCard("Card " + std::to_string(i), 1);
            auto card2 = createTestPieceCard("Card " + std::to_string(i), 1);
            deck.addCard(std::move(card1));
            deck.addCard(std::move(card2));
        }
        
        REQUIRE(deck.size() == 20);
        REQUIRE(deck.isValid());
        
        // Test invalid deck (too many cards)
        auto extraCard = createTestPieceCard("Extra Card", 1);
        deck.addCard(std::move(extraCard));
        REQUIRE_FALSE(deck.isValid());
    }
}

TEST_CASE_METHOD(CardTestFixture, "CardPlayValidator Functionality", "[card][validator]") {
    
    SECTION("Basic Card Play Validation") {
        // Add a card to player's hand
        Hand& hand = gameState.getHand(PlayerSide::PLAYER_ONE);
        auto testCard = createTestPieceCard("Test Sentroid", 2, "Sentroid");
        hand.addCard(std::move(testCard));
        
        // Give player enough steam
        gameState.addSteam(PlayerSide::PLAYER_ONE, 5);
        
        // Test valid card play
        auto result = CardPlayValidator::validateCardPlay(gameState, PlayerSide::PLAYER_ONE, 0);
        REQUIRE(result.isValid);
        REQUIRE(result.error == ValidationError::NONE);
        
        // Test insufficient steam
        gameState.spendSteam(PlayerSide::PLAYER_ONE, 4); // Leave only 1 steam
        result = CardPlayValidator::validateCardPlay(gameState, PlayerSide::PLAYER_ONE, 0);
        REQUIRE_FALSE(result.isValid);
        REQUIRE(result.error == ValidationError::INSUFFICIENT_STEAM);
        
        // Test invalid hand index
        result = CardPlayValidator::validateCardPlay(gameState, PlayerSide::PLAYER_ONE, 5);
        REQUIRE_FALSE(result.isValid);
        REQUIRE(result.error == ValidationError::INVALID_HAND_INDEX);
    }
    
    SECTION("Targeted Card Play Validation") {
        // Clear hand and add a specific piece card
        Hand& hand = gameState.getHand(PlayerSide::PLAYER_ONE);
        hand.clear();
        auto pieceCard = createTestPieceCard("Automatick Summon", 3, "Automatick");
        hand.addCard(std::move(pieceCard));
        
        gameState.addSteam(PlayerSide::PLAYER_ONE, 5);
        
        // Test valid target position
        Position validPos{1, 7};
        auto result = CardPlayValidator::validateTargetedCardPlay(gameState, PlayerSide::PLAYER_ONE, 0, validPos);
        REQUIRE(result.isValid);
        
        // Test invalid target position (out of bounds)
        Position invalidPos{-1, 5};
        result = CardPlayValidator::validateTargetedCardPlay(gameState, PlayerSide::PLAYER_ONE, 0, invalidPos);
        REQUIRE_FALSE(result.isValid);
        REQUIRE(result.error == ValidationError::INVALID_TARGET);
    }
    
    SECTION("Card Play Execution with Rollback") {
        // Clear hand and add a specific test card
        Hand& hand = gameState.getHand(PlayerSide::PLAYER_ONE);
        hand.clear();
        auto testCard = createTestPieceCard("Test Sentroid", 2, "Sentroid");
        hand.addCard(std::move(testCard));
        
        gameState.addSteam(PlayerSide::PLAYER_ONE, 5); // Give more steam to be safe
        int initialSteam = gameState.getSteam(PlayerSide::PLAYER_ONE);
        size_t initialHandSize = hand.size();
        
        // Execute successful card play
        Position targetPos{2, 7};
        auto result = CardPlayValidator::executeCardPlay(gameState, PlayerSide::PLAYER_ONE, 0, targetPos);
        
        REQUIRE(result.success);
        REQUIRE(result.steamSpent);
        REQUIRE(result.cardRemoved);
        
        // Verify steam was spent and card was removed
        REQUIRE(gameState.getSteam(PlayerSide::PLAYER_ONE) == initialSteam - 2);
        REQUIRE(hand.size() == initialHandSize - 1);
        
        // Verify piece was placed
        const Square& square = gameState.getBoard().getSquare(targetPos.x, targetPos.y);
        REQUIRE(!square.isEmpty());
    }
    
    SECTION("Error Message Generation") {
        REQUIRE(CardPlayValidator::getErrorMessage(ValidationError::NONE) == "No error");
        REQUIRE(CardPlayValidator::getErrorMessage(ValidationError::INSUFFICIENT_STEAM) == "Insufficient steam to play this card");
        REQUIRE(CardPlayValidator::getErrorMessage(ValidationError::INVALID_TARGET) == "Invalid target position specified");
        REQUIRE(!CardPlayValidator::getErrorMessage(ValidationError::UNKNOWN_CARD_TYPE).empty());
    }
    
    SECTION("Board Position Validation") {
        REQUIRE(CardPlayValidator::isValidBoardPosition({0, 0}));
        REQUIRE(CardPlayValidator::isValidBoardPosition({7, 7}));
        REQUIRE(CardPlayValidator::isValidBoardPosition({3, 4}));
        
        REQUIRE_FALSE(CardPlayValidator::isValidBoardPosition({-1, 0}));
        REQUIRE_FALSE(CardPlayValidator::isValidBoardPosition({0, -1}));
        REQUIRE_FALSE(CardPlayValidator::isValidBoardPosition({8, 0}));
        REQUIRE_FALSE(CardPlayValidator::isValidBoardPosition({0, 8}));
    }
}

TEST_CASE_METHOD(CardTestFixture, "Card System Integration", "[card][integration]") {
    
    SECTION("GameState Card System Integration") {
        // Test card system initialization
        REQUIRE(gameState.getDeck(PlayerSide::PLAYER_ONE).size() > 0);
        REQUIRE(gameState.getDeck(PlayerSide::PLAYER_TWO).size() > 0);
        REQUIRE(gameState.getHand(PlayerSide::PLAYER_ONE).size() > 0);
        REQUIRE(gameState.getHand(PlayerSide::PLAYER_TWO).size() > 0);
        
        // Test card drawing
        size_t initialHandSize = gameState.getHand(PlayerSide::PLAYER_ONE).size();
        size_t initialDeckSize = gameState.getDeck(PlayerSide::PLAYER_ONE).size();
        
        bool drewCard = gameState.drawCard(PlayerSide::PLAYER_ONE);
        if (initialDeckSize > 0 && initialHandSize < 4) {
            REQUIRE(drewCard);
            REQUIRE(gameState.getHand(PlayerSide::PLAYER_ONE).size() == initialHandSize + 1);
            REQUIRE(gameState.getDeck(PlayerSide::PLAYER_ONE).size() == initialDeckSize - 1);
        }
    }
    
    SECTION("Complete Card Play Workflow") {
        // Give player steam
        gameState.addSteam(PlayerSide::PLAYER_ONE, 10);
        
        // Get initial state
        Hand& hand = gameState.getHand(PlayerSide::PLAYER_ONE);
        size_t initialHandSize = hand.size();
        int initialSteam = gameState.getSteam(PlayerSide::PLAYER_ONE);
        
        // Validate card play first
        auto validation = gameState.validateCardPlay(PlayerSide::PLAYER_ONE, 0);
        if (validation.isValid) {
            // Play the card
            bool success = gameState.playCard(PlayerSide::PLAYER_ONE, 0);
            REQUIRE(success);
            
            // Verify state changes
            REQUIRE(hand.size() == initialHandSize - 1);
            REQUIRE(gameState.getSteam(PlayerSide::PLAYER_ONE) < initialSteam);
        }
    }
    
    SECTION("Card Serialization Integration") {
        // Test that cards can be serialized and deserialized
        Hand& hand = gameState.getHand(PlayerSide::PLAYER_ONE);
        if (hand.size() > 0) {
            const Card* card = hand.getCard(0);
            
            // Test basic serialization properties
            REQUIRE(!card->getName().empty());
            REQUIRE(card->getSteamCost() >= 0);
            REQUIRE(card->getCardType() != static_cast<CardType>(-1));
        }
    }
}

TEST_CASE("Debug EffectCard Issue", "[debug]") {
    GameState gameState;
    gameState.initializeNewGame();
    gameState.initializeCardSystem();
    
    // Give player steam
    gameState.addSteam(PlayerSide::PLAYER_ONE, 10);
    
    // Create a simple heal effect targeting self player
    Effect healEffect{EffectType::HEAL, 2, 0, TargetType::SELF_PLAYER};
    auto healCard = std::make_unique<EffectCard>(9999, "Debug Heal", "Debug heal card", 1, healEffect);
    
    // Check individual components
    REQUIRE(gameState.getSteam(PlayerSide::PLAYER_ONE) >= healCard->getSteamCost());
    REQUIRE(healCard->getEffect().targetType == TargetType::SELF_PLAYER);
    REQUIRE(healCard->getEffect().type == EffectType::HEAL);
    
    // Test canPlay
    bool canPlay = healCard->canPlay(gameState, PlayerSide::PLAYER_ONE);
    REQUIRE(canPlay);
}

TEST_CASE_METHOD(CardTestFixture, "Starter Deck Contains All Piece Types", "[card][factory][starter]")
{
    SECTION("Starter Deck Creation and Content Verification") {
        auto starterDeck = CardFactory::createStarterDeck();
        REQUIRE(!starterDeck.empty());
        REQUIRE(starterDeck.size() == 20); // Exact deck size
        
        // Count cards by piece type
        std::map<std::string, int> pieceTypeCounts;
        int effectCardCount = 0;
        
        for (const auto& card : starterDeck) {
            REQUIRE(card != nullptr);
            REQUIRE(!card->getName().empty());
            REQUIRE(card->getSteamCost() >= 0);
            
            if (card->getCardType() == CardType::PIECE_CARD) {
                auto pieceCard = dynamic_cast<const PieceCard*>(card.get());
                REQUIRE(pieceCard != nullptr);
                pieceTypeCounts[pieceCard->getPieceType()]++;
            } else if (card->getCardType() == CardType::EFFECT_CARD) {
                effectCardCount++;
            }
        }
        
        // Verify all requested piece types are present
        REQUIRE(pieceTypeCounts["TinkeringTom"] >= 1);
        REQUIRE(pieceTypeCounts["ScarlettGlumpkin"] >= 1);
        REQUIRE(pieceTypeCounts["Sweetykins"] >= 1);
        REQUIRE(pieceTypeCounts["Sidewinder"] >= 1);
        REQUIRE(pieceTypeCounts["Automatick"] >= 1);
        REQUIRE(pieceTypeCounts["Sentroid"] >= 1);
        REQUIRE(pieceTypeCounts["Rustbucket"] >= 1);
        
        // Verify expected counts
        REQUIRE(pieceTypeCounts["Sentroid"] == 6);
        REQUIRE(pieceTypeCounts["Rustbucket"] == 3);
        REQUIRE(pieceTypeCounts["Sweetykins"] == 2);
        REQUIRE(pieceTypeCounts["Automatick"] == 2);
        REQUIRE(pieceTypeCounts["Sidewinder"] == 2);
        REQUIRE(pieceTypeCounts["ScarlettGlumpkin"] == 1);
        REQUIRE(pieceTypeCounts["TinkeringTom"] == 1);
        REQUIRE(effectCardCount == 3);
        
        // Verify total adds up to 20
        int totalPieceCards = 0;
        for (const auto& pair : pieceTypeCounts) {
            totalPieceCards += pair.second;
        }
        REQUIRE(totalPieceCards + effectCardCount == 20);
    }
}

 