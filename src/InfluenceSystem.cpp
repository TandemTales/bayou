#include "InfluenceSystem.h"
#include "GameBoard.h"
#include "Square.h"
#include "Piece.h"
#include "PlayerSide.h"

namespace BayouBonanza {

void InfluenceSystem::calculateBoardInfluence(GameBoard& board) {
    // Reset all influence values (but NOT control - that's persistent)
    resetInfluenceValues(board);
    
    // Calculate influence for each piece
    calculateAllPieceInfluence(board);
    
    // Update control based on new influence values using sticky control logic
    updateSquareControlFromInfluence(board);
}

void InfluenceSystem::calculatePieceInfluence(GameBoard& board, Position piecePos) {
    // Validate position
    if (!board.isValidPosition(piecePos.x, piecePos.y)) {
        return;
    }
    
    Square& square = board.getSquare(piecePos.x, piecePos.y);
    
    // Check if there's a piece on this square
    if (square.isEmpty()) {
        return;
    }
    
    Piece* piece = square.getPiece();
    PlayerSide pieceSide = piece->getSide();
    
    // Every piece automatically controls its own square
    square.setControlValue(pieceSide, 999); // High value for automatic control
    
    // Every piece influences the 8 adjacent squares with 1 influence point each
    // Adjacent squares: all 8 directions around the piece
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            // Skip the piece's own square (already handled above)
            if (dx == 0 && dy == 0) {
                continue;
            }
            
            int targetX = piecePos.x + dx;
            int targetY = piecePos.y + dy;
            
            // Check if the target square is valid
            if (board.isValidPosition(targetX, targetY)) {
                Square& targetSquare = board.getSquare(targetX, targetY);
                
                // Add 1 influence point to the adjacent square
                int currentInfluence = targetSquare.getControlValue(pieceSide);
                targetSquare.setControlValue(pieceSide, currentInfluence + 1);
            }
        }
    }
}

void InfluenceSystem::calculateAllPieceInfluence(GameBoard& board) {
    // Iterate through all squares and calculate influence for each piece
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            const Square& square = board.getSquare(x, y);
            if (!square.isEmpty()) {
                calculatePieceInfluence(board, Position(x, y));
            }
        }
    }
}

void InfluenceSystem::resetInfluenceValues(GameBoard& board) {
    // Reset all influence values to 0 for both players
    // NOTE: This does NOT reset persistent control - only current influence
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Square& square = board.getSquare(x, y);
            square.setControlValue(PlayerSide::PLAYER_ONE, 0);
            square.setControlValue(PlayerSide::PLAYER_TWO, 0);
        }
    }
}

void InfluenceSystem::updateSquareControlFromInfluence(GameBoard& board) {
    // Update control for each square based on current influence using sticky control logic
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Square& square = board.getSquare(x, y);
            square.updateControlFromInfluence();
        }
    }
}

void InfluenceSystem::determineSquareControl(GameBoard& board) {
    // This method is now replaced by updateSquareControlFromInfluence
    // Keeping for API compatibility
    updateSquareControlFromInfluence(board);
}

PlayerSide InfluenceSystem::getControllingPlayer(const Square& square) {
    return square.getControlledBy();
}

} // namespace BayouBonanza 