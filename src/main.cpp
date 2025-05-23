#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <cctype> // Required for std::tolower

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
                std::string symbol_str = piece->getSymbol();
                symbol = symbol_str.empty() ? '.' : symbol_str[0];
                
                if (piece->getSide() == PlayerSide::PLAYER_TWO) {
                    symbol = std::tolower(symbol);
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

    // Load font
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        std::cerr << "Error loading font from assets/fonts/Roboto-Regular.ttf\n"; // Updated error message
        return -1; // Or handle error appropriately
    }
    
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

    std::shared_ptr<Piece> selectedPiece = nullptr;
    sf::Vector2i originalSquareCoords(-1, -1); // Store board coordinates (x,y)
    sf::Vector2f mouseOffset; // Offset from piece top-left to mouse click position
    bool isPieceSelected = false;
    sf::Vector2f currentMousePosition; // For drawing the piece while dragging
    
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

            // --- Mouse Event Handling ---
            float windowWidth = static_cast<float>(window.getSize().x);
            float windowHeight = static_cast<float>(window.getSize().y);
            float boardSize = std::min(windowWidth, windowHeight) * 0.8f;
            float squareSize = boardSize / GameBoard::BOARD_SIZE;
            float boardStartX = (windowWidth - boardSize) / 2.0f;
            float boardStartY = (windowHeight - boardSize) / 2.0f;

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    // Convert window coordinates to board coordinates
                    int boardX = static_cast<int>((mousePos.x - boardStartX) / squareSize);
                    int boardY = static_cast<int>((mousePos.y - boardStartY) / squareSize);

                    if (boardX >= 0 && boardX < GameBoard::BOARD_SIZE && boardY >= 0 && boardY < GameBoard::BOARD_SIZE)
                    {
                        const Square& square = gameState.getBoard().getSquare(boardX, boardY);
                        if (!square.isEmpty() && square.getPiece()->getSide() == gameState.getActivePlayer())
                        {
                            selectedPiece = square.getPiece();
                            originalSquareCoords = sf::Vector2i(boardX, boardY);
                            isPieceSelected = true;
                            // Calculate offset: mouse_pos - piece_top_left_pos
                            float pieceScreenX = boardStartX + boardX * squareSize;
                            float pieceScreenY = boardStartY + boardY * squareSize;
                            mouseOffset = sf::Vector2f(mousePos.x - pieceScreenX, mousePos.y - pieceScreenY);
                            currentMousePosition = sf::Vector2f(mousePos.x, mousePos.y); // Initialize position
                        }
                    }
                }
            }
            else if (event.type == sf::Event::MouseMoved)
            {
                if (isPieceSelected)
                {
                    currentMousePosition = sf::Vector2f(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
                }
            }
            else if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left && isPieceSelected)
                {
                    // Convert release position to board coordinates
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    int targetX = static_cast<int>((mousePos.x - boardStartX) / squareSize);
                    int targetY = static_cast<int>((mousePos.y - boardStartY) / squareSize);

                    // Check if the target is within board boundaries
                    if (targetX >= 0 && targetX < GameBoard::BOARD_SIZE && targetY >= 0 && targetY < GameBoard::BOARD_SIZE) {
                        // Create Position object for the target
                        Position targetPosition(targetX, targetY); 
                        // Check if the move is valid according to piece logic
                        if (selectedPiece && selectedPiece->getSide() == gameState.getActivePlayer() &&
                            selectedPiece->isValidMove(gameState.getBoard(), targetPosition)) // Corrected call
                        {
                            std::cout << "Move validated. Processing action: " // Changed log message slightly for clarity
                                      << originalSquareCoords.x << "," << originalSquareCoords.y << " -> "
                                      << targetX << "," << targetY << std::endl;
                            
                            // This part should be consistent with previous fixes for TurnManager::processMoveAction
                            Position startPosition(originalSquareCoords.x, originalSquareCoords.y);
                            Move gameMove(selectedPiece, startPosition, targetPosition);
                            turnManager.processMoveAction(gameMove); 
                            
                            std::cout << "Move action processed by TurnManager. Current board state:" << std::endl;
                            printBoardState(gameState); 

                        } else {
                            // Invalid move
                            std::cout << "Invalid move attempt: " // Changed log message slightly
                                      << originalSquareCoords.x << "," << originalSquareCoords.y << " -> "
                                      << targetX << "," << targetY << std::endl;
                            // Piece returns to original square visually (no GameState change was made)
                        }
                    } else {
                        // Target square is off-board
                        std::cout << "Invalid move: Target square is off-board." << std::endl;
                        // Piece returns to original square visually
                    }
                    
                    // Deselect piece regardless of move outcome
                    selectedPiece = nullptr;
                    isPieceSelected = false;
                    originalSquareCoords = sf::Vector2i(-1, -1); 
                }
            }
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
        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                // Skip drawing the selected piece here if it's being dragged
                if (isPieceSelected && x == originalSquareCoords.x && y == originalSquareCoords.y) {
                    continue; 
                }

                const Square& square = board.getSquare(x, y);
                if (!square.isEmpty()) {
                    std::shared_ptr<Piece> piece = square.getPiece();
                    std::string symbol = piece->getSymbol();

                    sf::Text pieceText;
                    pieceText.setFont(font);
                    pieceText.setString(symbol);
                    pieceText.setCharacterSize(static_cast<unsigned int>(squareSize * 0.6f)); // Adjust size as needed

                    if (piece->getSide() == PlayerSide::PLAYER_ONE) {
                        pieceText.setFillColor(sf::Color::Blue);
                    } else {
                        pieceText.setFillColor(sf::Color::Red);
                    }

                    sf::FloatRect textBounds = pieceText.getLocalBounds();
                    pieceText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                                        textBounds.top + textBounds.height / 2.0f);
                    pieceText.setPosition(boardStartX + x * squareSize + squareSize / 2.0f,
                                          boardStartY + y * squareSize + squareSize / 2.0f);

                    window.draw(pieceText);
                }
            }
        }

        // Draw the selected piece at the mouse cursor position if it's being dragged
        if (isPieceSelected && selectedPiece) {
            sf::RectangleShape selectedPieceShape(sf::Vector2f(squareSize, squareSize));
            // Adjust position using mouseOffset so the piece is grabbed correctly
            selectedPieceShape.setPosition(currentMousePosition.x - mouseOffset.x, 
                                         currentMousePosition.y - mouseOffset.y);
            
            if (selectedPiece->getSide() == PlayerSide::PLAYER_ONE) {
                selectedPieceShape.setFillColor(sf::Color(0, 0, 255, 180)); // Blue with some transparency
            } else {
                selectedPieceShape.setFillColor(sf::Color(255, 0, 0, 180)); // Red with some transparency
            }
            window.draw(selectedPieceShape);
        }
        // --- End Piece Rendering ---
        
        // Update the window
        window.display();
    }
    
    return 0;
}
