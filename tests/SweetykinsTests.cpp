#include <catch2/catch_test_macros.hpp>
#include "Sweetykins.h"
#include "GameBoard.h"
#include "Square.h"

using namespace BayouBonanza;

TEST_CASE("Sweetykins piece functionality", "[sweetykins]") {
    GameBoard board;
    
    SECTION("Basic Sweetykins properties") {
        Sweetykins sweetykins(PlayerSide::PLAYER_ONE);
        
        REQUIRE(sweetykins.getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(sweetykins.getAttack() == 4);
        REQUIRE(sweetykins.getHealth() == 12);
        REQUIRE(sweetykins.getTypeName() == "Sweetykins");
    }
    
    SECTION("Sweetykins movement validation") {
        Sweetykins sweetykins(PlayerSide::PLAYER_ONE);
        Position pos(3, 3);
        sweetykins.setPosition(pos);
        
        // Place sweetykins on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<Sweetykins>(sweetykins));
        
        // Sweetykins should be able to move horizontally and vertically
        REQUIRE(sweetykins.isValidMove(board, Position(3, 0))); // Up 3 squares
        REQUIRE(sweetykins.isValidMove(board, Position(3, 7))); // Down 4 squares
        REQUIRE(sweetykins.isValidMove(board, Position(0, 3))); // Left 3 squares
        REQUIRE(sweetykins.isValidMove(board, Position(7, 3))); // Right 4 squares
        
        // Sweetykins should not be able to move diagonally
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(0, 0))); // Diagonally up-left
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(6, 0))); // Diagonally up-right
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(1, 5))); // Diagonally down-left
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(5, 5))); // Diagonally down-right
        
        // Sweetykins should not be able to move to its own position
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(3, 3)));
        
        // Test valid moves generation
        std::vector<Position> validMoves = sweetykins.getValidMoves(board);
        REQUIRE(validMoves.size() > 0);
    }
    
    SECTION("Sweetykins movement with obstacles") {
        Sweetykins sweetykins(PlayerSide::PLAYER_ONE);
        Position sweetykinsPos(3, 3);
        sweetykins.setPosition(sweetykinsPos);
        
        // Place sweetykins on board
        board.getSquare(sweetykinsPos.x, sweetykinsPos.y).setPiece(std::make_shared<Sweetykins>(sweetykins));
        
        // Place a friendly piece in the path
        Sweetykins friendlyPiece(PlayerSide::PLAYER_ONE);
        Position friendlyPos(3, 1);
        friendlyPiece.setPosition(friendlyPos);
        board.getSquare(friendlyPos.x, friendlyPos.y).setPiece(std::make_shared<Sweetykins>(friendlyPiece));
        
        // Sweetykins should not be able to move to or past the friendly piece's position
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(3, 1))); // Blocked by friendly
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(3, 0))); // Beyond blocked path
        
        // Sweetykins should still be able to move to unblocked positions
        REQUIRE(sweetykins.isValidMove(board, Position(3, 2))); // One square up (before blocker)
        REQUIRE(sweetykins.isValidMove(board, Position(6, 3))); // Right three squares
        
        // Place an enemy piece in another direction
        Sweetykins enemyPiece(PlayerSide::PLAYER_TWO);
        Position enemyPos(6, 3);
        enemyPiece.setPosition(enemyPos);
        board.getSquare(enemyPos.x, enemyPos.y).setPiece(std::make_shared<Sweetykins>(enemyPiece));
        
        // Sweetykins should be able to move to (capture) the enemy piece's position
        REQUIRE(sweetykins.isValidMove(board, enemyPos));
        
        // But not beyond the enemy piece
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(7, 3)));
        
        // Test valid moves with obstacles
        std::vector<Position> validMoves = sweetykins.getValidMoves(board);
        
        // Should not include blocked positions
        bool foundBlockedPosition = false;
        for (const auto& move : validMoves) {
            if (move.x == 3 && move.y == 1) {
                foundBlockedPosition = true;
                break;
            }
        }
        REQUIRE_FALSE(foundBlockedPosition);
        
        // Should not include positions beyond blocked path
        bool foundBeyondBlocked = false;
        for (const auto& move : validMoves) {
            if (move.x == 3 && move.y == 0) {
                foundBeyondBlocked = true;
                break;
            }
        }
        REQUIRE_FALSE(foundBeyondBlocked);
        
        // Should include capturable enemy position
        bool foundEnemyPosition = false;
        for (const auto& move : validMoves) {
            if (move.x == 6 && move.y == 3) {
                foundEnemyPosition = true;
                break;
            }
        }
        REQUIRE(foundEnemyPosition);
    }
    
    SECTION("Sweetykins boundary testing") {
        // Place sweetykins at the edge of the board
        Sweetykins sweetykins(PlayerSide::PLAYER_ONE);
        Position edgePos(0, 0);
        sweetykins.setPosition(edgePos);
        
        // Place sweetykins on board
        board.getSquare(edgePos.x, edgePos.y).setPiece(std::make_shared<Sweetykins>(sweetykins));
        
        // Sweetykins should not be able to move out of bounds
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(-1, 0)));  // Out of bounds left
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(0, -1)));  // Out of bounds up
        
        // Sweetykins should be able to move within bounds
        REQUIRE(sweetykins.isValidMove(board, Position(7, 0))); // All the way right
        REQUIRE(sweetykins.isValidMove(board, Position(0, 7))); // All the way down
        
        // Sweetykins should not be able to move diagonally
        REQUIRE_FALSE(sweetykins.isValidMove(board, Position(1, 1))); // Diagonal
    }
    
    SECTION("Sweetykins influence area") {
        Sweetykins sweetykins(PlayerSide::PLAYER_ONE);
        Position sweetykinsPos(3, 3);
        sweetykins.setPosition(sweetykinsPos);
        
        // Place sweetykins on board
        board.getSquare(sweetykinsPos.x, sweetykinsPos.y).setPiece(std::make_shared<Sweetykins>(sweetykins));
        
        // For the Sweetykins, the influence area should match the valid moves
        std::vector<Position> validMoves = sweetykins.getValidMoves(board);
        std::vector<Position> influenceArea = sweetykins.getInfluenceArea(board);
        
        // The influence area should be the same as valid moves for a Sweetykins
        REQUIRE(validMoves.size() == influenceArea.size());
        
        // Check that all valid moves are in the influence area
        for (const auto& move : validMoves) {
            bool found = false;
            for (const auto& influence : influenceArea) {
                if (move.x == influence.x && move.y == influence.y) {
                    found = true;
                    break;
                }
            }
            REQUIRE(found);
        }
    }
}
