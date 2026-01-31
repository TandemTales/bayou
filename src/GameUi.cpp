#include "GameUi.h"

#include <algorithm>
#include <cmath>

namespace BayouBonanza {

namespace {
void configureText(sf::Text& text, const sf::Font& font, unsigned int size, const sf::Color& color) {
    text.setFont(font);
    text.setCharacterSize(size);
    text.setFillColor(color);
}
}

GameUiState createGameUiState(const sf::Font& font) {
    GameUiState ui;

    configureText(ui.uiMessageText, font, 24, sf::Color::White);
    ui.uiMessageText.setPosition(10.f, 10.f);

    configureText(ui.localPlayerUsernameText, font, 18, sf::Color::Cyan);
    configureText(ui.localPlayerRatingText, font, 16, sf::Color::White);
    configureText(ui.remotePlayerUsernameText, font, 18, sf::Color::Yellow);
    configureText(ui.remotePlayerRatingText, font, 16, sf::Color::White);
    configureText(ui.localPlayerSteamText, font, 16, sf::Color::White);

    configureText(ui.phaseText, font, 20, sf::Color::Yellow);
    ui.phaseText.setPosition(10.f, 35.f);

    configureText(ui.resultTitleText, font, 48, sf::Color::White);
    configureText(ui.ratingChangeText, font, 32, sf::Color::White);
    configureText(ui.menuButtonText, font, 24, sf::Color::White);

    ui.menuButton.setSize(sf::Vector2f(300.f, 50.f));
    ui.menuButton.setFillColor(sf::Color(100, 100, 100));
    ui.menuButton.setOrigin(ui.menuButton.getSize().x / 2.f, ui.menuButton.getSize().y / 2.f);
    ui.menuButton.setPosition(GraphicsManager::BASE_WIDTH / 2.f,
                              GraphicsManager::BASE_HEIGHT / 2.f + 100.f);

    ui.menuButtonText.setString("Return to Menu");
    sf::FloatRect btnBounds = ui.menuButtonText.getLocalBounds();
    ui.menuButtonText.setOrigin(btnBounds.left + btnBounds.width / 2.f,
                                 btnBounds.top + btnBounds.height / 2.f);
    ui.menuButtonText.setPosition(ui.menuButton.getPosition());

    return ui;
}

void updateUiMessage(GameUiState& uiState, const std::string& message) {
    uiState.uiMessageText.setString(message);
}

void updatePhaseText(GameUiState& uiState, const std::string& phase) {
    uiState.phaseText.setString(phase);
}

void updatePlayerInfo(GameUiState& uiState,
                      PlayerSide localSide,
                      const std::string& localName,
                      int localRating,
                      const std::string& remoteName,
                      int remoteRating) {
    uiState.localPlayerUsernameText.setString("You: " + localName);
    uiState.localPlayerRatingText.setString("Rating: " + std::to_string(localRating));

    uiState.remotePlayerUsernameText.setString("Opponent: " + remoteName);
    uiState.remotePlayerRatingText.setString("Rating: " + std::to_string(remoteRating));

    float baseY = uiState.uiMessageText.getPosition().y + 55.f;
    uiState.localPlayerUsernameText.setPosition(10.f, baseY);
    uiState.localPlayerRatingText.setPosition(10.f, baseY + 20.f);
    uiState.localPlayerSteamText.setPosition(10.f, baseY + 40.f);

    float remoteTextWidthEstimate = 200.f;
    float remoteX = GraphicsManager::BASE_WIDTH - remoteTextWidthEstimate - 10.f;
    uiState.remotePlayerUsernameText.setPosition(remoteX, baseY);
    uiState.remotePlayerRatingText.setPosition(remoteX, baseY + 20.f);

    (void)localSide; // Local side currently unused but kept for future adjustments
}

void updateLocalSteam(GameUiState& uiState, int steam) {
    uiState.localPlayerSteamText.setString("Steam: " + std::to_string(steam));
}

void prepareWinScreen(GameUiState& uiState,
                      PlayerSide winner,
                      PlayerSide localSide,
                      const std::string& opponentName,
                      int& localRating,
                      int opponentRating,
                      const sf::Font& font) {
    uiState.showWinMessage = true;

    bool isDraw = (winner == PlayerSide::NEUTRAL);
    bool isWin = (winner == localSide);
    uiState.endScreenTitle = isDraw ? "Draw" : (isWin ? "Victory!" : "Defeat!");

    if (isDraw) {
        uiState.winMessage = "The game ended in a draw.";
    } else if (isWin) {
        uiState.winMessage = "You defeated " + opponentName + ".";
    } else {
        uiState.winMessage = opponentName + " defeated you.";
    }

    const int K_FACTOR = 32;
    int myAdj = localRating + 1000;
    int oppAdj = opponentRating + 1000;
    double expected = 1.0 / (1.0 + std::pow(10.0, (oppAdj - myAdj) / 400.0));
    double score = isDraw ? 0.5 : (isWin ? 1.0 : 0.0);
    int newAdj = myAdj + static_cast<int>(K_FACTOR * (score - expected));
    int newRating = std::max(0, newAdj - 1000);
    int ratingChange = newRating - localRating;
    localRating = newRating;

    uiState.localPlayerRatingText.setString("Rating: " + std::to_string(localRating));

    uiState.resultTitleText.setString(uiState.endScreenTitle);
    uiState.resultTitleText.setFillColor(isWin ? sf::Color::Green : (isDraw ? sf::Color::White : sf::Color::Red));
    sf::FloatRect titleBounds = uiState.resultTitleText.getLocalBounds();
    uiState.resultTitleText.setOrigin(titleBounds.left + titleBounds.width / 2.f,
                                      titleBounds.top + titleBounds.height / 2.f);
    uiState.resultTitleText.setPosition(GraphicsManager::BASE_WIDTH / 2.f,
                                        GraphicsManager::BASE_HEIGHT / 2.f - 100.f);

    std::string ratingStr = "Rating: " + std::to_string(localRating);
    ratingStr += " (" + std::string(ratingChange >= 0 ? "+" : "") + std::to_string(ratingChange) + ")";
    uiState.ratingChangeText.setString(ratingStr);
    sf::FloatRect ratingBounds = uiState.ratingChangeText.getLocalBounds();
    uiState.ratingChangeText.setOrigin(ratingBounds.left + ratingBounds.width / 2.f,
                                       ratingBounds.top + ratingBounds.height / 2.f);
    uiState.ratingChangeText.setPosition(GraphicsManager::BASE_WIDTH / 2.f,
                                         GraphicsManager::BASE_HEIGHT / 2.f);

    uiState.menuButton.setPosition(GraphicsManager::BASE_WIDTH / 2.f,
                                   GraphicsManager::BASE_HEIGHT / 2.f + 100.f);
    sf::FloatRect btnBounds = uiState.menuButtonText.getLocalBounds();
    uiState.menuButtonText.setOrigin(btnBounds.left + btnBounds.width / 2.f,
                                     btnBounds.top + btnBounds.height / 2.f);
    uiState.menuButtonText.setPosition(uiState.menuButton.getPosition());
}

void drawWinScreen(sf::RenderWindow& window,
                   const GameUiState& uiState,
                   const sf::Font& font) {
    if (!uiState.showWinMessage) {
        return;
    }

    sf::RectangleShape overlay(sf::Vector2f(GraphicsManager::BASE_WIDTH, GraphicsManager::BASE_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    window.draw(uiState.resultTitleText);

    sf::Text descText;
    descText.setFont(font);
    descText.setString(uiState.winMessage);
    descText.setCharacterSize(24);
    descText.setFillColor(sf::Color::White);
    sf::FloatRect dBounds = descText.getLocalBounds();
    descText.setOrigin(dBounds.left + dBounds.width / 2.f,
                       dBounds.top + dBounds.height / 2.f);
    descText.setPosition(GraphicsManager::BASE_WIDTH / 2.f,
                         GraphicsManager::BASE_HEIGHT / 2.f - 30.f);
    window.draw(descText);

    window.draw(uiState.ratingChangeText);
    window.draw(uiState.menuButton);
    window.draw(uiState.menuButtonText);
}

} // namespace BayouBonanza
