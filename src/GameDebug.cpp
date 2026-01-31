#include "GameDebug.h"
#include <iostream>

#include "GameBoard.h"
#include "Square.h"

namespace BayouBonanza {

void printBoardState(const GameState& gameState, PlayerSide localPlayer ) {
    const GameBoard& board = gameState.getBoard();
    std::cout << "Current board state:" << std::endl;
    for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
            const Square& square = board.getSquare(x, y);
            if (square.isEmpty()) {
                std::cout << ". ";
            } else {
                Piece* piece = square.getPiece();
                std::cout << piece->getSymbol() << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // Print local player's hand if specified
    if (localPlayer !) {
        const Hand& hand = gameState.getHand(localPlayer);

        std::cout << "My Hand (" << hand.size() << " cards, " 
                  << gameState.getSteam(localPlayer) << " steam):" << std::endl;
        for (size_t i = 0; i < hand.size(); ++i) {
            const Card* card = hand.getCard(i);
            if (card) {
                std::cout << "  [" << i << "] " << card->getName() 
                          << " (Cost: " << card->getSteamCost() << ")" << std::endl;
            }
        }
        std::cout << "=====================" << std::endl;
    }
}

} // namespace BayouBonanza
