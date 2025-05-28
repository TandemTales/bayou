#include <catch2/catch_test_macros.hpp>
#include "Piece.h"
#include "GameBoard.h"
#include "Square.h"

using namespace BayouBonanza;

// Test class that implements the abstract Piece
class TestPiece : public Piece {
public:
    TestPiece(PlayerSide side, int attack, int health) : Piece(side, attack, health) {}
    
    bool isValidMove(const GameBoard& board, const Position& target) const override {
        return true; // Always valid for testing
    }
    
    std::vector<Position> getValidMoves(const GameBoard& board) const override {
        return std::vector<Position>{}; // Empty for testing
    }
    
    std::string getTypeName() const override {
        return "TestPiece";
    }

    std::string getSymbol() const override {
        return "T"; // Dummy symbol for testing
    }
    
    PieceType getPieceType() const override {
        return PieceType::PAWN; // Dummy type for testing
    }
};

TEST_CASE("Piece base class functionality", "[piece]") {
    GameBoard board;
    
    SECTION("Construction and attributes") {
        TestPiece piece(PlayerSide::PLAYER_ONE, 5, 10);
        
        REQUIRE(piece.getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(piece.getAttack() == 5);
        REQUIRE(piece.getHealth() == 10);
        REQUIRE(piece.getTypeName() == "TestPiece");
    }
    
    SECTION("Position setting and getting") {
        TestPiece piece(PlayerSide::PLAYER_ONE, 5, 10);
        Position pos(3, 4);
        
        piece.setPosition(pos);
        Position resultPos = piece.getPosition();
        
        REQUIRE(resultPos.x == 3);
        REQUIRE(resultPos.y == 4);
    }
    
    SECTION("Health manipulation") {
        TestPiece piece(PlayerSide::PLAYER_ONE, 5, 10);
        
        // Test setHealth
        piece.setHealth(8);
        REQUIRE(piece.getHealth() == 8);
        
        // Test takeDamage without death
        bool isDead = piece.takeDamage(3);
        REQUIRE(piece.getHealth() == 5);
        REQUIRE_FALSE(isDead);
        
        // Test takeDamage with death
        isDead = piece.takeDamage(6);
        REQUIRE(piece.getHealth() == -1);
        REQUIRE(isDead);
    }
    
    SECTION("Default influence area") {
        TestPiece piece(PlayerSide::PLAYER_ONE, 5, 10);
        Position pos(3, 3);
        piece.setPosition(pos);
        
        // The default influence area is the piece's own square plus adjacent squares
        std::vector<Position> influence = piece.getInfluenceArea(board);
        
        // Should have 9 squares in influence area (self + 8 adjacent)
        REQUIRE(influence.size() == 9);
        
        // Verify the piece's own position is in the influence area
        bool foundOwnPos = false;
        for (const Position& p : influence) {
            if (p == pos) {
                foundOwnPos = true;
                break;
            }
        }
        REQUIRE(foundOwnPos);
    }
}
