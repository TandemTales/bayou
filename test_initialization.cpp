#include <iostream>
#include "GameState.h"
#include "GameInitializer.h"
#include "GameBoard.h"
#include "Square.h"
#include "Piece.h"

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
}

int main() {
    std::cout << "Testing Game Initialization\n";
    std::cout << "===========================\n";
    
    try {
        // Create game state and initializer
        GameState gameState;
        GameInitializer initializer;
        
        std::cout << "Before initialization:\n";
        printBoardState(gameState);
        
        // Initialize the game
        std::cout << "\nInitializing game...\n";
        initializer.initializeNewGame(gameState);
        
        std::cout << "\nAfter initialization:\n";
        printBoardState(gameState);
        
        // Count pieces and show details
        int player1Pieces = 0;
        int player2Pieces = 0;
        const GameBoard& board = gameState.getBoard();
        
        std::cout << "\nDetailed piece information:\n";
        for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
                const Square& square = board.getSquare(x, y);
                if (!square.isEmpty()) {
                    Piece* piece = square.getPiece();
                    std::cout << "Position (" << x << "," << y << "): ";
                    std::cout << "Symbol='" << piece->getSymbol() << "' ";
                    std::cout << "Side=" << (piece->getSide() == PlayerSide::PLAYER_ONE ? "P1" : "P2") << " ";
                    std::cout << "Type=" << static_cast<int>(piece->getPieceType()) << std::endl;
                    
                    if (piece->getSide() == PlayerSide::PLAYER_ONE) {
                        player1Pieces++;
                    } else if (piece->getSide() == PlayerSide::PLAYER_TWO) {
                        player2Pieces++;
                    }
                }
            }
        }
        
        std::cout << "\nPiece count:\n";
        std::cout << "Player 1: " << player1Pieces << " pieces\n";
        std::cout << "Player 2: " << player2Pieces << " pieces\n";
        
        if (player1Pieces == 16 && player2Pieces == 16) {
            std::cout << "\n✓ SUCCESS: Both players have 16 pieces as expected!\n";
        } else {
            std::cout << "\n✗ FAILURE: Expected 16 pieces per player\n";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 