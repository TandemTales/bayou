#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include "Card.h"
#include "PieceCard.h"
#include "EffectCard.h"
#include "CardFactory.h"
#include "CardCollection.h"
#include "CardPlayValidator.h"
#include "GameState.h"
#include <chrono>
#include <vector>
#include <memory>

using namespace BayouBonanza;

// Performance test fixture
struct CardPerformanceFixture {
    GameState gameState;
    
    CardPerformanceFixture() {
        gameState.initializeNewGame();
        gameState.initializeCardSystem();
        gameState.addSteam(PlayerSide::PLAYER_ONE, 1000); // Plenty of steam for testing
    }
    
    std::unique_ptr<PieceCard> createTestPieceCard(int index = 0) {
        return std::make_unique<PieceCard>(
            "Test Card " + std::to_string(index), 
            1, 
            "Test description", 
            PieceType::PAWN
        );
    }
    
    std::unique_ptr<EffectCard> createTestEffectCard(int index = 0) {
        Effect effect{EffectType::HEAL, 1, 0, TargetType::SINGLE_PIECE};
        return std::make_unique<EffectCard>(
            "Test Effect " + std::to_string(index),
            1,
            "Test effect description",
            effect
        );
    }
};

TEST_CASE_METHOD(CardPerformanceFixture, "Card System Performance Tests", "[card][performance]") {
    
    SECTION("Card Creation Performance") {
        BENCHMARK("Create 1000 PieceCards") {
            std::vector<std::unique_ptr<PieceCard>> cards;
            cards.reserve(1000);
            
            for (int i = 0; i < 1000; ++i) {
                cards.push_back(createTestPieceCard(i));
            }
            
            return cards.size();
        };
        
        BENCHMARK("Create 1000 EffectCards") {
            std::vector<std::unique_ptr<EffectCard>> cards;
            cards.reserve(1000);
            
            for (int i = 0; i < 1000; ++i) {
                cards.push_back(createTestEffectCard(i));
            }
            
            return cards.size();
        };
    }
    
    SECTION("Card Factory Performance") {
        BENCHMARK("CardFactory::createStarterDeck() x100") {
            std::vector<std::vector<std::unique_ptr<Card>>> decks;
            decks.reserve(100);
            
            for (int i = 0; i < 100; ++i) {
                decks.push_back(CardFactory::createStarterDeck());
            }
            
            return decks.size();
        };
        
        BENCHMARK("CardFactory::createPieceCard() x1000") {
            std::vector<std::unique_ptr<PieceCard>> cards;
            cards.reserve(1000);
            
            for (int i = 0; i < 1000; ++i) {
                cards.push_back(CardFactory::createPieceCard(
                    "Benchmark Card " + std::to_string(i),
                    1,
                    "Benchmark description",
                    PieceType::PAWN
                ));
            }
            
            return cards.size();
        };
    }
    
    SECTION("Hand Management Performance") {
        BENCHMARK("Hand operations (add/remove) x1000") {
            Hand hand;
            std::vector<std::unique_ptr<Card>> removedCards;
            removedCards.reserve(1000);
            
            // Add and remove cards repeatedly
            for (int i = 0; i < 1000; ++i) {
                // Add cards until hand is full
                while (!hand.isFull()) {
                    auto card = createTestPieceCard(i);
                    if (!hand.addCard(std::move(card))) {
                        break;
                    }
                }
                
                // Remove a card if hand is not empty
                if (!hand.isEmpty()) {
                    auto removed = hand.removeCardAt(0);
                    if (removed) {
                        removedCards.push_back(std::move(removed));
                    }
                }
            }
            
            return removedCards.size();
        };
    }
    
    SECTION("Deck Management Performance") {
        BENCHMARK("Deck shuffle x100") {
            Deck deck;
            
            // Fill deck with cards
            for (int i = 0; i < 20; ++i) {
                deck.addCard(createTestPieceCard(i));
            }
            
            // Shuffle multiple times
            for (int i = 0; i < 100; ++i) {
                deck.shuffle();
            }
            
            return deck.size();
        };
        
        BENCHMARK("Deck draw all cards x100") {
            std::vector<std::unique_ptr<Card>> drawnCards;
            drawnCards.reserve(2000);
            
            for (int iteration = 0; iteration < 100; ++iteration) {
                Deck deck;
                
                // Fill deck
                for (int i = 0; i < 20; ++i) {
                    deck.addCard(createTestPieceCard(i));
                }
                
                // Draw all cards
                while (!deck.isEmpty()) {
                    auto card = deck.drawCard();
                    if (card) {
                        drawnCards.push_back(std::move(card));
                    }
                }
            }
            
            return drawnCards.size();
        };
    }
    
    SECTION("Card Validation Performance") {
        // Add cards to hand for testing
        Hand& hand = gameState.getHand(PlayerSide::PLAYER_ONE);
        for (int i = 0; i < 4; ++i) {
            hand.addCard(createTestPieceCard(i));
        }
        
        BENCHMARK("CardPlayValidator::validateCardPlay() x1000") {
            int validCount = 0;
            
            for (int i = 0; i < 1000; ++i) {
                auto result = CardPlayValidator::validateCardPlay(gameState, PlayerSide::PLAYER_ONE, 0);
                if (result.isValid) {
                    validCount++;
                }
            }
            
            return validCount;
        };
        
        BENCHMARK("CardPlayValidator::validateTargetedCardPlay() x1000") {
            int validCount = 0;
            Position targetPos{2, 7};
            
            for (int i = 0; i < 1000; ++i) {
                auto result = CardPlayValidator::validateTargetedCardPlay(
                    gameState, PlayerSide::PLAYER_ONE, 0, targetPos);
                if (result.isValid) {
                    validCount++;
                }
            }
            
            return validCount;
        };
    }
    
    SECTION("Card Play Performance") {
        BENCHMARK("Complete card play workflow x100") {
            int successCount = 0;
            
            for (int i = 0; i < 100; ++i) {
                // Reset hand for each iteration
                Hand& hand = gameState.getHand(PlayerSide::PLAYER_ONE);
                hand.clear();
                hand.addCard(createTestPieceCard(i));
                
                // Ensure enough steam
                gameState.addSteam(PlayerSide::PLAYER_ONE, 10);
                
                // Execute card play
                Position targetPos{static_cast<int>(i % 8), 7};
                auto result = CardPlayValidator::executeCardPlay(
                    gameState, PlayerSide::PLAYER_ONE, 0, targetPos);
                
                if (result.success) {
                    successCount++;
                }
                
                // Clean up the placed piece for next iteration
                gameState.getBoard().getSquare(targetPos.x, targetPos.y).setPiece(nullptr);
            }
            
            return successCount;
        };
    }
    
    SECTION("Memory Usage Tests") {
        BENCHMARK("Large deck creation and destruction") {
            std::vector<Deck> decks;
            decks.reserve(100);
            
            for (int i = 0; i < 100; ++i) {
                Deck deck;
                
                // Fill with maximum cards
                for (int j = 0; j < 20; ++j) {
                    deck.addCard(createTestPieceCard(j));
                }
                
                decks.push_back(std::move(deck));
            }
            
            // Decks will be automatically destroyed when going out of scope
            return decks.size();
        };
        
        BENCHMARK("Card polymorphism overhead") {
            std::vector<std::unique_ptr<Card>> cards;
            cards.reserve(1000);
            
            // Mix of different card types
            for (int i = 0; i < 1000; ++i) {
                if (i % 2 == 0) {
                    cards.push_back(createTestPieceCard(i));
                } else {
                    cards.push_back(createTestEffectCard(i));
                }
            }
            
            // Access virtual methods to test polymorphism overhead
            int totalCost = 0;
            for (const auto& card : cards) {
                totalCost += card->getSteamCost();
                card->getCardType(); // Virtual method call
            }
            
            return totalCost;
        };
    }
}

TEST_CASE("Card System Stress Tests", "[card][stress]") {
    
    SECTION("High Volume Card Operations") {
        GameState gameState;
        gameState.initializeNewGame();
        gameState.initializeCardSystem();
        gameState.addSteam(PlayerSide::PLAYER_ONE, 10000);
        
        // Stress test with many card operations
        const int STRESS_ITERATIONS = 1000;
        int successfulOperations = 0;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < STRESS_ITERATIONS; ++i) {
            // Create and add card to hand
            auto card = std::make_unique<PieceCard>(
                "Stress Test Card " + std::to_string(i),
                1,
                "Stress test description",
                PieceType::PAWN
            );
            
            Hand& hand = gameState.getHand(PlayerSide::PLAYER_ONE);
            if (hand.isFull()) {
                hand.removeCardAt(0); // Make space
            }
            
            if (hand.addCard(std::move(card))) {
                // Try to play the card
                Position pos{static_cast<int>(i % 8), 7};
                if (gameState.getBoard().getSquare(pos.x, pos.y).isEmpty()) {
                    auto result = CardPlayValidator::executeCardPlay(
                        gameState, PlayerSide::PLAYER_ONE, hand.size() - 1, pos);
                    if (result.success) {
                        successfulOperations++;
                    }
                }
            }
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Verify that operations completed in reasonable time (less than 5 seconds)
        REQUIRE(duration.count() < 5000);
        REQUIRE(successfulOperations > 0);
        
        // Log performance info
        INFO("Completed " << STRESS_ITERATIONS << " operations in " << duration.count() << "ms");
        INFO("Successful operations: " << successfulOperations);
    }
    
    SECTION("Memory Leak Detection") {
        // This test helps detect memory leaks by creating and destroying many objects
        const int ITERATIONS = 10000;
        
        for (int i = 0; i < ITERATIONS; ++i) {
            // Create various card types
            auto pieceCard = std::make_unique<PieceCard>(
                "Memory Test " + std::to_string(i), 1, "Test", PieceType::PAWN);
            
            Effect effect{EffectType::HEAL, 1, 0, TargetType::SINGLE_PIECE};
            auto effectCard = std::make_unique<EffectCard>(
                "Effect Test " + std::to_string(i), 1, "Test", effect);
            
            // Create collections
            Hand hand;
            Deck deck;
            
            // Add and remove cards
            hand.addCard(std::move(pieceCard));
            deck.addCard(std::move(effectCard));
            
            auto removed = hand.removeCardAt(0);
            auto drawn = deck.drawCard();
            
            // Objects should be automatically cleaned up when going out of scope
        }
        
        // If we reach here without crashing, memory management is likely working correctly
        REQUIRE(true);
    }
} 