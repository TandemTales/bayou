#include <catch2/catch_test_macros.hpp>
#include "Queen.h"
#include "GameBoard.h"
#include "Square.h"

using namespace BayouBonanza;

TEST_CASE("Queen piece functionality", "[queen]") {
    GameBoard board;
    
    SECTION("Construction and attributes") {
        Queen queen(PlayerSide::PLAYER_ONE);
        
        REQUIRE(queen.getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(queen.getAttack() == 5);
        REQUIRE(queen.getHealth() == 8);
        REQUIRE(queen.getTypeName() == "Queen");
    }
    
    SECTION("Valid moves - empty board") {
        Queen queen(PlayerSide::PLAYER_ONE);
        Position pos(3, 3);
        queen.setPosition(pos);
        
        // Place queen on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<Queen>(queen));
        
        // Queen should be able to move in all 8 directions
        
        // Horizontal and vertical moves
        REQUIRE(queen.isValidMove(board, Position(3, 0))); // Up 3 squares
        REQUIRE(queen.isValidMove(board, Position(3, 7))); // Down 4 squares
        REQUIRE(queen.isValidMove(board, Position(0, 3))); // Left 3 squares
        REQUIRE(queen.isValidMove(board, Position(7, 3))); // Right 4 squares
        
        // Diagonal moves
        REQUIRE(queen.isValidMove(board, Position(0, 0))); // Diagonally up-left 3 squares
        REQUIRE(queen.isValidMove(board, Position(6, 0))); // Diagonally up-right 3 squares
        REQUIRE(queen.isValidMove(board, Position(0, 6))); // Diagonally down-left 3 squares
        REQUIRE(queen.isValidMove(board, Position(7, 7))); // Diagonally down-right 4 squares
        
        // Queen should not be able to move in non-straight lines
        REQUIRE_FALSE(queen.isValidMove(board, Position(2, 0))); // L-shaped move
        REQUIRE_FALSE(queen.isValidMove(board, Position(5, 2))); // Knight-like move
        
        // Queen should not be able to move to its own position
        REQUIRE_FALSE(queen.isValidMove(board, Position(3, 3)));
        
        // Verify getValidMoves returns correct positions (should be many)
        std::vector<Position> validMoves = queen.getValidMoves(board);
        REQUIRE(validMoves.size() > 20); // Queen has many possible moves from center
    }
    
    SECTION("Valid moves with pieces blocking") {
        Queen queen(PlayerSide::PLAYER_ONE);
        Position queenPos(3, 3);
        queen.setPosition(queenPos);
        
        // Place queen on board
        board.getSquare(queenPos.x, queenPos.y).setPiece(std::make_shared<Queen>(queen));
        
        // Place a friendly piece at position (3, 1) - blocking upward movement
        Queen friendlyPiece(PlayerSide::PLAYER_ONE);
        Position friendlyPos(3, 1);
        friendlyPiece.setPosition(friendlyPos);
        board.getSquare(friendlyPos.x, friendlyPos.y).setPiece(std::make_shared<Queen>(friendlyPiece));
        
        // Queen should not be able to move to or past the friendly piece's position
        REQUIRE_FALSE(queen.isValidMove(board, Position(3, 1))); // Blocked by friendly
        REQUIRE_FALSE(queen.isValidMove(board, Position(3, 0))); // Beyond blocked path
        
        // Queen should still be able to move to unblocked positions
        REQUIRE(queen.isValidMove(board, Position(3, 2))); // One square up (before blocker)
        REQUIRE(queen.isValidMove(board, Position(4, 4))); // Diagonal down-right
        
        // Place an enemy piece at position (5, 5) - allowing capture
        Queen enemyPiece(PlayerSide::PLAYER_TWO);
        Position enemyPos(5, 5);
        enemyPiece.setPosition(enemyPos);
        board.getSquare(enemyPos.x, enemyPos.y).setPiece(std::make_shared<Queen>(enemyPiece));
        
        // Queen should be able to move to (capture) the enemy piece's position
        REQUIRE(queen.isValidMove(board, enemyPos));
        
        // But should not be able to move past it
        REQUIRE_FALSE(queen.isValidMove(board, Position(6, 6)));
        
        // Verify getValidMoves excludes blocked positions
        std::vector<Position> validMoves = queen.getValidMoves(board);
        
        // Check that the enemy position is included in valid moves
        bool foundEnemyPos = false;
        for (const Position& pos : validMoves) {
            if (pos == enemyPos) {
                foundEnemyPos = true;
                break;
            }
        }
        REQUIRE(foundEnemyPos);
        
        // Check that positions beyond blockages are not included
        bool foundBeyondBlocked = false;
        for (const Position& pos : validMoves) {
            // Check if any position is beyond the friendly piece at (3, 1)
            if (pos.x == 3 && pos.y < 1) {
                foundBeyondBlocked = true;
                break;
            }
            // Check if any position is beyond the enemy piece at (5, 5)
            if (pos.x > 5 && pos.y > 5 && (pos.x - queenPos.x == pos.y - queenPos.y)) {
                foundBeyondBlocked = true;
                break;
            }
        }
        REQUIRE_FALSE(foundBeyondBlocked);
    }
    
    SECTION("Board boundary checking") {
        // Place queen at the edge of the board
        Queen queen(PlayerSide::PLAYER_ONE);
        Position edgePos(0, 0); // Top-left corner
        queen.setPosition(edgePos);
        
        // Place queen on board
        board.getSquare(edgePos.x, edgePos.y).setPiece(std::make_shared<Queen>(queen));
        
        // Queen should not be able to move out of bounds
        REQUIRE_FALSE(queen.isValidMove(board, Position(-1, 0)));   // Out of bounds left
        REQUIRE_FALSE(queen.isValidMove(board, Position(0, -1)));   // Out of bounds up
        REQUIRE_FALSE(queen.isValidMove(board, Position(-1, -1)));  // Out of bounds diagonally
        
        // Queen should be able to move within bounds
        REQUIRE(queen.isValidMove(board, Position(7, 0))); // All the way right
        REQUIRE(queen.isValidMove(board, Position(0, 7))); // All the way down
        REQUIRE(queen.isValidMove(board, Position(7, 7))); // Diagonal to bottom-right
    }
    
    SECTION("Influence area matches valid moves") {
        Queen queen(PlayerSide::PLAYER_ONE);
        Position queenPos(3, 3);
        queen.setPosition(queenPos);
        
        // Place queen on board
        board.getSquare(queenPos.x, queenPos.y).setPiece(std::make_shared<Queen>(queen));
        
        // For the Queen, the influence area should match the valid moves
        std::vector<Position> validMoves = queen.getValidMoves(board);
        std::vector<Position> influenceArea = queen.getInfluenceArea(board);
        
        REQUIRE(validMoves.size() == influenceArea.size());
        
        // Check that all valid moves are in the influence area
        for (const Position& move : validMoves) {
            bool found = false;
            for (const Position& influence : influenceArea) {
                if (move.x == influence.x && move.y == influence.y) {
                    found = true;
                    break;
                }
            }
            REQUIRE(found);
        }
    }
}
