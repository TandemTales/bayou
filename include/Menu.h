#ifndef BAYOU_BONANZA_MENU_H
#define BAYOU_BONANZA_MENU_H

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include "GraphicsManager.h"
#include "CardCollection.h"
#include "PlayerSide.h"

namespace BayouBonanza {

enum class MainMenuOption {
    DECK_EDITOR,
    PLAY_HUMAN,
    PLAY_AI,
    NONE
};

std::string runLoginScreen(sf::RenderWindow& window,
                           GraphicsManager& graphicsManager,
                           const sf::Font& font);

void showPlaceholderScreen(sf::RenderWindow& window,
                           GraphicsManager& graphicsManager,
                           const std::string& message,
                           const sf::Font& font);

MainMenuOption runMainMenu(sf::RenderWindow& window,
                           GraphicsManager& graphicsManager,
                           sf::TcpSocket& socket,
                           CardCollection& collection,
                           Deck& deck,
                           PlayerSide& playerSide,
                           std::string& username,
                           int& currentRating,
                           sf::Packet& gameStartPacketData,
                           bool& gameStartReceived,
                           const sf::Font& font);

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_MENU_H
