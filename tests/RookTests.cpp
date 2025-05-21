#include <catch2/catch_test_macros.hpp>
#include "Rook.h"
#include "GameBoard.h"
#include "Square.h"

using namespace BayouBonanza;

TEST_CASE("Rook piece functionality", "[rook]") {
    GameBoard board;
    
    SECTION("Construction and attributes") {
        Rook rook(PlayerSide::PLAYER_ONE);
        
        REQUIRE(rook.getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(rook.getAttack() == 4);
        REQUIRE(rook.getHealth() == 12);
        REQUIRE(rook.getTypeName() == "Rook");
    }
    
    SECTION("Valid moves - empty board") {
        Rook rook(PlayerSide::PLAYER_ONE);
        Position pos(3, 3);
        rook.setPosition(pos);
        
        // Place rook on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<Rook>(rook));
        
        // Rook should be able to move horizontally and vertically
        REQUIRE(rook.isValidMove(board, Position(3, 0))); // Up 3 squares
        REQUIRE(rook.isValidMove(board, Position(3, 7))); // Down 4 squares
        REQUIRE(rook.isValidMove(board, Position(0, 3))); // Left 3 squares
        REQUIRE(rook.isValidMove(board, Position(7, 3))); // Right 4 squares
        
        // Rook should not be able to move diagonally
        REQUIRE_FALSE(rook.isValidMove(board, Position(0, 0))); // Diagonally up-left
        REQUIRE_FALSE(rook.isValidMove(board, Position(6, 0))); // Diagonally up-right
        REQUIRE_FALSE(rook.isValidMove(board, Position(1, 5))); // Diagonally down-left
        REQUIRE_FALSE(rook.isValidMove(board, Position(5, 5))); // Diagonally down-right
        
        // Rook should not be able to move to its own position
        REQUIRE_FALSE(rook.isValidMove(board, Position(3, 3)));
        
        // Verify getValidMoves returns correct positions
        std::vector<Position> validMoves = rook.getValidMoves(board);
        REQUIRE(validMoves.size() == 14); // 7 in horizontal + 7 in vertical directions
    }
    
    SECTION("Valid moves with pieces blocking") {
        Rook rook(PlayerSide::PLAYER_ONE);
        Position rookPos(3, 3);
        rook.setPosition(rookPos);
        
        // Place rook on board
        board.getSquare(rookPos.x, rookPos.y).setPiece(std::make_shared<Rook>(rook));
        
        // Place a friendly piece at position (3, 1) - blocking upward movement
        Rook friendlyPiece(PlayerSide::PLAYER_ONE);
        Position friendlyPos(3, 1);
        friendlyPiece.setPosition(friendlyPos);
        board.getSquare(friendlyPos.x, friendlyPos.y).setPiece(std::make_shared<Rook>(friendlyPiece));
        
        // Rook should not be able to move to or past the friendly piece's position
        REQUIRE_FALSE(rook.isValidMove(board, Position(3, 1))); // Blocked by friendly
        REQUIRE_FALSE(rook.isValidMove(board, Position(3, 0))); // Beyond blocked path
        
        // Rook should still be able to move to unblocked positions
        REQUIRE(rook.isValidMove(board, Position(3, 2))); // One square up (before blocker)
        REQUIRE(rook.isValidMove(board, Position(6, 3))); // Right three squares
        
        // Place an enemy piece at position (6, 3) - allowing capture
        Rook enemyPiece(PlayerSide::PLAYER_TWO);
        Position enemyPos(6, 3);
        enemyPiece.setPosition(enemyPos);
        board.getSquare(enemyPos.x, enemyPos.y).setPiece(std::make_shared<Rook>(enemyPiece));
        
        // Rook should be able to move to (capture) the enemy piece's position
        REQUIRE(rook.isValidMove(board, enemyPos));
        
        // But should not be able to move past it
        REQUIRE_FALSE(rook.isValidMove(board, Position(7, 3)));
        
        // Verify getValidMoves excludes blocked positions
        std::vector<Position> validMoves = rook.getValidMoves(board);
        
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
            // Check if any position is beyond the enemy piece at (6, 3)
            if (pos.x > 6 && pos.y == 3) {
                foundBeyondBlocked = true;
                break;
            }
        }
        REQUIRE_FALSE(foundBeyondBlocked);
    }
    
    SECTION("Board boundary checking") {
        // Place rook at the edge of the board
        Rook rook(PlayerSide::PLAYER_ONE);
        Position edgePos(0, 0); // Top-left corner
        rook.setPosition(edgePos);
        
        // Place rook on board
        board.getSquare(edgePos.x, edgePos.y).setPiece(std::make_shared<Rook>(rook));
        
        // Rook should not be able to move out of bounds
        REQUIRE_FALSE(rook.isValidMove(board, Position(-1, 0)));  // Out of bounds left
        REQUIRE_FALSE(rook.isValidMove(board, Position(0, -1)));  // Out of bounds up
        
        // Rook should be able to move within bounds
        REQUIRE(rook.isValidMove(board, Position(7, 0))); // All the way right
        REQUIRE(rook.isValidMove(board, Position(0, 7))); // All the way down
        
        // Rook should not be able to move diagonally
        REQUIRE_FALSE(rook.isValidMove(board, Position(1, 1))); // Diagonal
    }
    
    SECTION("Influence area matches valid moves") {
        Rook rook(PlayerSide::PLAYER_ONE);
        Position rookPos(3, 3);
        rook.setPosition(rookPos);
        
        // Place rook on board
        board.getSquare(rookPos.x, rookPos.y).setPiece(std::make_shared<Rook>(rook));
        
        // For the Rook, the influence area should match the valid moves
        std::vector<Position> validMoves = rook.getValidMoves(board);
        std::vector<Position> influenceArea = rook.getInfluenceArea(board);
        
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
