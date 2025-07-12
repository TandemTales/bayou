#include <catch2/catch_test_macros.hpp>
#include "Piece.h"
#include "GameBoard.h"
#include "Square.h"
#include "PieceDefinitionManager.h"
#include "PieceFactory.h"
#include "PlayerSide.h"

using namespace BayouBonanza;

TEST_CASE("Automatick piece functionality", "[automatick]") {
    PieceDefinitionManager pdm;
    pdm.loadDefinitions("../../assets/data/cards.json");
    PieceFactory factory(pdm);
    GameBoard board;
    
    SECTION("Construction and attributes") {
        const auto* stats = pdm.getPieceStats("Automatick");
        REQUIRE(stats != nullptr);
        auto automatick = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        
        REQUIRE(automatick->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(automatick->getAttack() == stats->attack);
        REQUIRE(automatick->getHealth() == stats->health);
        REQUIRE(automatick->getTypeName() == "Automatick");
    }
    
    SECTION("Valid moves - empty board") {
        const auto& stats = pdm.getStats("Automatick");
        auto automatick = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        Position pos(3, 3);
        automatick->setPosition(pos);
        
        // Place automatick on board
        board.getSquare(pos.x, pos.y).setPiece(std::make_shared<Piece>(automatick));
        
        // Automatick should be able to move in L-shapes (2 squares in one direction and 1 square perpendicular)
        REQUIRE(automatick->isValidMove(board, Position(1, 2))); // 2 left, 1 up
        REQUIRE(automatick->isValidMove(board, Position(2, 1))); // 1 left, 2 up
        REQUIRE(automatick->isValidMove(board, Position(5, 2))); // 2 right, 1 up
        REQUIRE(automatick->isValidMove(board, Position(4, 1))); // 1 right, 2 up
        REQUIRE(automatick->isValidMove(board, Position(1, 4))); // 2 left, 1 down
        REQUIRE(automatick->isValidMove(board, Position(2, 5))); // 1 left, 2 down
        REQUIRE(automatick->isValidMove(board, Position(5, 4))); // 2 right, 1 down
        REQUIRE(automatick->isValidMove(board, Position(4, 5))); // 1 right, 2 down
        
        // Automatick should not be able to move in other patterns
        REQUIRE_FALSE(automatick->isValidMove(board, Position(2, 2))); // Diagonal
        REQUIRE_FALSE(automatick->isValidMove(board, Position(3, 5))); // Straight vertical
        REQUIRE_FALSE(automatick->isValidMove(board, Position(6, 3))); // Straight horizontal
        
        // Automatick should not be able to move to its own position
        REQUIRE_FALSE(automatick->isValidMove(board, Position(3, 3)));
        
        // Verify getValidMoves returns correct positions
        std::vector<Position> validMoves = automatick->getValidMoves(board);
        REQUIRE(validMoves.size() == 8); // 8 possible L-shaped moves from center
    }
    
    SECTION("Valid moves with pieces present") {
        const auto& stats = pdm.getStats("Automatick");
        auto automatick = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        Position automatickPos(3, 3);
        automatick->setPosition(automatickPos);
        
        // Place automatick on board
        board.getSquare(automatickPos.x, automatickPos.y).setPiece(std::make_shared<Piece>(automatick));
        
        // Place a friendly piece at position (1, 2)
        const auto& friendlyStats = pdm.getStats("Automatick");
        auto friendlyPiece = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        Position friendlyPos(1, 2);
        friendlyPiece->setPosition(friendlyPos);
        board.getSquare(friendlyPos.x, friendlyPos.y).setPiece(std::make_shared<Piece>(friendlyPiece));
        
        // Automatick should not be able to move to the friendly piece's position
        REQUIRE_FALSE(automatick->isValidMove(board, friendlyPos));
        
        // Place pieces in the path that a automatick would normally jump over
        // This should not affect the automatick's movement
        const auto& blockingStats1 = pdm.getStats("Automatick");
        auto blockingPiece1 = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        Position blockingPos1(2, 2); // Diagonal from automatick
        blockingPiece1->setPosition(blockingPos1);
        board.getSquare(blockingPos1.x, blockingPos1.y).setPiece(std::make_shared<Piece>(blockingPiece1));
        
        const auto& blockingStats2 = pdm.getStats("Automatick");
        auto blockingPiece2 = factory.createPiece("Automatick", PlayerSide::PLAYER_TWO);
        Position blockingPos2(2, 3); // Adjacent to automatick
        blockingPiece2->setPosition(blockingPos2);
        board.getSquare(blockingPos2.x, blockingPos2.y).setPiece(std::make_shared<Piece>(blockingPiece2));
        
        // Automatick should still be able to jump over these pieces to reach (1, 4)
        REQUIRE(automatick->isValidMove(board, Position(1, 4)));
        
        // Place an enemy piece at position (5, 4)
        const auto& enemyStats = pdm.getStats("Automatick");
        auto enemyPiece = factory.createPiece("Automatick", PlayerSide::PLAYER_TWO);
        Position enemyPos(5, 4);
        enemyPiece->setPosition(enemyPos);
        board.getSquare(enemyPos.x, enemyPos.y).setPiece(std::make_shared<Piece>(enemyPiece));
        
        // Automatick should be able to move to (capture) the enemy piece's position
        REQUIRE(automatick->isValidMove(board, enemyPos));
        
        // Verify getValidMoves excludes the friendly piece position
        std::vector<Position> validMoves = automatick->getValidMoves(board);
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
        // Place automatick at the edge of the board
        const auto& stats = pdm.getStats("Automatick");
        auto automatick = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        Position edgePos(0, 0); // Top-left corner
        automatick->setPosition(edgePos);
        
        // Place automatick on board
        board.getSquare(edgePos.x, edgePos.y).setPiece(std::make_shared<Piece>(automatick));
        
        // Automatick should not be able to move out of bounds
        REQUIRE_FALSE(automatick->isValidMove(board, Position(-1, 2)));  // 2 up, 1 left (out of bounds)
        REQUIRE_FALSE(automatick->isValidMove(board, Position(-2, 1)));  // 1 up, 2 left (out of bounds)
        
        // Automatick should be able to move within bounds
        REQUIRE(automatick->isValidMove(board, Position(1, 2))); // 1 right, 2 down
        REQUIRE(automatick->isValidMove(board, Position(2, 1))); // 2 right, 1 down
        
        // Verify getValidMoves only includes in-bounds positions
        std::vector<Position> validMoves = automatick->getValidMoves(board);
        REQUIRE(validMoves.size() == 2); // Only 2 valid moves from corner
    }
    
    SECTION("Influence area matches valid moves") {
        const auto& stats = pdm.getStats("Automatick");
        auto automatick = factory.createPiece("Automatick", PlayerSide::PLAYER_ONE);
        Position automatickPos(3, 3);
        automatick->setPosition(automatickPos);
        
        // Place automatick on board
        board.getSquare(automatickPos.x, automatickPos.y).setPiece(std::make_shared<Piece>(automatick));
        
        // For the Automatick, the influence area should match the valid moves
        std::vector<Position> validMoves = automatick->getValidMoves(board);
        std::vector<Position> influenceArea = automatick->getInfluenceArea(board);
        
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
