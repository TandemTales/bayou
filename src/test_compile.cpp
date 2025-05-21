#include <iostream>
#include <string>
#include <memory>

#include "GameBoard.h"
#include "Square.h"
#include "Piece.h"
#include "King.h"
#include "GameState.h"
#include "PlayerSide.h"

// Simple test program to verify core classes compile correctly
int main() {
    std::cout << "Testing compilation of core classes..." << std::endl;
    
    // Create a board
    BayouBonanza::GameBoard board;
    
    // Create a king
    std::shared_ptr<BayouBonanza::King> king = 
        std::make_shared<BayouBonanza::King>(BayouBonanza::PlayerSide::PLAYER_ONE);
    
    // Place it on the board
    board.getSquare(4, 7).setPiece(king);
    king->setPosition(BayouBonanza::Position(4, 7));
    
    // Print information
    std::cout << "King placed at position (4, 7)" << std::endl;
    std::cout << "Type: " << king->getTypeName() << std::endl;
    std::cout << "Health: " << king->getHealth() << std::endl;
    std::cout << "Attack: " << king->getAttack() << std::endl;
    
    return 0;
}
