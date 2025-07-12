#include <catch2/catch_test_macros.hpp>
#include "TurnManager.h"
#include "GameState.h"
#include "GameRules.h"
#include "GameInitializer.h"
// #include "King.h" // Removed - using data-driven approach with PieceFactory
// #include "Pawn.h" // Removed - using data-driven approach with PieceFactory
#include "GameBoard.h"
#include "Piece.h"
#include "Square.h"
#include "Move.h" // Required for Move object
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"

using namespace BayouBonanza;

// Helper function to reset game state for each test section
static void setupInitialState(GameState& gs, GameInitializer& init) {
    gs = GameState(); // Reset to default constructor
    init.initializeNewGame(gs);
}

TEST_CASE("TurnManager functionality", "[turnmanager]") {
    GameState gameState;
    GameRules gameRules;
    GameInitializer initializer;
    // TurnManager is created inside sections after setup

    SECTION("Valid move processing, turn switching, and turn number increment") {
        setupInitialState(gameState, initializer);
        TurnManager turnManager(gameState, gameRules);

        REQUIRE(gameState.getActivePlayer() == PlayerSide::PLAYER_ONE);
        REQUIRE(gameState.getTurnNumber() == 1);
        
        // Use TinkeringTom move since it's the only piece on the board
        Position startPos(4, 7); // Player One TinkeringTom position
        Position endPos(4, 6);   // Move forward one square

        Piece* tomToMove = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(tomToMove != nullptr);
        REQUIRE(tomToMove->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(tomToMove->isValidMove(gameState.getBoard(), endPos)); 

        // Create a temporary shared_ptr wrapper for the Move constructor
        std::shared_ptr<Piece> tomPtr(tomToMove, [](Piece*){});
        Move gameMove(tomPtr, startPos, endPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).isEmpty() == true);
        REQUIRE(gameState.getBoard().getSquare(endPos.x, endPos.y).getPiece() == tomToMove);
        REQUIRE(tomToMove->getPosition() == endPos);
        REQUIRE(gameState.getActivePlayer() == PlayerSide::PLAYER_TWO);
        REQUIRE(gameState.getTurnNumber() == 2); 
    }

    SECTION("Invalid move: Attempting to move opponent's piece") {
        setupInitialState(gameState, initializer);
        TurnManager turnManager(gameState, gameRules);

        PlayerSide initialPlayer = gameState.getActivePlayer();
        int initialTurnNumber = gameState.getTurnNumber();
        REQUIRE(initialPlayer == PlayerSide::PLAYER_ONE);
        
        Position startPos(4, 0); // Player Two TinkeringTom position
        Position endPos(4, 2);   // Move to empty square

        Piece* opponentTom = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(opponentTom != nullptr);
        REQUIRE(opponentTom->getSide() == PlayerSide::PLAYER_TWO); 
        
        // Create a temporary shared_ptr wrapper for the Move constructor
        std::shared_ptr<Piece> tomPtr(opponentTom, [](Piece*){});
        Move gameMove(tomPtr, startPos, endPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece() == opponentTom); 
        REQUIRE(gameState.getBoard().getSquare(endPos.x, endPos.y).isEmpty() == true);
        REQUIRE(gameState.getActivePlayer() == initialPlayer); 
        REQUIRE(gameState.getTurnNumber() == initialTurnNumber); 
    }

    SECTION("Invalid move: Move rejected by piece's isValidMove logic") {
        setupInitialState(gameState, initializer);
        TurnManager turnManager(gameState, gameRules);

        PlayerSide initialPlayer = gameState.getActivePlayer();
        int initialTurnNumber = gameState.getTurnNumber();
        REQUIRE(initialPlayer == PlayerSide::PLAYER_ONE);
        
        Position startPos(4, 7);     // Player One TinkeringTom position
        Position invalidEndPos(4, 4); // Invalid move - too far

        Piece* tomToMove = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(tomToMove != nullptr);
        REQUIRE(tomToMove->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE_FALSE(tomToMove->isValidMove(gameState.getBoard(), invalidEndPos)); 

        // Create a temporary shared_ptr wrapper for the Move constructor
        std::shared_ptr<Piece> tomPtr(tomToMove, [](Piece*){});
        Move gameMove(tomPtr, startPos, invalidEndPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece() == tomToMove); 
        REQUIRE(gameState.getBoard().getSquare(invalidEndPos.x, invalidEndPos.y).isEmpty() == true); 
        REQUIRE(gameState.getActivePlayer() == initialPlayer); 
        REQUIRE(gameState.getTurnNumber() == initialTurnNumber); 
    }

    SECTION("Valid move: P1 TinkeringTom forward one step") {
        setupInitialState(gameState, initializer);
        TurnManager turnManager(gameState, gameRules);

        REQUIRE(gameState.getActivePlayer() == PlayerSide::PLAYER_ONE);
        REQUIRE(gameState.getTurnNumber() == 1);

        Position startPos(4, 7); // Player One TinkeringTom position
        Position endPos(4, 6);   // Move forward one square
        
        Piece* tomToMove = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(tomToMove != nullptr);
        REQUIRE(tomToMove->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(tomToMove->isValidMove(gameState.getBoard(), endPos));

        // Create a temporary shared_ptr wrapper for the Move constructor
        std::shared_ptr<Piece> tomPtr(tomToMove, [](Piece*){});
        Move gameMove(tomPtr, startPos, endPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).isEmpty() == true);
        REQUIRE(gameState.getBoard().getSquare(endPos.x, endPos.y).getPiece() == tomToMove);
        REQUIRE(tomToMove->getPosition() == endPos);
        REQUIRE(gameState.getActivePlayer() == PlayerSide::PLAYER_TWO);
        REQUIRE(gameState.getTurnNumber() == 2);
    }
    
    SECTION("Invalid move: P1 TinkeringTom attempting to move too far") {
        setupInitialState(gameState, initializer);
        TurnManager turnManager(gameState, gameRules);

        PlayerSide initialPlayer = gameState.getActivePlayer();
        int initialTurnNumber = gameState.getTurnNumber();
        REQUIRE(initialPlayer == PlayerSide::PLAYER_ONE);

        Position startPos(4, 7);     // Player One TinkeringTom position
        Position invalidEndPos(4, 5); // Invalid move - too far (2 squares)

        Piece* tomToMove = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(tomToMove != nullptr);
        REQUIRE(tomToMove->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE_FALSE(tomToMove->isValidMove(gameState.getBoard(), invalidEndPos));
        
        // Create a temporary shared_ptr wrapper for the Move constructor
        std::shared_ptr<Piece> tomPtr(tomToMove, [](Piece*){});
        Move gameMove(tomPtr, startPos, invalidEndPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece() == tomToMove);
        REQUIRE(gameState.getActivePlayer() == initialPlayer);
        REQUIRE(gameState.getTurnNumber() == initialTurnNumber);
    }
}
