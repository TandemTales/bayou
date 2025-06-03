#include <catch2/catch_test_macros.hpp>
#include "InfluenceSystem.h"
#include "GameBoard.h"
#include "Square.h"
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"
#include "PlayerSide.h"
#include "PieceData.h"

using namespace BayouBonanza;

class InfluenceSystemTestFixture {
public:
    InfluenceSystemTestFixture() {
        // Load piece definitions
        if (!pieceDefManager.loadDefinitions("assets/data/pieces.json")) {
            // Handle error - for tests, we might want to use a test-specific file
            // or create minimal definitions programmatically
        }
        factory = std::make_unique<PieceFactory>(pieceDefManager);
        
        // Set global factory for Square deserialization
        Square::setGlobalPieceFactory(factory.get());
    }
    
    GameBoard board;
    PieceDefinitionManager pieceDefManager;
    std::unique_ptr<PieceFactory> factory;
    
    // Helper method to place a piece on the board
    void placePiece(const std::string& pieceType, PlayerSide side, int x, int y) {
        auto piece = factory->createPiece(pieceType, side);
        if (piece) {
            piece->setPosition({x, y});
            board.getSquare(x, y).setPiece(std::move(piece));
        }
    }
    
    // Helper method to check if a position has expected control values
    bool hasControlValues(int x, int y, int player1Control, int player2Control) {
        const Square& square = board.getSquare(x, y);
        return square.getControlValue(PlayerSide::PLAYER_ONE) == player1Control &&
               square.getControlValue(PlayerSide::PLAYER_TWO) == player2Control;
    }
};

TEST_CASE_METHOD(InfluenceSystemTestFixture, "InfluenceSystem basic functionality", "[InfluenceSystem]") {
    SECTION("Empty board has no control") {
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Check that all squares are neutral
        for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
                const Square& square = board.getSquare(x, y);
                REQUIRE(InfluenceSystem::getControllingPlayer(square) == PlayerSide::NEUTRAL);
            }
        }
    }
    
    SECTION("Single piece controls its own square and influences adjacent squares") {
        // Place a piece in the center of the board
        auto piece = factory->createPiece("Pawn", PlayerSide::PLAYER_ONE);
        board.getSquare(4, 4).setPiece(std::move(piece));
        
        InfluenceSystem::calculateBoardInfluence(board);
        
        // The piece should control its own square
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 4)) == PlayerSide::PLAYER_ONE);
        
        // Check all 8 adjacent squares should be controlled by Player One
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue; // Skip the piece's own square
                
                int x = 4 + dx;
                int y = 4 + dy;
                const Square& square = board.getSquare(x, y);
                
                // Should be controlled by Player One (1 influence point vs 0)
                REQUIRE(InfluenceSystem::getControllingPlayer(square) == PlayerSide::PLAYER_ONE);
                REQUIRE(square.getControlValue(PlayerSide::PLAYER_ONE) == 1);
                REQUIRE(square.getControlValue(PlayerSide::PLAYER_TWO) == 0);
            }
        }
        
        // Check that non-adjacent squares are neutral
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(2, 2)) == PlayerSide::NEUTRAL);
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(6, 6)) == PlayerSide::NEUTRAL);
    }
}

TEST_CASE_METHOD(InfluenceSystemTestFixture, "InfluenceSystem sticky control behavior", "[InfluenceSystem]") {
    SECTION("Control persists when piece moves away") {
        // Place a piece, establish control, then remove it
        auto piece = factory->createPiece("Pawn", PlayerSide::PLAYER_ONE);
        board.getSquare(4, 4).setPiece(std::move(piece));
        
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Verify Player One controls adjacent squares
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 5)) == PlayerSide::PLAYER_ONE);
        
        // Remove the piece (simulating it moving away)
        board.getSquare(4, 4).extractPiece();
        
        // Recalculate influence
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Player One should still control the square even with no influence
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 5)) == PlayerSide::PLAYER_ONE);
        REQUIRE(board.getSquare(4, 5).getControlValue(PlayerSide::PLAYER_ONE) == 0); // No current influence
        REQUIRE(board.getSquare(4, 5).getControlValue(PlayerSide::PLAYER_TWO) == 0); // No current influence
    }
    
    SECTION("Control only changes when another player has MORE influence") {
        // Clear board state from previous section
        for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
                if (!board.getSquare(x, y).isEmpty()) {
                    board.getSquare(x, y).extractPiece();
                }
                board.getSquare(x, y).setControlValue(PlayerSide::PLAYER_ONE, 0);
                board.getSquare(x, y).setControlValue(PlayerSide::PLAYER_TWO, 0);
                board.getSquare(x, y).setControlledBy(PlayerSide::NEUTRAL);
            }
        }
        
        // Player One establishes control
        auto piece1 = factory->createPiece("Pawn", PlayerSide::PLAYER_ONE);
        board.getSquare(4, 4).setPiece(std::move(piece1));
        
        InfluenceSystem::calculateBoardInfluence(board);
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 5)) == PlayerSide::PLAYER_ONE);
        
        // Remove Player One's piece
        board.getSquare(4, 4).extractPiece();
        
        // Player Two places a piece with equal influence (1 point)
        auto piece2 = factory->createPiece("Pawn", PlayerSide::PLAYER_TWO);
        board.getSquare(4, 6).setPiece(std::move(piece2)); // Adjacent to (4,5)
        
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Player Two should take control because they have MORE influence (1 > 0)
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 5)) == PlayerSide::PLAYER_TWO);
        REQUIRE(board.getSquare(4, 5).getControlValue(PlayerSide::PLAYER_ONE) == 0);
        REQUIRE(board.getSquare(4, 5).getControlValue(PlayerSide::PLAYER_TWO) == 1);
        
        // Add another Player Two piece to give them MORE influence
        auto piece3 = factory->createPiece("Pawn", PlayerSide::PLAYER_TWO);
        board.getSquare(3, 5).setPiece(std::move(piece3)); // Also adjacent to (4,5)
        
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Now Player Two should take control (2 influence vs 0 - MORE influence)
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 5)) == PlayerSide::PLAYER_TWO);
        REQUIRE(board.getSquare(4, 5).getControlValue(PlayerSide::PLAYER_ONE) == 0);
        REQUIRE(board.getSquare(4, 5).getControlValue(PlayerSide::PLAYER_TWO) == 2);
    }
}

TEST_CASE_METHOD(InfluenceSystemTestFixture, "InfluenceSystem piece type independence", "[InfluenceSystem]") {
    SECTION("Different piece types have same influence pattern") {
        // Test that different piece types all influence the same way
        std::vector<std::string> pieceTypes = {"Pawn", "Rook", "Bishop", "Knight", "Queen", "King"};
        
        for (const std::string& pieceType : pieceTypes) {
            // Clear pieces but preserve control state (don't call resetBoard())
            for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
                for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
                    if (!board.getSquare(x, y).isEmpty()) {
                        board.getSquare(x, y).extractPiece();
                    }
                    // Reset influence values but preserve control
                    board.getSquare(x, y).setControlValue(PlayerSide::PLAYER_ONE, 0);
                    board.getSquare(x, y).setControlValue(PlayerSide::PLAYER_TWO, 0);
                    // Reset control to neutral for clean test
                    board.getSquare(x, y).setControlledBy(PlayerSide::NEUTRAL);
                }
            }
            
            // Place piece in center
            auto piece = factory->createPiece(pieceType, PlayerSide::PLAYER_ONE);
            board.getSquare(4, 4).setPiece(std::move(piece));
            
            InfluenceSystem::calculateBoardInfluence(board);
            
            // All piece types should control their own square
            REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 4)) == PlayerSide::PLAYER_ONE);
            
            // All piece types should influence exactly the 8 adjacent squares
            int influencedSquares = 0;
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue; // Skip the piece's own square
                    
                    int x = 4 + dx;
                    int y = 4 + dy;
                    const Square& square = board.getSquare(x, y);
                    
                    if (InfluenceSystem::getControllingPlayer(square) == PlayerSide::PLAYER_ONE) {
                        influencedSquares++;
                        REQUIRE(square.getControlValue(PlayerSide::PLAYER_ONE) == 1);
                    }
                }
            }
            
            // Should influence exactly 8 adjacent squares
            REQUIRE(influencedSquares == 8);
        }
    }
}

TEST_CASE_METHOD(InfluenceSystemTestFixture, "InfluenceSystem contested squares", "[InfluenceSystem]") {
    SECTION("Adjacent pieces contest control") {
        // Place two opposing pieces next to each other
        auto piece1 = factory->createPiece("Pawn", PlayerSide::PLAYER_ONE);
        auto piece2 = factory->createPiece("Pawn", PlayerSide::PLAYER_TWO);
        
        board.getSquare(3, 3).setPiece(std::move(piece1));
        board.getSquare(5, 3).setPiece(std::move(piece2));
        
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Each piece controls its own square
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(3, 3)) == PlayerSide::PLAYER_ONE);
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(5, 3)) == PlayerSide::PLAYER_TWO);
        
        // The square between them (4,3) should be contested (both have 1 influence)
        // Since no one controlled it initially, it should remain neutral in a tie
        const Square& contestedSquare = board.getSquare(4, 3);
        REQUIRE(contestedSquare.getControlValue(PlayerSide::PLAYER_ONE) == 1);
        REQUIRE(contestedSquare.getControlValue(PlayerSide::PLAYER_TWO) == 1);
        REQUIRE(InfluenceSystem::getControllingPlayer(contestedSquare) == PlayerSide::NEUTRAL);
    }
    
    SECTION("Multiple pieces can influence same square") {
        // Place multiple pieces that influence the same square
        auto piece1 = factory->createPiece("Pawn", PlayerSide::PLAYER_ONE);
        auto piece2 = factory->createPiece("Rook", PlayerSide::PLAYER_ONE);
        
        // Both pieces will influence square (4,4)
        board.getSquare(3, 3).setPiece(std::move(piece1)); // Influences (4,4)
        board.getSquare(5, 5).setPiece(std::move(piece2)); // Also influences (4,4)
        
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Square (4,4) should have 2 influence points from Player One
        const Square& influencedSquare = board.getSquare(4, 4);
        REQUIRE(influencedSquare.getControlValue(PlayerSide::PLAYER_ONE) == 2);
        REQUIRE(influencedSquare.getControlValue(PlayerSide::PLAYER_TWO) == 0);
        REQUIRE(InfluenceSystem::getControllingPlayer(influencedSquare) == PlayerSide::PLAYER_ONE);
    }
}

TEST_CASE_METHOD(InfluenceSystemTestFixture, "InfluenceSystem edge cases", "[InfluenceSystem]") {
    SECTION("Pieces on board edges") {
        // Place piece on corner
        auto piece = factory->createPiece("Pawn", PlayerSide::PLAYER_ONE);
        board.getSquare(0, 0).setPiece(std::move(piece));
        
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Corner piece controls its own square
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(0, 0)) == PlayerSide::PLAYER_ONE);
        
        // Should influence only the 3 valid adjacent squares
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(1, 0)) == PlayerSide::PLAYER_ONE);
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(0, 1)) == PlayerSide::PLAYER_ONE);
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(1, 1)) == PlayerSide::PLAYER_ONE);
        
        // Other squares should be neutral
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(2, 2)) == PlayerSide::NEUTRAL);
    }
    
    SECTION("Reset influence values preserves control") {
        // Set up some influence and control
        auto piece = factory->createPiece("Pawn", PlayerSide::PLAYER_ONE);
        board.getSquare(4, 4).setPiece(std::move(piece));
        InfluenceSystem::calculateBoardInfluence(board);
        
        // Verify control is established
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 5)) == PlayerSide::PLAYER_ONE);
        
        // Remove piece and reset influence
        board.getSquare(4, 4).extractPiece();
        InfluenceSystem::resetInfluenceValues(board);
        
        // Control should be preserved even though influence is reset
        REQUIRE(InfluenceSystem::getControllingPlayer(board.getSquare(4, 5)) == PlayerSide::PLAYER_ONE);
        REQUIRE(board.getSquare(4, 5).getControlValue(PlayerSide::PLAYER_ONE) == 0);
        REQUIRE(board.getSquare(4, 5).getControlValue(PlayerSide::PLAYER_TWO) == 0);
    }
} 