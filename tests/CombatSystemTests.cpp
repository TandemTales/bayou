#include <catch2/catch_test_macros.hpp>
#include "../include/CombatSystem.h"
#include "../include/Piece.h"
#include "../include/GameBoard.h"
#include "../include/Square.h"

using namespace BayouBonanza;

// Test class that implements the abstract Piece for testing
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
};

TEST_CASE("Combat System basic functionality", "[combat]") {
    SECTION("Damage application") {
        auto attacker = std::make_shared<TestPiece>(PlayerSide::PLAYER_ONE, 5, 10);
        auto defender = std::make_shared<TestPiece>(PlayerSide::PLAYER_TWO, 3, 8);
        
        CombatSystem::applyDamage(attacker, defender);
        
        // With attack of 5, should deal 5 damage
        REQUIRE(defender->getHealth() == 3); // 8 - 5 = 3
    }
    
    SECTION("Defeat detection") {
        auto attacker = std::make_shared<TestPiece>(PlayerSide::PLAYER_ONE, 10, 10);
        auto defender = std::make_shared<TestPiece>(PlayerSide::PLAYER_TWO, 3, 8);
        
        CombatSystem::applyDamage(attacker, defender);
        
        // With attack of 10 vs health of 8, defender should be defeated
        REQUIRE(defender->getHealth() <= 0);
    }
}

TEST_CASE("Combat System integration", "[combat]") {
    GameBoard board;
    
    // Setup pieces on the board
    auto piece1 = std::make_shared<TestPiece>(PlayerSide::PLAYER_ONE, 5, 10);
    auto piece2 = std::make_shared<TestPiece>(PlayerSide::PLAYER_TWO, 3, 6);
    
    Position pos1(2, 3);
    Position pos2(4, 5);
    
    piece1->setPosition(pos1);
    piece2->setPosition(pos2);
    
    board.getSquare(pos1.x, pos1.y).setPiece(piece1);
    board.getSquare(pos2.x, pos2.y).setPiece(piece2);
    
    SECTION("Combat validation") {
        // Pieces at different positions should be valid for combat
        REQUIRE(CombatSystem::canEngageInCombat(board, pos1, pos2));
        
        // Same piece positions should not be valid
        REQUIRE_FALSE(CombatSystem::canEngageInCombat(board, pos1, pos1));
        
        // Empty position should not be valid
        Position emptyPos(0, 0);
        REQUIRE_FALSE(CombatSystem::canEngageInCombat(board, emptyPos, pos2));
        
        // Same side pieces should not be valid for combat
        auto piece3 = std::make_shared<TestPiece>(PlayerSide::PLAYER_ONE, 4, 8);
        Position pos3(1, 1);
        piece3->setPosition(pos3);
        board.getSquare(pos3.x, pos3.y).setPiece(piece3);
        
        REQUIRE_FALSE(CombatSystem::canEngageInCombat(board, pos1, pos3));
    }
    
    SECTION("Combat resolution") {
        // Resolve combat between the two pieces
        bool success = CombatSystem::resolveCombat(board, pos1, pos2);
        
        REQUIRE(success);
        
        // Piece 2 should have taken damage
        REQUIRE(piece2->getHealth() == 1); // 6 - 5 = 1
        
        // Piece 1 should not have taken damage (no counter-attacks)
        REQUIRE(piece1->getHealth() == 10);
        
        // Do another combat to defeat piece2
        success = CombatSystem::resolveCombat(board, pos1, pos2);
        
        REQUIRE(success);
        
        // Piece 2 should now be defeated and removed from the board
        REQUIRE(board.getSquare(pos2.x, pos2.y).isEmpty());
    }
}

TEST_CASE("Piece Removal functionality", "[combat]") {
    GameBoard board;
    
    // Setup pieces on the board
    auto piece1 = std::make_shared<TestPiece>(PlayerSide::PLAYER_ONE, 5, 10);
    auto piece2 = std::make_shared<TestPiece>(PlayerSide::PLAYER_TWO, 3, -2); // Already defeated
    
    Position pos1(1, 1);
    Position pos2(2, 2);
    
    piece1->setPosition(pos1);
    piece2->setPosition(pos2);
    
    board.getSquare(pos1.x, pos1.y).setPiece(piece1);
    board.getSquare(pos2.x, pos2.y).setPiece(piece2);
    
    SECTION("Remove single defeated piece") {
        // Piece 2 is already defeated, should be removable
        bool removed = CombatSystem::checkAndRemoveDeadPiece(board, pos2);
        
        REQUIRE(removed);
        REQUIRE(board.getSquare(pos2.x, pos2.y).isEmpty());
        
        // Piece 1 is not defeated, should not be removable
        removed = CombatSystem::checkAndRemoveDeadPiece(board, pos1);
        
        REQUIRE_FALSE(removed);
        REQUIRE_FALSE(board.getSquare(pos1.x, pos1.y).isEmpty());
    }
    
    SECTION("Remove all defeated pieces") {
        // Remove all defeated pieces
        CombatSystem::checkAndRemoveDeadPieces(board);
        
        // Piece 2 should be removed
        REQUIRE(board.getSquare(pos2.x, pos2.y).isEmpty());
        
        // Piece 1 should still be on the board
        REQUIRE_FALSE(board.getSquare(pos1.x, pos1.y).isEmpty());
    }
}
