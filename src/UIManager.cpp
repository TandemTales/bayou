#include "UIManager.h"
#include <iostream>
#include <cmath> // For std::pow in rating calculation

using namespace BayouBonanza;

UIManager::UIManager(sf::RenderWindow& window, sf::Font& font)
    : m_font(font), m_showEndScreen(false)
{
    // Initialize all sf::Text objects with the font
    m_uiMessageText.setFont(m_font);
    m_localPlayerUsernameText.setFont(m_font);
    m_localPlayerRatingText.setFont(m_font);
    m_remotePlayerUsernameText.setFont(m_font);
    m_remotePlayerRatingText.setFont(m_font);
    m_localPlayerSteamText.setFont(m_font);
    m_phaseText.setFont(m_font);
    m_resultTitleText.setFont(m_font);
    m_winMessageText.setFont(m_font);
    m_ratingChangeText.setFont(m_font);
    m_menuButtonText.setFont(m_font);
}

UIManager::~UIManager() {}

void UIManager::setupGameUI() {
    m_uiMessageText.setCharacterSize(24);
    m_uiMessageText.setFillColor(sf::Color::White);
    m_uiMessageText.setPosition(10.f, 10.f);

    m_localPlayerUsernameText.setCharacterSize(18);
    m_localPlayerUsernameText.setFillColor(sf::Color::Cyan);
    m_localPlayerUsernameText.setPosition(10.f, 80.f);

    m_localPlayerRatingText.setCharacterSize(16);
    m_localPlayerRatingText.setFillColor(sf::Color::White);
    m_localPlayerRatingText.setPosition(10.f, 100.f);

    m_localPlayerSteamText.setCharacterSize(16);
    m_localPlayerSteamText.setFillColor(sf::Color::White);
    m_localPlayerSteamText.setPosition(10.f, 120.f);

    m_remotePlayerUsernameText.setCharacterSize(18);
    m_remotePlayerUsernameText.setFillColor(sf::Color::Yellow);
    m_remotePlayerUsernameText.setPosition(GraphicsManager::BASE_WIDTH - 210.f, 80.f);

    m_remotePlayerRatingText.setCharacterSize(16);
    m_remotePlayerRatingText.setFillColor(sf::Color::White);
    m_remotePlayerRatingText.setPosition(GraphicsManager::BASE_WIDTH - 210.f, 100.f);

    m_phaseText.setCharacterSize(20);
    m_phaseText.setFillColor(sf::Color::Yellow);
    m_phaseText.setPosition(10.f, 35.f);
}

void UIManager::update(const GameState& gameState, PlayerSide myPlayerSide, const std::string& uiMessage) {
    m_uiMessageText.setString(uiMessage);

    if (gameState.getGamePhase() != GamePhase::UNINITIALIZED) {
        // Update player info if game is running
        m_localPlayerSteamText.setString("Steam: " + std::to_string(gameState.getSteam(myPlayerSide)));

        std::string phaseStr;
        switch (gameState.getGamePhase()) {
            case GamePhase::SETUP: phaseStr = "Setup"; break;
            case GamePhase::DRAW: phaseStr = "Drawing"; break;
            case GamePhase::PLAY: phaseStr = "Action"; break;
            case GamePhase::MOVE: phaseStr = "Action"; break;
            case GamePhase::GAME_OVER: phaseStr = "Game Over"; break;
            default: phaseStr = "Unknown"; break;
        }
        m_phaseText.setString(phaseStr + " Phase");
    }
}

void UIManager::draw(sf::RenderWindow& window, bool gameHasStarted) {
    window.draw(m_uiMessageText);
    if (gameHasStarted) {
        window.draw(m_phaseText);
        window.draw(m_localPlayerUsernameText);
        window.draw(m_localPlayerRatingText);
        window.draw(m_remotePlayerUsernameText);
        window.draw(m_remotePlayerRatingText);
        window.draw(m_localPlayerSteamText);
    }
}

void UIManager::showEndScreen(PlayerSide winner, const std::string& description, PlayerSide myPlayerSide, const std::string& myUsername, const std::string& opponentUsername, int& myRating, int opponentRating) {
    m_showEndScreen = true;
    bool isDraw = (winner == PlayerSide::NEUTRAL);
    bool isWin = (winner == myPlayerSide);
    std::string endScreenTitle = isDraw ? "Draw" : (isWin ? "Victory!" : "Defeat!");
    std::string winMessage;

    if (isDraw) {
        winMessage = "The game ended in a draw.";
    } else if (isWin) {
        winMessage = "You defeated " + opponentUsername + ".";
    } else {
        winMessage = opponentUsername + " defeated you.";
    }

    // Elo rating calculation
    const int K_FACTOR = 32;
    double expected = 1.0 / (1.0 + std::pow(10.0, (double)(opponentRating - myRating) / 400.0));
    double score = isDraw ? 0.5 : (isWin ? 1.0 : 0.0);
    int newRating = myRating + K_FACTOR * (score - expected);
    int ratingChange = newRating - myRating;
    myRating = newRating;

    // Prepare display texts
    m_resultTitleText.setString(endScreenTitle);
    m_resultTitleText.setCharacterSize(48);
    m_resultTitleText.setFillColor(isWin ? sf::Color::Green : sf::Color::Red);
    sf::FloatRect titleBounds = m_resultTitleText.getLocalBounds();
    m_resultTitleText.setOrigin(titleBounds.left + titleBounds.width / 2.f, titleBounds.top + titleBounds.height / 2.f);
    m_resultTitleText.setPosition(GraphicsManager::BASE_WIDTH / 2.f, GraphicsManager::BASE_HEIGHT / 2.f - 100.f);

    m_winMessageText.setString(winMessage);
    m_winMessageText.setCharacterSize(24);
    m_winMessageText.setFillColor(sf::Color::White);
    sf::FloatRect dBounds = m_winMessageText.getLocalBounds();
    m_winMessageText.setOrigin(dBounds.left + dBounds.width / 2.f, dBounds.top + dBounds.height / 2.f);
    m_winMessageText.setPosition(GraphicsManager::BASE_WIDTH / 2.f, GraphicsManager::BASE_HEIGHT / 2.f - 30.f);

    std::string ratingStr = "Rating: " + std::to_string(myRating) + " (" + (ratingChange >= 0 ? "+" : "") + std::to_string(ratingChange) + ")";
    m_ratingChangeText.setString(ratingStr);
    m_ratingChangeText.setCharacterSize(32);
    m_ratingChangeText.setFillColor(sf::Color::White);
    sf::FloatRect ratingBounds = m_ratingChangeText.getLocalBounds();
    m_ratingChangeText.setOrigin(ratingBounds.left + ratingBounds.width / 2.f, ratingBounds.top + ratingBounds.height / 2.f);
    m_ratingChangeText.setPosition(GraphicsManager::BASE_WIDTH / 2.f, GraphicsManager::BASE_HEIGHT / 2.f);

    m_menuButton.setSize(sf::Vector2f(300.f, 50.f));
    m_menuButton.setFillColor(sf::Color(100, 100, 100));
    m_menuButton.setOrigin(m_menuButton.getSize().x / 2.f, m_menuButton.getSize().y / 2.f);
    m_menuButton.setPosition(GraphicsManager::BASE_WIDTH / 2.f, GraphicsManager::BASE_HEIGHT / 2.f + 100.f);

    m_menuButtonText.setString("Return to Menu");
    m_menuButtonText.setCharacterSize(24);
    m_menuButtonText.setFillColor(sf::Color::White);
    sf::FloatRect btnBounds = m_menuButtonText.getLocalBounds();
    m_menuButtonText.setOrigin(btnBounds.left + btnBounds.width / 2.f, btnBounds.top + btnBounds.height / 2.f);
    m_menuButtonText.setPosition(m_menuButton.getPosition());
}

void UIManager::drawEndScreen(sf::RenderWindow& window) {
    if (!m_showEndScreen) return;

    sf::RectangleShape overlay(sf::Vector2f(GraphicsManager::BASE_WIDTH, GraphicsManager::BASE_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    window.draw(m_resultTitleText);
    window.draw(m_winMessageText);
    window.draw(m_ratingChangeText);
    window.draw(m_menuButton);
    window.draw(m_menuButtonText);
}

bool UIManager::isShowingEndScreen() const {
    return m_showEndScreen;
}

bool UIManager::isEndScreenButtonClicked(sf::Vector2f clickPos) {
    return m_menuButton.getGlobalBounds().contains(clickPos);
}

void UIManager::hideEndScreen() {
    m_showEndScreen = false;
}
