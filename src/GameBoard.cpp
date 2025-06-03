#include "GameBoard.h"
#include "Square.h" // Ensure Square and its packet operators are included
#include "InfluenceSystem.h" // Include the new InfluenceSystem
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
    // Use the new InfluenceSystem to calculate board influence and control
    InfluenceSystem::calculateBoardInfluence(*this);
}

// SFML Packet operators for GameBoard
sf::Packet& operator<<(sf::Packet& packet, const GameBoard& gb) {
    for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
            packet << gb.getSquare(x, y); // Square::operator<< does not need factory
        }
    }
    return packet;
}

// Updated to match signature without PieceFactory
sf::Packet& operator>>(sf::Packet& packet, GameBoard& gb) {
    for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
            // GameBoard::getSquare(x,y) returns a Square&.
            packet >> gb.getSquare(x, y);
        }
    }
    return packet;
}

} // namespace BayouBonanza
