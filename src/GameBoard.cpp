#include "GameBoard.h"

namespace BayouBonanza {

GameBoard::GameBoard() {
    resetBoard();
}

Square& GameBoard::getSquare(int x, int y) {
    return board[y][x];
}

const Square& GameBoard::getSquare(int x, int y) const {
    return board[y][x];
}

bool GameBoard::isValidPosition(int x, int y) const {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

void GameBoard::resetBoard() {
    // Clear all squares
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x] = Square();
        }
    }
}

void GameBoard::recalculateControlValues() {
    // Reset all control values
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            board[y][x].setControlValue(PlayerSide::PLAYER_ONE, 0);
            board[y][x].setControlValue(PlayerSide::PLAYER_TWO, 0);
        }
    }
    
    // Calculate control for each piece on the board
    // This is placeholder logic - the actual implementation will depend on
    // the Piece class which we'll create in later tasks
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            auto& square = board[y][x];
            if (!square.isEmpty()) {
                // TODO: Implement piece influence calculation logic
                // This will be implemented when we have the Piece class
            }
        }
    }
}

} // namespace BayouBonanza
