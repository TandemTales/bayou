#include "GameBoard.h"
#include "Square.h" // Ensure Square and its packet operators are included
#include <SFML/Network/Packet.hpp> // For sf::Packet

// GameBoard.h should already include Square.h
// Square.h should already include SFML/Network/Packet.hpp

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

// SFML Packet operators for GameBoard
sf::Packet& operator<<(sf::Packet& packet, const GameBoard& gb) {
    for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
            packet << gb.getSquare(x, y);
        }
    }
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, GameBoard& gb) {
    for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
            // GameBoard::getSquare(x,y) returns a Square&, so we can deserialize directly into it.
            packet >> gb.getSquare(x, y);
        }
    }
    return packet;
}

} // namespace BayouBonanza
