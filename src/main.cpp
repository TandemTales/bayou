#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <memory>

#include "GameBoard.h"
#include "GameState.h"
#include "GameRules.h"
#include "TurnManager.h"
#include "GameInitializer.h"
#include "GameOverDetector.h"
#include "King.h"

using namespace BayouBonanza;

// Function to print board state to console for debugging
void printBoardState(const GameState& gameState) {
    const GameBoard& board = gameState.getBoard();
    
    std::cout << "\n  Board State (Turn " << gameState.getTurnNumber() << "):\n";
    std::cout << "  Active Player: " << (gameState.getActivePlayer() == PlayerSide::PLAYER_ONE ? "Player 1" : "Player 2") << "\n\n";
    
    std::cout << "    0 1 2 3 4 5 6 7\n";
    std::cout << "  ----------------\n";
    
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        std::cout << y << " | ";
        
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            const Square& square = board.getSquare(x, y);
            
            char symbol = '.';
            if (!square.isEmpty()) {
                std::shared_ptr<Piece> piece = square.getPiece();
                
                // Use K for kings
                if (dynamic_cast<King*>(piece.get())) {
                    symbol = (piece->getSide() == PlayerSide::PLAYER_ONE) ? 'K' : 'k';
                }
            }
            
            // Print piece symbol and control indicator
            std::cout << symbol << ' ';
        }
        
        std::cout << "|\n";
    }
    
    std::cout << "  ----------------\n";
    std::cout << "  Player 1 Steam: " << gameState.getSteam(PlayerSide::PLAYER_ONE) << "\n";
    std::cout << "  Player 2 Steam: " << gameState.getSteam(PlayerSide::PLAYER_TWO) << "\n";
}

int main()
{
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Bayou Bonanza");
    
    // Set the framerate limit
    window.setFramerateLimit(60);
    
    // Create game components
    GameState gameState;
    GameRules gameRules;
    GameInitializer initializer;
    TurnManager turnManager(gameState, gameRules);
    GameOverDetector gameOverDetector;
    
    // Initialize a new game
    initializer.initializeNewGame(gameState);
    
    // Print initial board state
    std::cout << "Bayou Bonanza - Core Game Engine Demo" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    printBoardState(gameState);
    
    // Main game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window if requested
            if (event.type == sf::Event::Closed)
                window.close();
        }
        
        // Clear screen with a dark green background (bayou-like)
        window.clear(sf::Color(10, 50, 20));
        
        // Draw game elements here
        // Note: Actual game board rendering will be implemented in a future task
        
        // For now, just draw shapes and basic text instead of text with fancy fonts
        // Draw background rectangle
        sf::RectangleShape bgRect(sf::Vector2f(500, 150));
        bgRect.setPosition(150, 220);
        bgRect.setFillColor(sf::Color(30, 80, 40));
        bgRect.setOutlineThickness(4);
        bgRect.setOutlineColor(sf::Color(100, 170, 100));
        window.draw(bgRect);
        
        // Draw circles to represent the game board
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                sf::CircleShape circle(10);
                circle.setPosition(250 + x * 60, 280 + y * 20);
                circle.setFillColor(sf::Color(150, 200, 150));
                window.draw(circle);
            }
        }
        
        // Draw two colored rectangles to represent kings
        sf::RectangleShape king1(sf::Vector2f(20, 20));
        king1.setPosition(250, 280);
        king1.setFillColor(sf::Color::Blue);
        window.draw(king1);
        
        sf::RectangleShape king2(sf::Vector2f(20, 20));
        king2.setPosition(430, 340);
        king2.setFillColor(sf::Color::Red);
        window.draw(king2);
        
        // Update the window
        window.display();
    }
    
    return 0;
}
