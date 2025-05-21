#include "Piece.h"
#include "GameBoard.h"

namespace BayouBonanza {

Piece::Piece(PlayerSide side, int attack, int health) :
    side(side),
    attack(attack),
    health(health),
    position(-1, -1) {
}

PlayerSide Piece::getSide() const {
    return side;
}

int Piece::getAttack() const {
    return attack;
}

int Piece::getHealth() const {
    return health;
}

void Piece::setHealth(int health) {
    this->health = health;
}

bool Piece::takeDamage(int damage) {
    health -= damage;
    return health <= 0;
}

Position Piece::getPosition() const {
    return position;
}

void Piece::setPosition(const Position& pos) {
    position = pos;
}

std::vector<Position> Piece::getInfluenceArea(const GameBoard& board) const {
    // By default, a piece influences its own square and all adjacent squares
    std::vector<Position> influenceArea;
    
    // Add the piece's own position
    if (board.isValidPosition(position.x, position.y)) {
        influenceArea.push_back(position);
    }
    
    // Add all adjacent squares
    const int directions[8][2] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1, 0},           {1, 0},
        {-1, 1},  {0, 1},  {1, 1}
    };
    
    for (const auto& dir : directions) {
        int x = position.x + dir[0];
        int y = position.y + dir[1];
        
        if (board.isValidPosition(x, y)) {
            influenceArea.push_back(Position(x, y));
        }
    }
    
    return influenceArea;
}

} // namespace BayouBonanza
