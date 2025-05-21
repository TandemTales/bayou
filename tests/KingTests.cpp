#include <catch2/catch_test_macros.hpp>
#include "King.h"
#include "GameBoard.h"
#include "Square.h"

using namespace BayouBonanza;

TEST_CASE("King piece functionality", "[king]") {
    GameBoard board;
    
    SECTION("Construction and attributes") {
        King king(PlayerSide::PLAYER_ONE);
        
        REQUIRE(king.getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(king.getAttack() == 3);
        REQUIRE(king.getHealth() == 10);
        REQUIRE(king.getTypeName() == "King");
    }
    
    SECTION("Valid moves - empty board") {
        King king(PlayerSide::PLAYER_ONE);
        Position pos(3, 3);
        king.setPosition(pos);
        
        // Place king on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<King>(king));
        
        // King should be able to move one square in any direction
        REQUIRE(king.isValidMove(board, Position(2, 2))); // Diagonal up-left
        REQUIRE(king.isValidMove(board, Position(3, 2))); // Up
        REQUIRE(king.isValidMove(board, Position(4, 2))); // Diagonal up-right
        REQUIRE(king.isValidMove(board, Position(2, 3))); // Left
        REQUIRE(king.isValidMove(board, Position(4, 3))); // Right
        REQUIRE(king.isValidMove(board, Position(2, 4))); // Diagonal down-left
        REQUIRE(king.isValidMove(board, Position(3, 4))); // Down
        REQUIRE(king.isValidMove(board, Position(4, 4))); // Diagonal down-right
        
        // King should not be able to move more than one square
        REQUIRE_FALSE(king.isValidMove(board, Position(1, 1))); // Two squares diagonally
        REQUIRE_FALSE(king.isValidMove(board, Position(3, 1))); // Two squares up
        REQUIRE_FALSE(king.isValidMove(board, Position(5, 3))); // Two squares right
        
        // King should not be able to move to its own position
        REQUIRE_FALSE(king.isValidMove(board, Position(3, 3)));
        
        // Verify getValidMoves returns correct positions
        std::vector<Position> validMoves = king.getValidMoves(board);
        REQUIRE(validMoves.size() == 8); // 8 surrounding squares
    }
    
    SECTION("Valid moves with friendly pieces blocking") {
        King king(PlayerSide::PLAYER_ONE);
        Position kingPos(3, 3);
        king.setPosition(kingPos);
        
        // Place king on board
        board.getSquare(kingPos.x, kingPos.y).setPiece(std::make_shared<King>(king));
        
        // Place a friendly piece (same side) at position (2, 2)
        King friendlyPiece(PlayerSide::PLAYER_ONE);
        Position friendlyPos(2, 2);
        friendlyPiece.setPosition(friendlyPos);
        board.getSquare(friendlyPos.x, friendlyPos.y).setPiece(std::make_shared<King>(friendlyPiece));
        
        // King should not be able to move to the friendly piece's position
        REQUIRE_FALSE(king.isValidMove(board, friendlyPos));
        
        // King should still be able to move to other positions
        REQUIRE(king.isValidMove(board, Position(3, 2))); // Up
        REQUIRE(king.isValidMove(board, Position(4, 2))); // Diagonal up-right
        
        // Verify getValidMoves returns correct positions (7 instead of 8)
        std::vector<Position> validMoves = king.getValidMoves(board);
        REQUIRE(validMoves.size() == 7); // 7 surrounding squares (minus the blocked one)
    }
    
    SECTION("Valid moves with enemy pieces (capture)") {
        King king(PlayerSide::PLAYER_ONE);
        Position kingPos(3, 3);
        king.setPosition(kingPos);
        
        // Place king on board
        board.getSquare(kingPos.x, kingPos.y).setPiece(std::make_shared<King>(king));
        
        // Place an enemy piece at position (2, 2)
        King enemyPiece(PlayerSide::PLAYER_TWO);
        Position enemyPos(2, 2);
        enemyPiece.setPosition(enemyPos);
        board.getSquare(enemyPos.x, enemyPos.y).setPiece(std::make_shared<King>(enemyPiece));
        
        // King should be able to move to (capture) the enemy piece's position
        REQUIRE(king.isValidMove(board, enemyPos));
        
        // Verify getValidMoves includes the enemy position
        std::vector<Position> validMoves = king.getValidMoves(board);
        REQUIRE(validMoves.size() == 8); // All 8 surrounding squares
        
        // Check that the enemy position is included in valid moves
        bool foundEnemyPos = false;
        for (const Position& pos : validMoves) {
            if (pos == enemyPos) {
                foundEnemyPos = true;
                break;
            }
        }
        REQUIRE(foundEnemyPos);
    }
    
    SECTION("Board boundary checking") {
        // Place king at the edge of the board
        King king(PlayerSide::PLAYER_ONE);
        Position edgePos(0, 0); // Top-left corner
        king.setPosition(edgePos);
        
        // Place king on board
        board.getSquare(edgePos.x, edgePos.y).setPiece(std::make_shared<King>(king));
        
        // King should not be able to move out of bounds
        REQUIRE_FALSE(king.isValidMove(board, Position(-1, 0)));  // Out of bounds left
        REQUIRE_FALSE(king.isValidMove(board, Position(0, -1)));  // Out of bounds up
        REQUIRE_FALSE(king.isValidMove(board, Position(-1, -1))); // Out of bounds diagonally
        
        // King should be able to move within bounds
        REQUIRE(king.isValidMove(board, Position(1, 0))); // Right
        REQUIRE(king.isValidMove(board, Position(0, 1))); // Down
        REQUIRE(king.isValidMove(board, Position(1, 1))); // Diagonal down-right
        
        // Verify getValidMoves only includes in-bounds positions
        std::vector<Position> validMoves = king.getValidMoves(board);
        REQUIRE(validMoves.size() == 3); // Only 3 valid moves from corner
    }
}
