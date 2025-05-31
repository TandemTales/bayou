#include <iostream>
#include <memory>
#include "GameBoard.h"
#include "Piece.h"
#include "PieceFactory.h"
#include "GameState.h"
#include "PieceDefinitionManager.h"

int main() {
    std::cout << "Debug Pieces Test Program\n";
    std::cout << "========================\n\n";

    try {
        // Initialize the piece definition manager
        PieceDefinitionManager::getInstance().loadDefinitions("assets/pieces.json");
        std::cout << "✓ Piece definitions loaded successfully\n";

        // Create a game board
        GameBoard board(8, 8);
        std::cout << "✓ Game board created (8x8)\n";

        // Create a game state
        GameState gameState;
        std::cout << "✓ Game state initialized\n";

        // Test piece creation
        std::cout << "\n--- Testing Piece Creation ---\n";
        
        // Create some test pieces
        auto king = PieceFactory::createPiece("King", Position{4, 0}, PieceColor::WHITE);
        if (king) {
            std::cout << "✓ Created White King at (4, 0)\n";
            std::cout << "  Health: " << king->getHealth() << "/" << king->getMaxHealth() << "\n";
            std::cout << "  Attack: " << king->getAttack() << "\n";
        } else {
            std::cout << "✗ Failed to create White King\n";
        }

        auto pawn = PieceFactory::createPiece("Pawn", Position{4, 1}, PieceColor::WHITE);
        if (pawn) {
            std::cout << "✓ Created White Pawn at (4, 1)\n";
            std::cout << "  Health: " << pawn->getHealth() << "/" << pawn->getMaxHealth() << "\n";
            std::cout << "  Attack: " << pawn->getAttack() << "\n";
        } else {
            std::cout << "✗ Failed to create White Pawn\n";
        }

        auto blackKing = PieceFactory::createPiece("King", Position{4, 7}, PieceColor::BLACK);
        if (blackKing) {
            std::cout << "✓ Created Black King at (4, 7)\n";
            std::cout << "  Health: " << blackKing->getHealth() << "/" << blackKing->getMaxHealth() << "\n";
            std::cout << "  Attack: " << blackKing->getAttack() << "\n";
        } else {
            std::cout << "✗ Failed to create Black King\n";
        }

        // Test placing pieces on board
        std::cout << "\n--- Testing Board Placement ---\n";
        if (king && board.placePiece(std::move(king), Position{4, 0})) {
            std::cout << "✓ White King placed on board\n";
        } else {
            std::cout << "✗ Failed to place White King on board\n";
        }

        if (pawn && board.placePiece(std::move(pawn), Position{4, 1})) {
            std::cout << "✓ White Pawn placed on board\n";
        } else {
            std::cout << "✗ Failed to place White Pawn on board\n";
        }

        if (blackKing && board.placePiece(std::move(blackKing), Position{4, 7})) {
            std::cout << "✓ Black King placed on board\n";
        } else {
            std::cout << "✗ Failed to place Black King on board\n";
        }

        // Test getting pieces from board
        std::cout << "\n--- Testing Board Queries ---\n";
        const auto* pieceAtKingPos = board.getPiece(Position{4, 0});
        if (pieceAtKingPos) {
            std::cout << "✓ Found piece at (4, 0): " << pieceAtKingPos->getType() << "\n";
        } else {
            std::cout << "✗ No piece found at (4, 0)\n";
        }

        const auto* pieceAtPawnPos = board.getPiece(Position{4, 1});
        if (pieceAtPawnPos) {
            std::cout << "✓ Found piece at (4, 1): " << pieceAtPawnPos->getType() << "\n";
        } else {
            std::cout << "✗ No piece found at (4, 1)\n";
        }

        // Test empty square
        const auto* emptySquare = board.getPiece(Position{0, 0});
        if (!emptySquare) {
            std::cout << "✓ Correctly found no piece at (0, 0)\n";
        } else {
            std::cout << "✗ Unexpectedly found piece at (0, 0)\n";
        }

        std::cout << "\n--- Debug Test Complete ---\n";
        std::cout << "All basic piece and board operations tested.\n";

    } catch (const std::exception& e) {
        std::cerr << "Error during debug test: " << e.what() << std::endl;
        return 1;
    }

    return 0;
} 