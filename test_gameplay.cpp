#include <iostream>
#include <memory>
#include "GameState.h"
#include "GameInitializer.h"
#include "TurnManager.h"
#include "Move.h"
// #include "Pawn.h" // Removed - using data-driven approach with PieceFactory
#include "GameBoard.h"
#include "Square.h"
#include "Piece.h"
#include "PieceFactory.h"
#include "PlayerSide.h"

using namespace BayouBonanza;

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
                Piece* piece = square.getPiece();
                std::string symbol_str = piece->getSymbol();
                symbol = symbol_str.empty() ? '.' : symbol_str[0];
                
                if (piece->getSide() == PlayerSide::PLAYER_TWO) {
                    symbol = std::tolower(symbol);
                }
            }
            
            std::cout << symbol << ' ';
        }
        
        std::cout << "|\n";
    }
    
    std::cout << "  ----------------\n";
    std::cout << "  Player 1 Steam: " << gameState.getSteam(PlayerSide::PLAYER_ONE) << "\n";
    std::cout << "  Player 2 Steam: " << gameState.getSteam(PlayerSide::PLAYER_TWO) << "\n";
    std::cout.flush(); // Force output
}

int main() {
    std::cout << "Testing Bayou Bonanza Gameplay Loop\n";
    std::cout << "===================================\n";
    std::cout.flush();
    
    try {
        // Create game components
        std::cout << "Creating game components...\n";
        std::cout.flush();
        
        GameState gameState;
        GameRules gameRules;
        TurnManager turnManager(gameState, gameRules);
        
        // Initialize the game
        std::cout << "Initializing game...\n";
        std::cout.flush();
        
        turnManager.startNewGame();
        
        std::cout << "Game initialized successfully!\n";
        std::cout.flush();
        
        // Print initial board state
        printBoardState(gameState);
        
        // Test a simple TinkeringTom move for Player 1
        std::cout << "\nTesting Player 1 TinkeringTom move (e8 to e7)...\n";
        std::cout.flush();
        
        // Get the TinkeringTom at position (4, 7) - Player 1's TinkeringTom
        const Square& tomSquare = gameState.getBoard().getSquare(4, 7);
        if (!tomSquare.isEmpty()) {
            Piece* tom = tomSquare.getPiece();
            Position from(4, 7);
            Position to(4, 6);
            
            // Create a temporary shared_ptr wrapper for the Move constructor
            std::shared_ptr<Piece> tomPtr(tom, [](Piece*){});
            Move tomMove(tomPtr, from, to);
            
            bool moveProcessed = false;
            turnManager.processMoveAction(tomMove, [&](const ActionResult& result) {
                moveProcessed = true;
                std::cout << "Move result: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
                std::cout << "Message: " << result.message << "\n";
                std::cout.flush();
            });
            
            if (moveProcessed) {
                printBoardState(gameState);
            } else {
                std::cout << "Move was not processed!\n";
                std::cout.flush();
            }
        } else {
            std::cout << "No TinkeringTom found at position (4, 7)!\n";
            std::cout.flush();
        }
        
        // Test a simple TinkeringTom move for Player 2
        std::cout << "\nTesting Player 2 TinkeringTom move (e1 to e2)...\n";
        
        // Get the TinkeringTom at position (4, 0) - Player 2's TinkeringTom
        const Square& tom2Square = gameState.getBoard().getSquare(4, 0);
        if (!tom2Square.isEmpty()) {
            Piece* tom = tom2Square.getPiece();
            Position from(4, 0);
            Position to(4, 1);
            
            // Create a temporary shared_ptr wrapper for the Move constructor
            std::shared_ptr<Piece> tomPtr(tom, [](Piece*){});
            Move tomMove(tomPtr, from, to);
            
            bool moveProcessed = false;
            turnManager.processMoveAction(tomMove, [&](const ActionResult& result) {
                moveProcessed = true;
                std::cout << "Move result: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
                std::cout << "Message: " << result.message << "\n";
                std::cout.flush();
            });
            
            if (moveProcessed) {
                printBoardState(gameState);
            }
        }
        
        // Test an invalid move
        std::cout << "\nTesting invalid move (Player 1 trying to move Player 2's TinkeringTom)...\n";
        
        const Square& enemyTomSquare = gameState.getBoard().getSquare(4, 0);
        if (!enemyTomSquare.isEmpty()) {
            Piece* enemyTom = enemyTomSquare.getPiece();
            Position from(4, 0);
            Position to(4, 1);
            
            // Create a temporary shared_ptr wrapper for the Move constructor
            std::shared_ptr<Piece> enemyTomPtr(enemyTom, [](Piece*){});
            Move invalidMove(enemyTomPtr, from, to);
            
            bool moveProcessed = false;
            turnManager.processMoveAction(invalidMove, [&](const ActionResult& result) {
                moveProcessed = true;
                std::cout << "Move result: " << (result.success ? "SUCCESS" : "FAILED") << "\n";
                std::cout << "Message: " << result.message << "\n";
                std::cout.flush();
            });
        }
        
        std::cout << "\nGameplay test completed successfully!\n";
        std::cout.flush();
        
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << "\n";
        std::cout.flush();
        return 1;
    } catch (...) {
        std::cout << "Unknown exception caught!\n";
        std::cout.flush();
        return 1;
    }
    
    return 0;
} 