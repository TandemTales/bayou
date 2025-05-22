#include <catch2/catch_test_macros.hpp>
#include "TurnManager.h"
#include "GameState.h"
#include "GameRules.h"
#include "GameInitializer.h"
#include "King.h" 
#include "Pawn.h" 
#include "GameBoard.h"
#include "Piece.h"
#include "Square.h"
#include "Move.h" // Required for Move object

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
        
        Position startPos(3, 0); 
        Position endPos(3, 1);

        std::shared_ptr<Piece> kingToMove = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(kingToMove != nullptr);
        REQUIRE(kingToMove->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(kingToMove->isValidMove(gameState.getBoard(), endPos)); 

        Move gameMove(kingToMove, startPos, endPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).isEmpty() == true);
        REQUIRE(gameState.getBoard().getSquare(endPos.x, endPos.y).getPiece() == kingToMove);
        REQUIRE(kingToMove->getPosition() == endPos);
        REQUIRE(gameState.getActivePlayer() == PlayerSide::PLAYER_TWO);
        REQUIRE(gameState.getTurnNumber() == 2); 
    }

    SECTION("Invalid move: Attempting to move opponent's piece") {
        setupInitialState(gameState, initializer);
        TurnManager turnManager(gameState, gameRules);

        PlayerSide initialPlayer = gameState.getActivePlayer();
        int initialTurnNumber = gameState.getTurnNumber();
        REQUIRE(initialPlayer == PlayerSide::PLAYER_ONE);
        
        Position startPos(3, 7); 
        Position endPos(3, 6);

        std::shared_ptr<Piece> opponentKing = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(opponentKing != nullptr);
        REQUIRE(opponentKing->getSide() == PlayerSide::PLAYER_TWO); 
        
        Move gameMove(opponentKing, startPos, endPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece() == opponentKing); 
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
        
        Position startPos(3, 0); 
        Position invalidEndPos(3, 3); 

        std::shared_ptr<Piece> kingToMove = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(kingToMove != nullptr);
        REQUIRE(kingToMove->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE_FALSE(kingToMove->isValidMove(gameState.getBoard(), invalidEndPos)); 

        Move gameMove(kingToMove, startPos, invalidEndPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece() == kingToMove); 
        REQUIRE(gameState.getBoard().getSquare(invalidEndPos.x, invalidEndPos.y).isEmpty() == true); 
        REQUIRE(gameState.getActivePlayer() == initialPlayer); 
        REQUIRE(gameState.getTurnNumber() == initialTurnNumber); 
    }

    SECTION("Valid move: P1 Pawn forward one step") {
        setupInitialState(gameState, initializer);
        TurnManager turnManager(gameState, gameRules);

        REQUIRE(gameState.getActivePlayer() == PlayerSide::PLAYER_ONE);
        REQUIRE(gameState.getTurnNumber() == 1);

        Position startPos(0, 1); 
        Position endPos(0, 2);
        
        std::shared_ptr<Piece> pawnToMove = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(pawnToMove != nullptr);
        REQUIRE(pawnToMove->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE(pawnToMove->isValidMove(gameState.getBoard(), endPos));

        Move gameMove(pawnToMove, startPos, endPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).isEmpty() == true);
        REQUIRE(gameState.getBoard().getSquare(endPos.x, endPos.y).getPiece() == pawnToMove);
        REQUIRE(pawnToMove->getPosition() == endPos);
        REQUIRE(gameState.getActivePlayer() == PlayerSide::PLAYER_TWO);
        REQUIRE(gameState.getTurnNumber() == 2);
    }
    
    SECTION("Invalid move: P1 Pawn attempting to move backwards") {
        setupInitialState(gameState, initializer);
        TurnManager turnManager(gameState, gameRules);

        PlayerSide initialPlayer = gameState.getActivePlayer();
        int initialTurnNumber = gameState.getTurnNumber(); // Corrected: This was already correct here
        REQUIRE(initialPlayer == PlayerSide::PLAYER_ONE);

        Position startPos(0, 1); 
        Position invalidEndPos(0, 0); 

        std::shared_ptr<Piece> pawnToMove = gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece();
        REQUIRE(pawnToMove != nullptr);
        REQUIRE(pawnToMove->getSide() == PlayerSide::PLAYER_ONE);
        REQUIRE_FALSE(pawnToMove->isValidMove(gameState.getBoard(), invalidEndPos));
        
        Move gameMove(pawnToMove, startPos, invalidEndPos);
        turnManager.processMoveAction(gameMove); 

        REQUIRE(gameState.getBoard().getSquare(startPos.x, startPos.y).getPiece() == pawnToMove);
        REQUIRE(gameState.getActivePlayer() == initialPlayer);
        REQUIRE(gameState.getTurnNumber() == initialTurnNumber); // THIS IS THE LINE THAT HAD THE TYPO
    }
}
