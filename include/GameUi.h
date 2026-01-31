#ifndef BAYOU_BONANZA_GAME_UI_H
#define BAYOU_BONANZA_GAME_UI_H

#include <SFML/Graphics.hpp>

#include <string>

#include "GraphicsManager.h"
#include "PlayerSide.h"

namespace BayouBonanza {

struct GameUiState {
    sf::Text uiMessageText;
    sf::Text localPlayerUsernameText;
    sf::Text localPlayerRatingText;
    sf::Text remotePlayerUsernameText;
    sf::Text remotePlayerRatingText;
    sf::Text localPlayerSteamText;
    sf::Text phaseText;
    sf::Text resultTitleText;
    sf::Text ratingChangeText;
    sf::RectangleShape menuButton;
    sf::Text menuButtonText;
    bool showWinMessage{false};
    std::string winMessage;
    std::string endScreenTitle;
};

GameUiState createGameUiState(const sf::Font& font);

void updateUiMessage(GameUiState& uiState, const std::string& message);

void updatePhaseText(GameUiState& uiState, const std::string& phase);

void updatePlayerInfo(GameUiState& uiState,
                      PlayerSide localSide,
                      const std::string& localName,
                      int localRating,
                      const std::string& remoteName,
                      int remoteRating);

void updateLocalSteam(GameUiState& uiState, int steam);

void prepareWinScreen(GameUiState& uiState,
                      PlayerSide winner,
                      PlayerSide localSide,
                      const std::string& opponentName,
                      int& localRating,
                      int opponentRating,
                      const sf::Font& font);

void drawWinScreen(sf::RenderWindow& window,
                   const GameUiState& uiState,
                   const sf::Font& font);

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_GAME_UI_H
