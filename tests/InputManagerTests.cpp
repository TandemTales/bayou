#include <catch2/catch_test_macros.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "InputManager.h"
#include "GameState.h"
#include "GameInitializer.h"

using namespace BayouBonanza;

TEST_CASE("InputManager Construction", "[InputManager]") {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test Window");
    sf::TcpSocket socket;
    GameState gameState;
    bool gameHasStarted = false;
    PlayerSide myPlayerSide = PlayerSide::PLAYER_ONE;
    
    REQUIRE_NOTHROW(InputManager(window, socket, gameState, gameHasStarted, myPlayerSide));
}

TEST_CASE("InputManager Initial State", "[InputManager]") {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test Window");
    sf::TcpSocket socket;
    GameState gameState;
    bool gameHasStarted = false;
    PlayerSide myPlayerSide = PlayerSide::PLAYER_ONE;
    
    InputManager inputManager(window, socket, gameState, gameHasStarted, myPlayerSide);
    
    SECTION("Initial state should have no piece selected") {
        REQUIRE(inputManager.getSelectedPiece() == nullptr);
        REQUIRE_FALSE(inputManager.isPieceSelected());
        REQUIRE(inputManager.getOriginalSquareCoords() == sf::Vector2i(-1, -1));
        REQUIRE(inputManager.getCurrentMousePosition() == sf::Vector2f(0.f, 0.f));
        REQUIRE(inputManager.getMouseOffset() == sf::Vector2f(0.f, 0.f));
    }
}

TEST_CASE("InputManager Reset State", "[InputManager]") {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test Window");
    sf::TcpSocket socket;
    GameState gameState;
    bool gameHasStarted = false;
    PlayerSide myPlayerSide = PlayerSide::PLAYER_ONE;
    
    InputManager inputManager(window, socket, gameState, gameHasStarted, myPlayerSide);
    
    SECTION("Reset should clear all state") {
        inputManager.resetInputState();
        
        REQUIRE(inputManager.getSelectedPiece() == nullptr);
        REQUIRE_FALSE(inputManager.isPieceSelected());
        REQUIRE(inputManager.getOriginalSquareCoords() == sf::Vector2i(-1, -1));
        REQUIRE(inputManager.getCurrentMousePosition() == sf::Vector2f(0.f, 0.f));
        REQUIRE(inputManager.getMouseOffset() == sf::Vector2f(0.f, 0.f));
    }
}

TEST_CASE("InputManager Event Handling", "[InputManager]") {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Test Window");
    sf::TcpSocket socket;
    GameState gameState;
    bool gameHasStarted = true;
    PlayerSide myPlayerSide = PlayerSide::PLAYER_ONE;
    
    // Initialize game state with pieces
    GameInitializer initializer;
    initializer.initializeNewGame(gameState);
    
    InputManager inputManager(window, socket, gameState, gameHasStarted, myPlayerSide);
    
    SECTION("Should handle mouse events") {
        sf::Event mouseEvent;
        mouseEvent.type = sf::Event::MouseButtonPressed;
        mouseEvent.mouseButton.button = sf::Mouse::Left;
        mouseEvent.mouseButton.x = 100;
        mouseEvent.mouseButton.y = 100;
        
        REQUIRE(inputManager.handleEvent(mouseEvent));
    }
    
    SECTION("Should not handle non-mouse events") {
        sf::Event keyEvent;
        keyEvent.type = sf::Event::KeyPressed;
        keyEvent.key.code = sf::Keyboard::Space;
        
        REQUIRE_FALSE(inputManager.handleEvent(keyEvent));
    }
    
    SECTION("Should handle mouse movement") {
        sf::Event moveEvent;
        moveEvent.type = sf::Event::MouseMoved;
        moveEvent.mouseMove.x = 200;
        moveEvent.mouseMove.y = 200;
        
        REQUIRE(inputManager.handleEvent(moveEvent));
    }
    
    SECTION("Should handle mouse release") {
        sf::Event releaseEvent;
        releaseEvent.type = sf::Event::MouseButtonReleased;
        releaseEvent.mouseButton.button = sf::Mouse::Left;
        releaseEvent.mouseButton.x = 150;
        releaseEvent.mouseButton.y = 150;
        
        REQUIRE(inputManager.handleEvent(releaseEvent));
    }
} 