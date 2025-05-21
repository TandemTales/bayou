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

        // --- Game Board Rendering ---
        const GameBoard& board = gameState.getBoard();
        float windowWidth = static_cast<float>(window.getSize().x);
        float windowHeight = static_cast<float>(window.getSize().y);
        
        float boardSize = std::min(windowWidth, windowHeight) * 0.8f; // Use 80% of the smaller dimension
        float squareSize = boardSize / GameBoard::BOARD_SIZE;
        float boardStartX = (windowWidth - boardSize) / 2.0f;
        float boardStartY = (windowHeight - boardSize) / 2.0f;

        sf::Color lightSquareColor(170, 210, 130);
        sf::Color darkSquareColor(100, 150, 80);

        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                sf::RectangleShape squareShape(sf::Vector2f(squareSize, squareSize));
                squareShape.setPosition(boardStartX + x * squareSize, boardStartY + y * squareSize);

                // Alternate colors for chessboard pattern
                if ((x + y) % 2 == 0) {
                    squareShape.setFillColor(lightSquareColor);
                } else {
                    squareShape.setFillColor(darkSquareColor);
                }
                window.draw(squareShape);
            }
        }
        // --- End Game Board Rendering ---

        // --- Piece Rendering ---
        float pieceSizeFactor = 0.7f; // Piece is 70% of square size
        float pieceSize = squareSize * pieceSizeFactor;
        float pieceOffset = (squareSize - pieceSize) / 2.0f; // To center the piece

        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                const Square& square = board.getSquare(x, y);
                if (!square.isEmpty()) {
                    std::shared_ptr<Piece> piece = square.getPiece();
                    sf::RectangleShape pieceShape(sf::Vector2f(pieceSize, pieceSize));
                    pieceShape.setPosition(boardStartX + x * squareSize + pieceOffset, 
                                           boardStartY + y * squareSize + pieceOffset);

                    if (piece->getSide() == PlayerSide::PLAYER_ONE) {
                        pieceShape.setFillColor(sf::Color::Blue);
                    } else {
                        pieceShape.setFillColor(sf::Color::Red);
                    }
                    window.draw(pieceShape);
                }
            }
        }
        // --- End Piece Rendering ---
        
        // Update the window
        window.display();
    }
    
    return 0;
}
