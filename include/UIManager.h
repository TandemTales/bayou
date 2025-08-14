#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "PlayerSide.h"
#include "GameState.h" // For game state access
#include "GraphicsManager.h"

namespace BayouBonanza {

class UIManager {
public:
    UIManager(sf::RenderWindow& window, sf::Font& font);
    ~UIManager();

    void setupGameUI();
    void update(const GameState& gameState, PlayerSide myPlayerSide, const std::string& uiMessage);
    void draw(sf::RenderWindow& window, bool gameHasStarted);

    void showEndScreen(PlayerSide winner, const std::string& description, PlayerSide myPlayerSide, const std::string& myUsername, const std::string& opponentUsername, int& myRating, int opponentRating);
    void drawEndScreen(sf::RenderWindow& window);
    bool isShowingEndScreen() const;

    // To handle clicks on the "Return to Menu" button
    bool isEndScreenButtonClicked(sf::Vector2f clickPos);
    void hideEndScreen();


private:
    sf::Font& m_font;
    sf::Text m_uiMessageText;
    sf::Text m_localPlayerUsernameText;
    sf::Text m_localPlayerRatingText;
    sf::Text m_remotePlayerUsernameText;
    sf::Text m_remotePlayerRatingText;
    sf::Text m_localPlayerSteamText;
    sf::Text m_phaseText;

    // End screen elements
    bool m_showEndScreen;
    sf::Text m_resultTitleText;
    sf::Text m_winMessageText;
    sf::Text m_ratingChangeText;
    sf::RectangleShape m_menuButton;
    sf::Text m_menuButtonText;
};

} // namespace BayouBonanza
