#ifndef BAYOU_BONANZA_DECK_EDITOR_SCREEN_H
#define BAYOU_BONANZA_DECK_EDITOR_SCREEN_H

#include <SFML/Graphics.hpp>
#include <SFML/Network/TcpSocket.hpp>

#include "CardCollection.h"
#include "GraphicsManager.h"
#include "PieceDefinitionManager.h"

namespace BayouBonanza {

class Deck;

void runDeckEditor(sf::RenderWindow& window,
                   GraphicsManager& graphicsManager,
                   sf::TcpSocket& socket,
                   CardCollection& collection,
                   Deck& deck,
                   const sf::Font& font,
                   const PieceDefinitionManager& pieceDefinitions);

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_DECK_EDITOR_SCREEN_H
