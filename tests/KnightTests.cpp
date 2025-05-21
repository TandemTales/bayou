#include <catch2/catch_test_macros.hpp>
#include "Knight.h"
#include "GameBoard.h"
#include "Square.h"

using namespace BayouBonanza;

TEST_CASE("Knight piece functionality", "[knight]") {
    GameBoard board;
    
    SECTION("Construction and attributes") {
        Knight knight(PlayerSide::PLAYER_ONE);
        
        REQUIRE(knight.getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(knight.getAttack() == 3);
        REQUIRE(knight.getHealth() == 7);
        REQUIRE(knight.getTypeName() == "Knight");
    }
    
    SECTION("Valid moves - empty board") {
        Knight knight(PlayerSide::PLAYER_ONE);
        Position pos(3, 3);
        knight.setPosition(pos);
        
        // Place knight on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<Knight>(knight));
        
        // Knight should be able to move in L-shapes (2 squares in one direction and 1 square perpendicular)
        REQUIRE(knight.isValidMove(board, Position(1, 2))); // 2 left, 1 up
        REQUIRE(knight.isValidMove(board, Position(2, 1))); // 1 left, 2 up
        REQUIRE(knight.isValidMove(board, Position(5, 2))); // 2 right, 1 up
        REQUIRE(knight.isValidMove(board, Position(4, 1))); // 1 right, 2 up
        REQUIRE(knight.isValidMove(board, Position(1, 4))); // 2 left, 1 down
        REQUIRE(knight.isValidMove(board, Position(2, 5))); // 1 left, 2 down
        REQUIRE(knight.isValidMove(board, Position(5, 4))); // 2 right, 1 down
        REQUIRE(knight.isValidMove(board, Position(4, 5))); // 1 right, 2 down
        
        // Knight should not be able to move in other patterns
        REQUIRE_FALSE(knight.isValidMove(board, Position(2, 2))); // Diagonal
        REQUIRE_FALSE(knight.isValidMove(board, Position(3, 5))); // Straight vertical
        REQUIRE_FALSE(knight.isValidMove(board, Position(6, 3))); // Straight horizontal
        
        // Knight should not be able to move to its own position
        REQUIRE_FALSE(knight.isValidMove(board, Position(3, 3)));
        
        // Verify getValidMoves returns correct positions
        std::vector<Position> validMoves = knight.getValidMoves(board);
        REQUIRE(validMoves.size() == 8); // 8 possible L-shaped moves from center
    }
    
    SECTION("Valid moves with pieces present") {
        Knight knight(PlayerSide::PLAYER_ONE);
        Position knightPos(3, 3);
        knight.setPosition(knightPos);
        
        // Place knight on board
        board.getSquare(knightPos.x, knightPos.y).setPiece(std::make_shared<Knight>(knight));
        
        // Place a friendly piece at position (1, 2)
        Knight friendlyPiece(PlayerSide::PLAYER_ONE);
        Position friendlyPos(1, 2);
        friendlyPiece.setPosition(friendlyPos);
        board.getSquare(friendlyPos.x, friendlyPos.y).setPiece(std::make_shared<Knight>(friendlyPiece));
        
        // Knight should not be able to move to the friendly piece's position
        REQUIRE_FALSE(knight.isValidMove(board, friendlyPos));
        
        // Place pieces in the path that a knight would normally jump over
        // This should not affect the knight's movement
        Knight blockingPiece1(PlayerSide::PLAYER_ONE);
        Position blockingPos1(2, 2); // Diagonal from knight
        blockingPiece1.setPosition(blockingPos1);
        board.getSquare(blockingPos1.x, blockingPos1.y).setPiece(std::make_shared<Knight>(blockingPiece1));
        
        Knight blockingPiece2(PlayerSide::PLAYER_TWO);
        Position blockingPos2(2, 3); // Adjacent to knight
        blockingPiece2.setPosition(blockingPos2);
        board.getSquare(blockingPos2.x, blockingPos2.y).setPiece(std::make_shared<Knight>(blockingPiece2));
        
        // Knight should still be able to jump over these pieces to reach (1, 4)
        REQUIRE(knight.isValidMove(board, Position(1, 4)));
        
        // Place an enemy piece at position (5, 4)
        Knight enemyPiece(PlayerSide::PLAYER_TWO);
        Position enemyPos(5, 4);
        enemyPiece.setPosition(enemyPos);
        board.getSquare(enemyPos.x, enemyPos.y).setPiece(std::make_shared<Knight>(enemyPiece));
        
        // Knight should be able to move to (capture) the enemy piece's position
        REQUIRE(knight.isValidMove(board, enemyPos));
        
        // Verify getValidMoves excludes the friendly piece position
        std::vector<Position> validMoves = knight.getValidMoves(board);
        REQUIRE(validMoves.size() == 7); // 7 possible moves (8 normal - 1 blocked by friendly)
        
        // Check that the enemy position is included in valid moves
        bool foundEnemyPos = false;
        for (const Position& pos : validMoves) {
            if (pos == enemyPos) {
                foundEnemyPos = true;
                break;
            }
        }
        REQUIRE(foundEnemyPos);
        
        // Check that the friendly position is not included in valid moves
        bool foundFriendlyPos = false;
        for (const Position& pos : validMoves) {
            if (pos == friendlyPos) {
                foundFriendlyPos = true;
                break;
            }
        }
        REQUIRE_FALSE(foundFriendlyPos);
    }
    
    SECTION("Board boundary checking") {
        // Place knight at the edge of the board
        Knight knight(PlayerSide::PLAYER_ONE);
        Position edgePos(0, 0); // Top-left corner
        knight.setPosition(edgePos);
        
        // Place knight on board
        board.getSquare(edgePos.x, edgePos.y).setPiece(std::make_shared<Knight>(knight));
        
        // Knight should not be able to move out of bounds
        REQUIRE_FALSE(knight.isValidMove(board, Position(-1, 2)));  // 2 up, 1 left (out of bounds)
        REQUIRE_FALSE(knight.isValidMove(board, Position(-2, 1)));  // 1 up, 2 left (out of bounds)
        
        // Knight should be able to move within bounds
        REQUIRE(knight.isValidMove(board, Position(1, 2))); // 1 right, 2 down
        REQUIRE(knight.isValidMove(board, Position(2, 1))); // 2 right, 1 down
        
        // Verify getValidMoves only includes in-bounds positions
        std::vector<Position> validMoves = knight.getValidMoves(board);
        REQUIRE(validMoves.size() == 2); // Only 2 valid moves from corner
    }
    
    SECTION("Influence area matches valid moves") {
        Knight knight(PlayerSide::PLAYER_ONE);
        Position knightPos(3, 3);
        knight.setPosition(knightPos);
        
        // Place knight on board
        board.getSquare(knightPos.x, knightPos.y).setPiece(std::make_shared<Knight>(knight));
        
        // For the Knight, the influence area should match the valid moves
        std::vector<Position> validMoves = knight.getValidMoves(board);
        std::vector<Position> influenceArea = knight.getInfluenceArea(board);
        
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
