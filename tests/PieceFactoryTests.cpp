#include <catch2/catch_test_macros.hpp>
#include "PieceFactory.h"
#include "King.h"
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"
#include "Knight.h"
#include "Pawn.h"

using namespace BayouBonanza;

TEST_CASE("PieceFactory functionality", "[piecefactory]") {
    
    SECTION("Create King") {
        auto piece = PieceFactory::createKing(PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece->getTypeName() == "King");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 3);
        REQUIRE(piece->getHealth() == 10);
    }
    
    SECTION("Create Queen") {
        auto piece = PieceFactory::createQueen(PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece->getTypeName() == "Queen");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 5);
        REQUIRE(piece->getHealth() == 8);
    }
    
    SECTION("Create Rook") {
        auto piece = PieceFactory::createRook(PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece->getTypeName() == "Rook");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 4);
        REQUIRE(piece->getHealth() == 12);
    }
    
    SECTION("Create Bishop") {
        auto piece = PieceFactory::createBishop(PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece->getTypeName() == "Bishop");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 4);
        REQUIRE(piece->getHealth() == 8);
    }
    
    SECTION("Create Knight") {
        auto piece = PieceFactory::createKnight(PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece->getTypeName() == "Knight");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 3);
        REQUIRE(piece->getHealth() == 7);
    }
    
    SECTION("Create Pawn") {
        auto piece = PieceFactory::createPawn(PlayerSide::PLAYER_ONE);
        
        REQUIRE(piece->getTypeName() == "Pawn");
        REQUIRE(piece->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece->getAttack() == 2);
        REQUIRE(piece->getHealth() == 5);
    }
    
    SECTION("Create piece by type name") {
        // Test creating each piece type by name
        auto king = PieceFactory::createPieceByType("King", PlayerSide::PLAYER_ONE);
        REQUIRE(king->getTypeName() == "King");
        
        auto queen = PieceFactory::createPieceByType("Queen", PlayerSide::PLAYER_ONE);
        REQUIRE(queen->getTypeName() == "Queen");
        
        auto rook = PieceFactory::createPieceByType("Rook", PlayerSide::PLAYER_ONE);
        REQUIRE(rook->getTypeName() == "Rook");
        
        auto bishop = PieceFactory::createPieceByType("Bishop", PlayerSide::PLAYER_ONE);
        REQUIRE(bishop->getTypeName() == "Bishop");
        
        auto knight = PieceFactory::createPieceByType("Knight", PlayerSide::PLAYER_ONE);
        REQUIRE(knight->getTypeName() == "Knight");
        
        auto pawn = PieceFactory::createPieceByType("Pawn", PlayerSide::PLAYER_ONE);
        REQUIRE(pawn->getTypeName() == "Pawn");
        
        // Test with invalid piece type name
        auto invalidPiece = PieceFactory::createPieceByType("InvalidPiece", PlayerSide::PLAYER_ONE);
        REQUIRE(invalidPiece == nullptr);
    }
    
    SECTION("Create piece for different player sides") {
        auto piece1 = PieceFactory::createKing(PlayerSide::PLAYER_ONE);
        REQUIRE(piece1->getSide() == PlayerSide::PLAYER_ONE);
        
        auto piece2 = PieceFactory::createKing(PlayerSide::PLAYER_TWO);
        REQUIRE(piece2->getSide() == PlayerSide::PLAYER_TWO);
    }
}
