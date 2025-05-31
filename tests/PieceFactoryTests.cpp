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
    
    SECTION("Create King") {
        auto piece = factory.createPiece("King", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "King");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 3);  // Based on pieces.json
        REQUIRE(piece->getHealth() == 10); // Based on pieces.json
    }
    
    SECTION("Create Queen") {
        auto piece = factory.createPiece("Queen", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Queen");
        REQUIRE(piece->getSymbol() == "Q");
        REQUIRE(piece->getAttack() == 9);  // Changed from 5 to 9
        REQUIRE(piece->getHealth() == 9);
    }
    
    SECTION("Create Rook") {
        auto piece = factory.createPiece("Rook", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Rook");
        REQUIRE(piece->getSymbol() == "R");
        REQUIRE(piece->getAttack() == 5);  // This is correct
        REQUIRE(piece->getHealth() == 5);
    }
    
    SECTION("Create Bishop") {
        auto piece = factory.createPiece("Bishop", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Bishop");
        REQUIRE(piece->getSymbol() == "B");
        REQUIRE(piece->getAttack() == 3);  // This is correct
        REQUIRE(piece->getHealth() == 3);
    }
    
    SECTION("Create Knight") {
        auto piece = factory.createPiece("Knight", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Knight");
        REQUIRE(piece->getSymbol() == "N");
        REQUIRE(piece->getAttack() == 3);
        REQUIRE(piece->getHealth() == 3);  // Changed from 7 to 3
    }
    
    SECTION("Create Pawn") {
        auto piece = factory.createPiece("Pawn", PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece != nullptr);
        REQUIRE(piece->getTypeName() == "Pawn");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 1); // Based on pieces.json
        REQUIRE(piece->getHealth() == 1); // Based on pieces.json
    }
    
    SECTION("Create piece for different player sides") {
        auto piece1 = factory.createPiece("King", PlayerSide::PLAYER_ONE);
        REQUIRE(piece1 != nullptr);
        REQUIRE(piece1->getSide() == PlayerSide::PLAYER_ONE);
        
        auto piece2 = factory.createPiece("King", PlayerSide::PLAYER_TWO);
        REQUIRE(piece2 != nullptr);
        REQUIRE(piece2->getSide() == PlayerSide::PLAYER_TWO);
    }
    
    SECTION("Create invalid piece type") {
        auto invalidPiece = factory.createPiece("InvalidPiece", PlayerSide::PLAYER_ONE);
        REQUIRE(invalidPiece == nullptr);
    }
}
