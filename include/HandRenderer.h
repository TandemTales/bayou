#ifndef BAYOU_BONANZA_HAND_RENDERER_H
#define BAYOU_BONANZA_HAND_RENDERER_H

#include <SFML/Graphics.hpp>
#include "GameState.h"
#include "PlayerSide.h"
#include "GraphicsManager.h"

namespace BayouBonanza {

void renderPlayerHand(sf::RenderWindow& window,
                      const GameState& gameState,
                      PlayerSide player,
                      const GraphicsManager& graphicsManager,
                      const sf::Font& font,
                      int selectedCardIndex = -1);

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_HAND_RENDERER_H
