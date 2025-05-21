#include <catch2/catch_test_macros.hpp>
#include "Pawn.h"
#include "GameBoard.h"
#include "Square.h"

using namespace BayouBonanza;

TEST_CASE("Pawn piece functionality", "[pawn]") {
    GameBoard board;
    
    SECTION("Construction and attributes") {
        Pawn pawn(PlayerSide::PLAYER_ONE);
        
        REQUIRE(pawn.getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(pawn.getAttack() == 2);
        REQUIRE(pawn.getHealth() == 5);
        REQUIRE(pawn.getTypeName() == "Pawn");
    }
    
    SECTION("Valid moves - Player One (North) Pawn") {
        Pawn pawn(PlayerSide::PLAYER_ONE);
        Position pos(3, 3);
        pawn.setPosition(pos);
        
        // Place pawn on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<Pawn>(pawn));
        
        // Pawn should be able to move forward (north direction is -y)
        REQUIRE(pawn.isValidMove(board, Position(3, 2))); // Forward one square
        
        // Pawn should not be able to move in other directions
        REQUIRE_FALSE(pawn.isValidMove(board, Position(3, 4))); // Backward
        REQUIRE_FALSE(pawn.isValidMove(board, Position(2, 3))); // Left
        REQUIRE_FALSE(pawn.isValidMove(board, Position(4, 3))); // Right
        REQUIRE_FALSE(pawn.isValidMove(board, Position(2, 2))); // Diagonal without enemy
        REQUIRE_FALSE(pawn.isValidMove(board, Position(4, 2))); // Diagonal without enemy
        
        // Pawn should not be able to move to its own position
        REQUIRE_FALSE(pawn.isValidMove(board, Position(3, 3)));
        
        // Place enemy pieces at capture positions
        Pawn enemyPawn1(PlayerSide::PLAYER_TWO);
        Position enemyPos1(2, 2);
        enemyPawn1.setPosition(enemyPos1);
        board.getSquare(enemyPos1.x, enemyPos1.y).setPiece(std::make_shared<Pawn>(enemyPawn1));
        
        Pawn enemyPawn2(PlayerSide::PLAYER_TWO);
        Position enemyPos2(4, 2);
        enemyPawn2.setPosition(enemyPos2);
        board.getSquare(enemyPos2.x, enemyPos2.y).setPiece(std::make_shared<Pawn>(enemyPawn2));
        
        // Pawn should be able to capture diagonally
        REQUIRE(pawn.isValidMove(board, Position(2, 2))); // Capture left-forward
        REQUIRE(pawn.isValidMove(board, Position(4, 2))); // Capture right-forward
        
        // Verify getValidMoves returns correct positions
        std::vector<Position> validMoves = pawn.getValidMoves(board);
        REQUIRE(validMoves.size() == 3); // Forward + 2 diagonal captures
    }
    
    SECTION("Valid moves - Player Two (South) Pawn") {
        Pawn pawn(PlayerSide::PLAYER_TWO);
        Position pos(3, 3);
        pawn.setPosition(pos);
        
        // Place pawn on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<Pawn>(pawn));
        
        // Pawn should be able to move forward (south direction is +y)
        REQUIRE(pawn.isValidMove(board, Position(3, 4))); // Forward one square
        
        // Pawn should not be able to move in other directions
        REQUIRE_FALSE(pawn.isValidMove(board, Position(3, 2))); // Backward
        REQUIRE_FALSE(pawn.isValidMove(board, Position(2, 3))); // Left
        REQUIRE_FALSE(pawn.isValidMove(board, Position(4, 3))); // Right
        REQUIRE_FALSE(pawn.isValidMove(board, Position(2, 4))); // Diagonal without enemy
        REQUIRE_FALSE(pawn.isValidMove(board, Position(4, 4))); // Diagonal without enemy
        
        // Place enemy pieces at capture positions
        Pawn enemyPawn1(PlayerSide::PLAYER_ONE);
        Position enemyPos1(2, 4);
        enemyPawn1.setPosition(enemyPos1);
        board.getSquare(enemyPos1.x, enemyPos1.y).setPiece(std::make_shared<Pawn>(enemyPawn1));
        
        Pawn enemyPawn2(PlayerSide::PLAYER_ONE);
        Position enemyPos2(4, 4);
        enemyPawn2.setPosition(enemyPos2);
        board.getSquare(enemyPos2.x, enemyPos2.y).setPiece(std::make_shared<Pawn>(enemyPawn2));
        
        // Pawn should be able to capture diagonally
        REQUIRE(pawn.isValidMove(board, Position(2, 4))); // Capture left-forward
        REQUIRE(pawn.isValidMove(board, Position(4, 4))); // Capture right-forward
    }
    
    SECTION("Movement blocked by pieces") {
        Pawn pawn(PlayerSide::PLAYER_ONE);
        Position pawnPos(3, 3);
        pawn.setPosition(pawnPos);
        
        // Place pawn on board
        board.getSquare(pawnPos.x, pawnPos.y).setPiece(std::make_shared<Pawn>(pawn));
        
        // Place a piece (friendly or enemy doesn't matter) blocking forward movement
        Pawn blockingPiece(PlayerSide::PLAYER_ONE);
        Position blockingPos(3, 2);
        blockingPiece.setPosition(blockingPos);
        board.getSquare(blockingPos.x, blockingPos.y).setPiece(std::make_shared<Pawn>(blockingPiece));
        
        // Pawn should not be able to move forward when blocked
        REQUIRE_FALSE(pawn.isValidMove(board, Position(3, 2)));
        
        // Verify getValidMoves excludes the blocked forward move
        std::vector<Position> validMoves = pawn.getValidMoves(board);
        REQUIRE(validMoves.empty()); // No valid moves when forward is blocked and no captures are available
    }
    
    SECTION("Board boundary checking") {
        // Place pawn at the edge of the board
        Pawn northPawn(PlayerSide::PLAYER_ONE);
        Position northEdgePos(0, 0); // Top-left corner
        northPawn.setPosition(northEdgePos);
        
        // Place pawn on board
        board.getSquare(northEdgePos.x, northEdgePos.y).setPiece(std::make_shared<Pawn>(northPawn));
        
        // North pawn at top edge can't move forward (would go out of bounds)
        REQUIRE_FALSE(northPawn.isValidMove(board, Position(0, -1)));
        
        // Place a south pawn at the bottom edge
        Pawn southPawn(PlayerSide::PLAYER_TWO);
        Position southEdgePos(7, 7); // Bottom-right corner
        southPawn.setPosition(southEdgePos);
        
        // Place pawn on board
        board.getSquare(southEdgePos.x, southEdgePos.y).setPiece(std::make_shared<Pawn>(southPawn));
        
        // South pawn at bottom edge can't move forward (would go out of bounds)
        REQUIRE_FALSE(southPawn.isValidMove(board, Position(7, 8)));
    }
    
    SECTION("Influence area for Pawn") {
        Pawn northPawn(PlayerSide::PLAYER_ONE);
        Position northPawnPos(3, 3);
        northPawn.setPosition(northPawnPos);
        
        // Place north pawn on board
        board.getSquare(northPawnPos.x, northPawnPos.y).setPiece(std::make_shared<Pawn>(northPawn));
        
        // For the Pawn, the influence area should be the diagonal capture squares
        std::vector<Position> influenceArea = northPawn.getInfluenceArea(board);
        REQUIRE(influenceArea.size() == 2); // 2 diagonal capture positions
        
        // Check that influence area contains the diagonal capture squares
        bool foundLeftDiagonal = false;
        bool foundRightDiagonal = false;
        
        for (const Position& pos : influenceArea) {
            if (pos.x == 2 && pos.y == 2) {
                foundLeftDiagonal = true;
            }
            if (pos.x == 4 && pos.y == 2) {
                foundRightDiagonal = true;
            }
        }
        
        REQUIRE(foundLeftDiagonal);
        REQUIRE(foundRightDiagonal);
        
        // Influence should not include the forward square
        bool foundForward = false;
        for (const Position& pos : influenceArea) {
            if (pos.x == 3 && pos.y == 2) {
                foundForward = true;
                break;
            }
        }
        REQUIRE_FALSE(foundForward);
    }
}
