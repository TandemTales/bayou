#include <catch2/catch_test_macros.hpp>
#include "Bishop.h"
#include "GameBoard.h"
#include "Square.h"

using namespace BayouBonanza;

TEST_CASE("Bishop piece functionality", "[bishop]") {
    GameBoard board;
    
    SECTION("Construction and attributes") {
        Bishop bishop(PlayerSide::PLAYER_ONE);
        
        REQUIRE(bishop.getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(bishop.getAttack() == 4);
        REQUIRE(bishop.getHealth() == 8);
        REQUIRE(bishop.getTypeName() == "Bishop");
    }
    
    SECTION("Valid moves - empty board") {
        Bishop bishop(PlayerSide::PLAYER_ONE);
        Position pos(3, 3);
        bishop.setPosition(pos);
        
        // Place bishop on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<Bishop>(bishop));
        
        // Bishop should be able to move diagonally
        REQUIRE(bishop.isValidMove(board, Position(0, 0))); // Diagonally up-left 3 squares
        REQUIRE(bishop.isValidMove(board, Position(6, 0))); // Diagonally up-right 3 squares
        REQUIRE(bishop.isValidMove(board, Position(0, 6))); // Diagonally down-left 3 squares
        REQUIRE(bishop.isValidMove(board, Position(7, 7))); // Diagonally down-right 4 squares
        
        // Bishop should not be able to move horizontally or vertically
        REQUIRE_FALSE(bishop.isValidMove(board, Position(3, 0))); // Up
        REQUIRE_FALSE(bishop.isValidMove(board, Position(3, 7))); // Down
        REQUIRE_FALSE(bishop.isValidMove(board, Position(0, 3))); // Left
        REQUIRE_FALSE(bishop.isValidMove(board, Position(7, 3))); // Right
        
        // Bishop should not be able to move to its own position
        REQUIRE_FALSE(bishop.isValidMove(board, Position(3, 3)));
        
        // Verify getValidMoves returns correct positions
        std::vector<Position> validMoves = bishop.getValidMoves(board);
        REQUIRE(validMoves.size() == 13); // 13 diagonal positions from center
    }
    
    SECTION("Valid moves with pieces blocking") {
        Bishop bishop(PlayerSide::PLAYER_ONE);
        Position bishopPos(3, 3);
        bishop.setPosition(bishopPos);
        
        // Place bishop on board
        board.getSquare(bishopPos.x, bishopPos.y).setPiece(std::make_shared<Bishop>(bishop));
        
        // Place a friendly piece at position (1, 1) - blocking up-left diagonal movement
        Bishop friendlyPiece(PlayerSide::PLAYER_ONE);
        Position friendlyPos(1, 1);
        friendlyPiece.setPosition(friendlyPos);
        board.getSquare(friendlyPos.x, friendlyPos.y).setPiece(std::make_shared<Bishop>(friendlyPiece));
        
        // Bishop should not be able to move to or past the friendly piece's position
        REQUIRE_FALSE(bishop.isValidMove(board, Position(1, 1))); // Blocked by friendly
        REQUIRE_FALSE(bishop.isValidMove(board, Position(0, 0))); // Beyond blocked path
        
        // Bishop should still be able to move to unblocked positions
        REQUIRE(bishop.isValidMove(board, Position(2, 2))); // One square up-left (before blocker)
        REQUIRE(bishop.isValidMove(board, Position(6, 0))); // Diagonally up-right
        
        // Place an enemy piece at position (5, 5) - allowing capture
        Bishop enemyPiece(PlayerSide::PLAYER_TWO);
        Position enemyPos(5, 5);
        enemyPiece.setPosition(enemyPos);
        board.getSquare(enemyPos.x, enemyPos.y).setPiece(std::make_shared<Bishop>(enemyPiece));
        
        // Bishop should be able to move to (capture) the enemy piece's position
        REQUIRE(bishop.isValidMove(board, enemyPos));
        
        // But should not be able to move past it
        REQUIRE_FALSE(bishop.isValidMove(board, Position(6, 6)));
        REQUIRE_FALSE(bishop.isValidMove(board, Position(7, 7)));
        
        // Verify getValidMoves excludes blocked positions
        std::vector<Position> validMoves = bishop.getValidMoves(board);
        
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
            // Check if any position is beyond the friendly piece at (1, 1)
            if (pos.x < 1 && pos.y < 1) {
                foundBeyondBlocked = true;
                break;
            }
            // Check if any position is beyond the enemy piece at (5, 5)
            if (pos.x > 5 && pos.y > 5) {
                foundBeyondBlocked = true;
                break;
            }
        }
        REQUIRE_FALSE(foundBeyondBlocked);
    }
    
    SECTION("Board boundary checking") {
        // Place bishop at the edge of the board
        Bishop bishop(PlayerSide::PLAYER_ONE);
        Position edgePos(0, 0); // Top-left corner
        bishop.setPosition(edgePos);
        
        // Place bishop on board
        board.getSquare(edgePos.x, edgePos.y).setPiece(std::make_shared<Bishop>(bishop));
        
        // Bishop should not be able to move out of bounds
        REQUIRE_FALSE(bishop.isValidMove(board, Position(-1, -1)));  // Out of bounds diagonally
        
        // Bishop should be able to move within bounds
        REQUIRE(bishop.isValidMove(board, Position(7, 7))); // Diagonally down-right
        
        // Bishop should not be able to move horizontally or vertically
        REQUIRE_FALSE(bishop.isValidMove(board, Position(0, 7))); // Down
        REQUIRE_FALSE(bishop.isValidMove(board, Position(7, 0))); // Right
    }
    
    SECTION("Influence area matches valid moves") {
        Bishop bishop(PlayerSide::PLAYER_ONE);
        Position bishopPos(3, 3);
        bishop.setPosition(bishopPos);
        
        // Place bishop on board
        board.getSquare(bishopPos.x, bishopPos.y).setPiece(std::make_shared<Bishop>(bishop));
        
        // For the Bishop, the influence area should match the valid moves
        std::vector<Position> validMoves = bishop.getValidMoves(board);
        std::vector<Position> influenceArea = bishop.getInfluenceArea(board);
        
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
