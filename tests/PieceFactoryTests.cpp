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
    bool loaded = pdm.loadDefinitions("assets/data/pieces.json"); // Try relative path from test executable directory first
    if (!loaded) {
        loaded = pdm.loadDefinitions("../../assets/data/pieces.json"); // Fallback to original path
    }
    REQUIRE(loaded); // Crucial for tests to run
    
    SECTION("Create TinkeringTom") {
        auto piece = factory.createPiece("TinkeringTom", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "TinkeringTom");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 3);  // Based on pieces.json
        REQUIRE(piece->getHealth() == 10); // Based on pieces.json
    }
    
    SECTION("Create ScarlettGlumpkin") {
        auto piece = factory.createPiece("ScarlettGlumpkin", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "ScarlettGlumpkin");
        REQUIRE(piece->getSymbol() == "G");
        REQUIRE(piece->getAttack() == 9);
        REQUIRE(piece->getHealth() == 9);
    }
    
    SECTION("Create Sweetykins") {
        auto piece = factory.createPiece("Sweetykins", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Sweetykins");
        REQUIRE(piece->getSymbol() == "S");
        REQUIRE(piece->getAttack() == 5);  // This is correct
        REQUIRE(piece->getHealth() == 5);
    }
    
    SECTION("Create Sidewinder") {
        auto piece = factory.createPiece("Sidewinder", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Sidewinder");
        REQUIRE(piece->getSymbol() == "W");
        REQUIRE(piece->getAttack() == 3);
        REQUIRE(piece->getHealth() == 3);
    }
    
    SECTION("Create Automatick") {
        auto piece = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Automatick");
        REQUIRE(piece->getSymbol() == "A");
        REQUIRE(piece->getAttack() == 3);
        REQUIRE(piece->getHealth() == 3);  // Changed from 7 to 3
    }
    
    SECTION("Create Sentroid") {
        auto piece = factory.createPiece("Sentroid", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Sentroid");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 1); // Based on pieces.json
        REQUIRE(piece->getHealth() == 1); // Based on pieces.json
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
