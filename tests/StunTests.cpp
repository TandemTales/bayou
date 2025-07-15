#include <catch2/catch_test_macros.hpp>
#include "PieceDefinitionManager.h"
#include "PieceFactory.h"
#include "GameState.h"
#include "MoveExecutor.h"
#include "Move.h"

using namespace BayouBonanza;

TEST_CASE("Stun and cooldown mechanics") {
    PieceDefinitionManager pdm;
    bool loaded = pdm.loadDefinitions("assets/data/cards.json");
    if (!loaded) {
        loaded = pdm.loadDefinitions("../../assets/data/cards.json");
    }
    REQUIRE(loaded);
    PieceFactory factory(pdm);

    GameState state;
    GameBoard& board = state.getBoard();

    auto attacker = factory.createPiece("Rustbucket", PlayerSide::PLAYER_ONE);
    auto defender = factory.createPiece("TinkeringTom", PlayerSide::PLAYER_TWO);
    REQUIRE(attacker);
    REQUIRE(defender);
    REQUIRE(attacker->getCooldown() == 1);

    attacker->setPosition({0,0});
    defender->setPosition({0,1});
    board.getSquare(0,0).setPiece(std::move(attacker));
    board.getSquare(0,1).setPiece(std::move(defender));

    std::shared_ptr<Piece> attackerPtr(board.getSquare(0,0).getPiece(), [](Piece*){});
    Move move(attackerPtr, {0,0}, {0,1});
    MoveExecutor exec;
    auto result = exec.executeMove(state, move);
    REQUIRE(result == MoveResult::SUCCESS);

    Piece* defPiece = board.getSquare(0,1).getPiece();
    Piece* attPiece = board.getSquare(0,0).getPiece();
    REQUIRE(defPiece);
    REQUIRE(attPiece);

    CHECK(defPiece->isStunned());
    CHECK(defPiece->getStunRemaining() == 2);
    CHECK(attPiece->isStunned());
    CHECK(attPiece->getStunRemaining() == 1);

    state.setActivePlayer(PlayerSide::PLAYER_ONE);
    state.processTurnStart();
    // attacker stun decremented
    CHECK(attPiece->getStunRemaining() == 0);
    // defender not decremented yet (other player)
    CHECK(defPiece->getStunRemaining() == 2);

    state.setActivePlayer(PlayerSide::PLAYER_TWO);
    state.processTurnStart();
    CHECK(defPiece->getStunRemaining() == 1);
    CHECK(defPiece->isStunned());
}
