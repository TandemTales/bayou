#include <catch2/catch_test_macros.hpp>
#include "Piece.h"
#include "GameBoard.h"
#include "Square.h"
#include "PieceDefinitionManager.h"
#include "PieceFactory.h"
#include "PlayerSide.h"
#include "PieceData.h"  // For Position
#include "MoveExecutor.h"
#include "GameState.h"

using namespace BayouBonanza;
#include <vector>
#include <algorithm> // For std::find_if

// Note: "assets/data/cards.json" needs to be accessible when tests are run.
// Catch2 tests are typically run from the build directory.
// Ensure "assets" is copied to a location findable from there, or use absolute paths/configure paths.
// For this example, we'll assume "assets/data/cards.json" can be found relative to the build/test execution dir.
// A common practice is to copy assets to the build directory alongside the test executable.

struct TestFixture {
    BayouBonanza::PieceDefinitionManager pdm;
    BayouBonanza::PieceFactory factory;
    BayouBonanza::GameBoard board; // GameBoard constructor initializes an 8x8 board

    TestFixture() : factory(pdm) {
        // Attempt to load definitions. Path might need adjustment based on test execution directory.
        // Assets are copied to the test executable directory, so use relative path from there
        bool loaded = pdm.loadDefinitions("assets/data/cards.json"); // Try relative path from test executable directory
        if (!loaded) {
            loaded = pdm.loadDefinitions("../../assets/data/cards.json"); // Fallback to original path
        }
        REQUIRE(loaded); // Crucial for tests to run
    }

    // Helper to check if a position is in a list of moves
    static bool containsMove(const std::vector<Position>& moves, const Position& pos) {
        return std::find_if(moves.begin(), moves.end(), [&](const Position& move) {
            return move.x == pos.x && move.y == pos.y;
        }) != moves.end();
    }
};


TEST_CASE_METHOD(TestFixture, "Piece Data-Driven Functionality", "[piece]") {

        SECTION("TinkeringTom Functionality") {
            const auto* stats = pdm.getPieceStats("TinkeringTom");
            REQUIRE(stats != nullptr);
            auto king = factory.createPiece("TinkeringTom", BayouBonanza::PlayerSide::PLAYER_ONE);
            REQUIRE(king != nullptr);

            REQUIRE(king->getTypeName() == "TinkeringTom");
            REQUIRE(king->getSymbol() == stats->symbol);
            REQUIRE(king->getAttack() == stats->attack);
            REQUIRE(king->getHealth() == stats->health);

        board.getSquare(4, 7).setPiece(std::move(king)); // Place TinkeringTom
        BayouBonanza::Piece* kingPtr = board.getSquare(4,7).getPiece();
        REQUIRE(kingPtr != nullptr);
        kingPtr->setPosition({3,3});


        auto validMoves = kingPtr->getValidMoves(board); 
        // TinkeringTom moves one step in all 8 directions
        REQUIRE(validMoves.size() == 8); // Assuming edge of board limits some moves for a typical 4,7 start
                                        // For a 3,3 start, it's 8. For 4,7 on 8x8, it's 5 moves.
                                        // Let's place at 3,3 for full 8 moves.
        board.getSquare(4,7).setPiece(nullptr); // remove old king
        auto king2 = factory.createPiece("TinkeringTom", BayouBonanza::PlayerSide::PLAYER_ONE);
        board.getSquare(3,3).setPiece(std::move(king2));
        BayouBonanza::Piece* king2Ptr = board.getSquare(3,3).getPiece();
        king2Ptr->setPosition({3,3});
        validMoves = king2Ptr->getValidMoves(board);
        REQUIRE(validMoves.size() == 8);


        REQUIRE(TestFixture::containsMove(validMoves, {2,2}));
        REQUIRE(TestFixture::containsMove(validMoves, {3,2}));
        REQUIRE(TestFixture::containsMove(validMoves, {4,2}));
        REQUIRE(TestFixture::containsMove(validMoves, {2,3}));
        REQUIRE(TestFixture::containsMove(validMoves, {4,3}));
        REQUIRE(TestFixture::containsMove(validMoves, {2,4}));
        REQUIRE(TestFixture::containsMove(validMoves, {3,4}));
        REQUIRE(TestFixture::containsMove(validMoves, {4,4}));
        
        auto influence = king2Ptr->getInfluenceArea(board);
        REQUIRE(influence.size() == 8); // Same as moves for TinkeringTom
    }

    SECTION("Sentroid Functionality - Player One") {
        const auto* stats = pdm.getPieceStats("Sentroid");
        REQUIRE(stats != nullptr);
        auto pawn = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_ONE);
        REQUIRE(pawn != nullptr);
        REQUIRE(pawn->getTypeName() == "Sentroid");
        REQUIRE(pawn->getSide() == BayouBonanza::PlayerSide::PLAYER_ONE);
        REQUIRE(pawn->getAttack() == stats->attack);
        REQUIRE(pawn->getHealth() == stats->health);

        board.getSquare(3, 6).setPiece(std::move(pawn)); // Place Sentroid for Player One
        BayouBonanza::Piece* pawnPtr = board.getSquare(3,6).getPiece();
        REQUIRE(pawnPtr != nullptr);
        pawnPtr->setPosition({3,6});


        auto validMoves = pawnPtr->getValidMoves(board);
        REQUIRE(validMoves.size() == 1); // Forward one step
        REQUIRE(TestFixture::containsMove(validMoves, {3,5})); // Player ONE moves Y-

        // Test capture
        auto enemyPawn = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_TWO);
        board.getSquare(2,5).setPiece(std::move(enemyPawn));
        board.getSquare(2,5).getPiece()->setPosition({2,5});

        auto enemyPawn2 = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_TWO);
        board.getSquare(4,5).setPiece(std::move(enemyPawn2));
        board.getSquare(4,5).getPiece()->setPosition({4,5});


        validMoves = pawnPtr->getValidMoves(board);
        REQUIRE(validMoves.size() == 3); // 1 forward, 2 captures
        REQUIRE(TestFixture::containsMove(validMoves, {3,5})); 
        REQUIRE(TestFixture::containsMove(validMoves, {2,5})); 
        REQUIRE(TestFixture::containsMove(validMoves, {4,5})); 

        auto influence = pawnPtr->getInfluenceArea(board);
        REQUIRE(influence.size() == 2); // Diagonal forward influence/capture
        REQUIRE(TestFixture::containsMove(influence, {2,5}));
        REQUIRE(TestFixture::containsMove(influence, {4,5}));
    }
    
    SECTION("Sentroid Functionality - Player Two") {
        const auto* stats = pdm.getPieceStats("Sentroid");
        REQUIRE(stats != nullptr);
        auto pawn = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_TWO);
        REQUIRE(pawn != nullptr);
        REQUIRE(pawn->getTypeName() == "Sentroid");
        REQUIRE(pawn->getSide() == BayouBonanza::PlayerSide::PLAYER_TWO);
        REQUIRE(pawn->getAttack() == stats->attack);
        REQUIRE(pawn->getHealth() == stats->health);

        board.getSquare(3, 1).setPiece(std::move(pawn)); // Place Sentroid for Player Two
        BayouBonanza::Piece* pawnPtr = board.getSquare(3,1).getPiece();
        REQUIRE(pawnPtr != nullptr);
        pawnPtr->setPosition({3,1});

        auto validMoves = pawnPtr->getValidMoves(board);
        REQUIRE(validMoves.size() == 1);
        REQUIRE(TestFixture::containsMove(validMoves, {3,2})); // Player TWO moves Y+
    }

    SECTION("Sweetykins Functionality - Sliding Piece") {
        auto sweetykins = factory.createPiece("Sweetykins", BayouBonanza::PlayerSide::PLAYER_ONE);
        board.getSquare(0,0).setPiece(std::move(sweetykins));
        BayouBonanza::Piece* sweetykinsPtr = board.getSquare(0,0).getPiece();
        sweetykinsPtr->setPosition({0,0});

        auto validMoves = sweetykinsPtr->getValidMoves(board);
        REQUIRE(validMoves.size() == 14); // 7 horizontal, 7 vertical

        // Place a friendly piece to block
        auto friendlyPawn = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_ONE);
        board.getSquare(0,3).setPiece(std::move(friendlyPawn));
        board.getSquare(0,3).getPiece()->setPosition({0,3});
        
        validMoves = sweetykinsPtr->getValidMoves(board);
        REQUIRE(validMoves.size() == 7 + 2); // 7 horizontal, 2 vertical (0,1), (0,2)
        
        // Place an enemy piece to capture
        auto enemyPawn = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_TWO);
        board.getSquare(3,0).setPiece(std::move(enemyPawn));
        board.getSquare(3,0).getPiece()->setPosition({3,0});

        validMoves = sweetykinsPtr->getValidMoves(board);
         // 2 vertical, 2 horizontal before enemy, 1 for enemy capture square
        REQUIRE(validMoves.size() == 2 + 3);
        REQUIRE(TestFixture::containsMove(validMoves, {3,0})); // Can move to capture
    }
    
    SECTION("Automatick Functionality - Jumping Piece") {
        auto knight = factory.createPiece("Automatick", BayouBonanza::PlayerSide::PLAYER_ONE);
        board.getSquare(1,0).setPiece(std::move(knight));
        BayouBonanza::Piece* knightPtr = board.getSquare(1,0).getPiece();
        knightPtr->setPosition({1,0});

        auto validMoves = knightPtr->getValidMoves(board);
        REQUIRE(validMoves.size() == 3); // From (1,0) on 8x8 board: (0,2), (2,2), (3,1)

        // Place pieces around it, should not affect moves
        auto friendlyPawn = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_ONE);
        board.getSquare(1,1).setPiece(std::move(friendlyPawn));
        board.getSquare(1,1).getPiece()->setPosition({1,1});
        
        auto enemyPawn = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_TWO);
        board.getSquare(0,1).setPiece(std::move(enemyPawn));
        board.getSquare(0,1).getPiece()->setPosition({0,1});

        validMoves = knightPtr->getValidMoves(board);
        REQUIRE(validMoves.size() == 3);
    }

    SECTION("Rustbucket Ranged Attack") {
        GameState gameState;
        MoveExecutor executor;

        auto archer = factory.createPiece("Rustbucket", BayouBonanza::PlayerSide::PLAYER_ONE);
        gameState.getBoard().getSquare(3,3).setPiece(std::move(archer));
        BayouBonanza::Piece* archerPtr = gameState.getBoard().getSquare(3,3).getPiece();
        archerPtr->setPosition({3,3});

        auto enemyPawn = factory.createPiece("Sentroid", BayouBonanza::PlayerSide::PLAYER_TWO);
        gameState.getBoard().getSquare(3,5).setPiece(std::move(enemyPawn));
        gameState.getBoard().getSquare(3,5).getPiece()->setPosition({3,5});

        std::shared_ptr<Piece> archerShared(archerPtr, [](Piece*){});
        Move attackMove(archerShared, {3,3}, {3,5});
        executor.executeMove(gameState, attackMove);

        REQUIRE(gameState.getBoard().getSquare(3,3).getPiece() == archerPtr);
        REQUIRE(gameState.getBoard().getSquare(3,5).isEmpty());
    }
}
