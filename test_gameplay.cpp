#include <iostream>
#include <cctype>
#include "GameState.h"
#include "GameRules.h"
#include "TurnManager.h"
#include "Move.h"
#include "King.h"
#include "Pawn.h"

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
                std::shared_ptr<Piece> piece = square.getPiece();
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
        
        // Test a simple pawn move for Player 1
        std::cout << "\nTesting Player 1 pawn move (e2 to e3)...\n";
        std::cout.flush();
        
        // Get the pawn at position (4, 6) - Player 1's e-pawn
        const Square& pawnSquare = gameState.getBoard().getSquare(4, 6);
        if (!pawnSquare.isEmpty()) {
            std::shared_ptr<Piece> pawn = pawnSquare.getPiece();
            Position from(4, 6);
            Position to(4, 5);
            
            Move pawnMove(pawn, from, to);
            
            bool moveProcessed = false;
            turnManager.processMoveAction(pawnMove, [&](const ActionResult& result) {
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
            std::cout << "No pawn found at position (4, 6)!\n";
            std::cout.flush();
        }
        
        // Test a simple pawn move for Player 2
        std::cout << "\nTesting Player 2 pawn move (e7 to e6)...\n";
        
        // Get the pawn at position (4, 1) - Player 2's e-pawn
        const Square& pawn2Square = gameState.getBoard().getSquare(4, 1);
        if (!pawn2Square.isEmpty()) {
            std::shared_ptr<Piece> pawn = pawn2Square.getPiece();
            Position from(4, 1);
            Position to(4, 2);
            
            Move pawnMove(pawn, from, to);
            
            bool moveProcessed = false;
            turnManager.processMoveAction(pawnMove, [&](const ActionResult& result) {
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
        std::cout << "\nTesting invalid move (Player 1 trying to move Player 2's piece)...\n";
        
        const Square& enemyPawnSquare = gameState.getBoard().getSquare(0, 1);
        if (!enemyPawnSquare.isEmpty()) {
            std::shared_ptr<Piece> enemyPawn = enemyPawnSquare.getPiece();
            Position from(0, 1);
            Position to(0, 2);
            
            Move invalidMove(enemyPawn, from, to);
            
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