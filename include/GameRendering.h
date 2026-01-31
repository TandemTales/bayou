#ifndef BAYOU_BONANZA_GAME_RENDERING_H
#define BAYOU_BONANZA_GAME_RENDERING_H

#include <SFML/Graphics.hpp>

#include "Piece.h"

namespace BayouBonanza {

void renderHealthBar(sf::RenderWindow& window,
                     const Piece* piece,
                     float squareX,
                     float squareY,
                     float squareSize);

void renderAttackValue(sf::RenderWindow& window,
                       const Piece* piece,
                       float squareX,
                       float squareY,
                       float squareSize,
                       const sf::Font& font);

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_GAME_RENDERING_H
