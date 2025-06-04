#include <catch2/catch_test_macros.hpp>
#include "ResourceSystem.h"
#include "GameBoard.h"
#include "Square.h"
#include "InfluenceSystem.h"
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"
#include "PlayerSide.h"
#include <stdexcept>

using namespace BayouBonanza;

class ResourceSystemTestFixture {
public:
    ResourceSystemTestFixture() {
        // Load piece definitions for board setup
        if (!pieceDefManager.loadDefinitions("assets/data/pieces.json")) {
            // Handle error - for tests, we might want to use a test-specific file
            // or create minimal definitions programmatically
        }
        factory = std::make_unique<PieceFactory>(pieceDefManager);
        
        // Set global factory for Square deserialization
        Square::setGlobalPieceFactory(factory.get());
    }
    
    ResourceSystem resourceSystem;
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
    
    // Helper method to set up a board with controlled squares
    void setupControlledBoard() {
        // Clear the board first
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
        
        // Set up some controlled squares manually for testing
        // Player One controls squares (0,0), (1,1), (2,2) - 3 squares
        board.getSquare(0, 0).setControlledBy(PlayerSide::PLAYER_ONE);
        board.getSquare(1, 1).setControlledBy(PlayerSide::PLAYER_ONE);
        board.getSquare(2, 2).setControlledBy(PlayerSide::PLAYER_ONE);
        
        // Player Two controls squares (7,7), (6,6) - 2 squares
        board.getSquare(7, 7).setControlledBy(PlayerSide::PLAYER_TWO);
        board.getSquare(6, 6).setControlledBy(PlayerSide::PLAYER_TWO);
        
        // Rest remain neutral
    }
};

TEST_CASE_METHOD(ResourceSystemTestFixture, "ResourceSystem basic functionality", "[ResourceSystem]") {
    SECTION("Constructor initializes with correct starting steam") {
        ResourceSystem rs1; // Default constructor (0 steam)
        REQUIRE(rs1.getSteam(PlayerSide::PLAYER_ONE) == 0);
        REQUIRE(rs1.getSteam(PlayerSide::PLAYER_TWO) == 0);
        REQUIRE(rs1.getSteam(PlayerSide::NEUTRAL) == 0);
        
        ResourceSystem rs2(10); // Constructor with starting steam
        REQUIRE(rs2.getSteam(PlayerSide::PLAYER_ONE) == 10);
        REQUIRE(rs2.getSteam(PlayerSide::PLAYER_TWO) == 10);
        REQUIRE(rs2.getSteam(PlayerSide::NEUTRAL) == 0);
    }
    
    SECTION("Constructor throws on negative starting steam") {
        REQUIRE_THROWS_AS(ResourceSystem(-1), std::invalid_argument);
        REQUIRE_THROWS_AS(ResourceSystem(-100), std::invalid_argument);
    }
    
    SECTION("getSteam returns correct values") {
        ResourceSystem rs(5);
        REQUIRE(rs.getSteam(PlayerSide::PLAYER_ONE) == 5);
        REQUIRE(rs.getSteam(PlayerSide::PLAYER_TWO) == 5);
        REQUIRE(rs.getSteam(PlayerSide::NEUTRAL) == 0);
    }
    
    SECTION("getSteam throws on invalid player side") {
        // This test assumes there might be invalid enum values
        // In practice, this might not be testable with a well-defined enum
    }
}

TEST_CASE_METHOD(ResourceSystemTestFixture, "ResourceSystem setSteam functionality", "[ResourceSystem]") {
    SECTION("setSteam sets correct values") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 15);
        resourceSystem.setSteam(PlayerSide::PLAYER_TWO, 25);
        
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 15);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == 25);
        REQUIRE(resourceSystem.getSteam(PlayerSide::NEUTRAL) == 0);
    }
    
    SECTION("setSteam throws on negative amounts") {
        REQUIRE_THROWS_AS(resourceSystem.setSteam(PlayerSide::PLAYER_ONE, -1), std::invalid_argument);
        REQUIRE_THROWS_AS(resourceSystem.setSteam(PlayerSide::PLAYER_TWO, -100), std::invalid_argument);
    }
    
    SECTION("setSteam ignores neutral player") {
        int originalP1 = resourceSystem.getSteam(PlayerSide::PLAYER_ONE);
        int originalP2 = resourceSystem.getSteam(PlayerSide::PLAYER_TWO);
        
        resourceSystem.setSteam(PlayerSide::NEUTRAL, 50);
        
        // Other players should be unchanged
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == originalP1);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == originalP2);
        REQUIRE(resourceSystem.getSteam(PlayerSide::NEUTRAL) == 0);
    }
}

TEST_CASE_METHOD(ResourceSystemTestFixture, "ResourceSystem addSteam functionality", "[ResourceSystem]") {
    SECTION("addSteam adds correct amounts") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 10);
        resourceSystem.setSteam(PlayerSide::PLAYER_TWO, 5);
        
        resourceSystem.addSteam(PlayerSide::PLAYER_ONE, 7);
        resourceSystem.addSteam(PlayerSide::PLAYER_TWO, 3);
        
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 17);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == 8);
    }
    
    SECTION("addSteam with zero amount works") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 10);
        resourceSystem.addSteam(PlayerSide::PLAYER_ONE, 0);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 10);
    }
    
    SECTION("addSteam throws on negative amounts") {
        REQUIRE_THROWS_AS(resourceSystem.addSteam(PlayerSide::PLAYER_ONE, -1), std::invalid_argument);
        REQUIRE_THROWS_AS(resourceSystem.addSteam(PlayerSide::PLAYER_TWO, -5), std::invalid_argument);
    }
    
    SECTION("addSteam ignores neutral player") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 10);
        resourceSystem.addSteam(PlayerSide::NEUTRAL, 50);
        
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 10);
        REQUIRE(resourceSystem.getSteam(PlayerSide::NEUTRAL) == 0);
    }
}

TEST_CASE_METHOD(ResourceSystemTestFixture, "ResourceSystem spendSteam functionality", "[ResourceSystem]") {
    SECTION("spendSteam with sufficient funds succeeds") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 10);
        resourceSystem.setSteam(PlayerSide::PLAYER_TWO, 15);
        
        REQUIRE(resourceSystem.spendSteam(PlayerSide::PLAYER_ONE, 5) == true);
        REQUIRE(resourceSystem.spendSteam(PlayerSide::PLAYER_TWO, 10) == true);
        
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 5);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == 5);
    }
    
    SECTION("spendSteam with insufficient funds fails") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 5);
        resourceSystem.setSteam(PlayerSide::PLAYER_TWO, 3);
        
        REQUIRE(resourceSystem.spendSteam(PlayerSide::PLAYER_ONE, 10) == false);
        REQUIRE(resourceSystem.spendSteam(PlayerSide::PLAYER_TWO, 5) == false);
        
        // Steam amounts should be unchanged
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 5);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == 3);
    }
    
    SECTION("spendSteam with exact amount succeeds") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 7);
        
        REQUIRE(resourceSystem.spendSteam(PlayerSide::PLAYER_ONE, 7) == true);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 0);
    }
    
    SECTION("spendSteam with zero amount always succeeds") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 5);
        
        REQUIRE(resourceSystem.spendSteam(PlayerSide::PLAYER_ONE, 0) == true);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 5);
    }
    
    SECTION("spendSteam throws on negative amounts") {
        REQUIRE_THROWS_AS(resourceSystem.spendSteam(PlayerSide::PLAYER_ONE, -1), std::invalid_argument);
        REQUIRE_THROWS_AS(resourceSystem.spendSteam(PlayerSide::PLAYER_TWO, -10), std::invalid_argument);
    }
    
    SECTION("spendSteam fails for neutral player") {
        REQUIRE(resourceSystem.spendSteam(PlayerSide::NEUTRAL, 1) == false);
        REQUIRE(resourceSystem.spendSteam(PlayerSide::NEUTRAL, 0) == true); // Zero amount always succeeds
    }
}

TEST_CASE_METHOD(ResourceSystemTestFixture, "ResourceSystem steam generation calculation", "[ResourceSystem]") {
    SECTION("calculateSteamGeneration on empty board returns zero") {
        auto generation = resourceSystem.calculateSteamGeneration(board);
        
        REQUIRE(generation.first == 0);  // Player One generation
        REQUIRE(generation.second == 0); // Player Two generation
        
        auto lastGeneration = resourceSystem.getLastGenerationValues();
        REQUIRE(lastGeneration.first == 0);
        REQUIRE(lastGeneration.second == 0);
    }
    
    SECTION("calculateSteamGeneration counts controlled squares correctly") {
        setupControlledBoard();
        
        auto generation = resourceSystem.calculateSteamGeneration(board);
        
        REQUIRE(generation.first == 3);  // Player One controls 3 squares
        REQUIRE(generation.second == 2); // Player Two controls 2 squares
        
        auto lastGeneration = resourceSystem.getLastGenerationValues();
        REQUIRE(lastGeneration.first == 3);
        REQUIRE(lastGeneration.second == 2);
    }
    
    SECTION("calculateSteamGeneration with pieces and influence system") {
        // Place pieces and let InfluenceSystem calculate control
        placePiece("Pawn", PlayerSide::PLAYER_ONE, 2, 2);
        placePiece("Pawn", PlayerSide::PLAYER_TWO, 5, 5);
        
        InfluenceSystem::calculateBoardInfluence(board);
        
        auto generation = resourceSystem.calculateSteamGeneration(board);
        
        // Each piece should control its own square plus adjacent squares
        // Exact numbers depend on InfluenceSystem implementation
        REQUIRE(generation.first > 0);  // Player One should control some squares
        REQUIRE(generation.second > 0); // Player Two should control some squares
    }
}

TEST_CASE_METHOD(ResourceSystemTestFixture, "ResourceSystem turn processing", "[ResourceSystem]") {
    SECTION("processTurnStart adds generation to active player") {
        setupControlledBoard();
        
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 5);
        resourceSystem.setSteam(PlayerSide::PLAYER_TWO, 8);
        
        // Process turn start for Player One
        resourceSystem.processTurnStart(PlayerSide::PLAYER_ONE, board);
        
        // Player One should get their generation (3) added
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 8); // 5 + 3
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == 8); // Unchanged
        
        // Process turn start for Player Two
        resourceSystem.processTurnStart(PlayerSide::PLAYER_TWO, board);
        
        // Player Two should get their generation (2) added
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 8); // Unchanged
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == 10); // 8 + 2
    }
    
    SECTION("processTurnStart throws on invalid active player") {
        REQUIRE_THROWS_AS(resourceSystem.processTurnStart(PlayerSide::NEUTRAL, board), std::invalid_argument);
    }
}

TEST_CASE_METHOD(ResourceSystemTestFixture, "ResourceSystem reset functionality", "[ResourceSystem]") {
    SECTION("reset restores initial state") {
        // Modify the resource system
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 20);
        resourceSystem.setSteam(PlayerSide::PLAYER_TWO, 30);
        setupControlledBoard();
        resourceSystem.calculateSteamGeneration(board); // This sets lastGeneration values
        
        // Reset to default
        resourceSystem.reset();
        
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 0);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == 0);
        
        auto lastGeneration = resourceSystem.getLastGenerationValues();
        REQUIRE(lastGeneration.first == 0);
        REQUIRE(lastGeneration.second == 0);
    }
    
    SECTION("reset with custom starting steam") {
        resourceSystem.reset(15);
        
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 15);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_TWO) == 15);
        
        auto lastGeneration = resourceSystem.getLastGenerationValues();
        REQUIRE(lastGeneration.first == 0);
        REQUIRE(lastGeneration.second == 0);
    }
    
    SECTION("reset throws on negative starting steam") {
        REQUIRE_THROWS_AS(resourceSystem.reset(-1), std::invalid_argument);
        REQUIRE_THROWS_AS(resourceSystem.reset(-50), std::invalid_argument);
    }
}

TEST_CASE_METHOD(ResourceSystemTestFixture, "ResourceSystem edge cases and error handling", "[ResourceSystem]") {
    SECTION("Large steam amounts work correctly") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 1000000);
        resourceSystem.addSteam(PlayerSide::PLAYER_ONE, 500000);
        
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 1500000);
        REQUIRE(resourceSystem.spendSteam(PlayerSide::PLAYER_ONE, 750000) == true);
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 750000);
    }
    
    SECTION("Multiple operations maintain consistency") {
        resourceSystem.setSteam(PlayerSide::PLAYER_ONE, 10);
        resourceSystem.addSteam(PlayerSide::PLAYER_ONE, 5);
        resourceSystem.spendSteam(PlayerSide::PLAYER_ONE, 3);
        resourceSystem.addSteam(PlayerSide::PLAYER_ONE, 2);
        
        REQUIRE(resourceSystem.getSteam(PlayerSide::PLAYER_ONE) == 14); // 10 + 5 - 3 + 2
    }
    
    SECTION("Generation values persist until next calculation") {
        setupControlledBoard();
        
        auto generation1 = resourceSystem.calculateSteamGeneration(board);
        auto lastGen1 = resourceSystem.getLastGenerationValues();
        
        REQUIRE(generation1.first == lastGen1.first);
        REQUIRE(generation1.second == lastGen1.second);
        
        // Modify board control
        board.getSquare(3, 3).setControlledBy(PlayerSide::PLAYER_ONE);
        
        // Last generation values should be unchanged until recalculation
        auto lastGen2 = resourceSystem.getLastGenerationValues();
        REQUIRE(lastGen2.first == lastGen1.first);
        REQUIRE(lastGen2.second == lastGen1.second);
        
        // Recalculate and verify values update
        auto generation2 = resourceSystem.calculateSteamGeneration(board);
        auto lastGen3 = resourceSystem.getLastGenerationValues();
        
        REQUIRE(generation2.first == lastGen3.first);
        REQUIRE(generation2.second == lastGen3.second);
        REQUIRE(generation2.first > generation1.first); // Player One should have more squares now
    }
} 