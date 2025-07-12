#include <catch2/catch_test_macros.hpp>
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"
#include "PlayerSide.h"
// Removed includes for King.h, Queen.h, etc. - using data-driven approach with PieceFactory

using namespace BayouBonanza;

TEST_CASE("PieceFactory functionality", "[piecefactory]") {
    PieceDefinitionManager pdm;
    PieceFactory factory(pdm);
    
    // Load piece definitions
    bool loaded = pdm.loadDefinitions("assets/data/cards.json"); // Try relative path from test executable directory first
    if (!loaded) {
        loaded = pdm.loadDefinitions("../../assets/data/cards.json"); // Fallback to original path
    }
    REQUIRE(loaded); // Crucial for tests to run
    
    SECTION("Create TinkeringTom") {
        const auto* stats = pdm.getPieceStats("TinkeringTom");
        REQUIRE(stats != nullptr);
        auto piece = factory.createPiece("TinkeringTom", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "TinkeringTom");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == stats->attack);
        REQUIRE(piece->getHealth() == stats->health);
    }
    
    SECTION("Create ScarlettGlumpkin") {
        const auto* stats = pdm.getPieceStats("ScarlettGlumpkin");
        REQUIRE(stats != nullptr);
        auto piece = factory.createPiece("ScarlettGlumpkin", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "ScarlettGlumpkin");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == stats->attack);
        REQUIRE(piece->getHealth() == stats->health);
    }
    
    SECTION("Create Sweetykins") {
        const auto* stats = pdm.getPieceStats("Sweetykins");
        REQUIRE(stats != nullptr);
        auto piece = factory.createPiece("Sweetykins", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Sweetykins");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == stats->attack);
        REQUIRE(piece->getHealth() == stats->health);
    }
    
    SECTION("Create Sidewinder") {
        const auto* stats = pdm.getPieceStats("Sidewinder");
        REQUIRE(stats != nullptr);
        auto piece = factory.createPiece("Sidewinder", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Sidewinder");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == stats->attack);
        REQUIRE(piece->getHealth() == stats->health);
    }
    
    SECTION("Create Automatick") {
        const auto* stats = pdm.getPieceStats("Automatick");
        REQUIRE(stats != nullptr);
        auto piece = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Automatick");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == stats->attack);
        REQUIRE(piece->getHealth() == stats->health);
    }
    
    SECTION("Create Sentroid") {
        const auto* stats = pdm.getPieceStats("Sentroid");
        REQUIRE(stats != nullptr);
        auto piece = factory.createPiece("Sentroid", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Sentroid");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == stats->attack);
        REQUIRE(piece->getHealth() == stats->health);
    }
    
    SECTION("Create Rustbucket") {
        const auto* stats = pdm.getPieceStats("Rustbucket");
        REQUIRE(stats != nullptr);
        auto piece = factory.createPiece("Rustbucket", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Rustbucket");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == stats->attack);
        REQUIRE(piece->getHealth() == stats->health);
    }
    
    SECTION("Create piece for different player sides") {
        auto piece1 = factory.createPiece("TinkeringTom", PlayerSide::PLAYER_ONE);
        REQUIRE(piece1 != nullptr);
        REQUIRE(piece1->getSide() == PlayerSide::PLAYER_ONE);
        
        auto piece2 = factory.createPiece("TinkeringTom", PlayerSide::PLAYER_TWO);
        REQUIRE(piece2 != nullptr);
        REQUIRE(piece2->getSide() == PlayerSide::PLAYER_TWO);
    }
    
    SECTION("Create invalid piece type") {
        auto invalidPiece = factory.createPiece("InvalidPiece", PlayerSide::PLAYER_ONE);
        REQUIRE(invalidPiece == nullptr);
    }
}
