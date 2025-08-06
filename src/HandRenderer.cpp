#include "HandRenderer.h"
#include "Card.h"

namespace BayouBonanza {

void renderPlayerHand(sf::RenderWindow& window,
                      const GameState& gameState,
                      PlayerSide player,
                      const GraphicsManager& graphicsManager,
                      const sf::Font& font,
                      int selectedCardIndex) {
    const Hand& hand = gameState.getHand(player);
    if (hand.size() == 0) {
        return;
    }

    auto boardParams = graphicsManager.getBoardRenderParams();
    int playerSteam = gameState.getSteam(player);

    float cardWidth = 120.0f;
    float cardHeight = 120.0f;
    float cardSpacing = 10.0f;
    float totalHandWidth = hand.size() * cardWidth + (hand.size() - 1) * cardSpacing;

    float handStartX = (GraphicsManager::BASE_WIDTH - totalHandWidth) / 2.0f;
    float handY = boardParams.boardStartY + boardParams.boardSize + 10.0f;

    for (size_t i = 0; i < hand.size(); ++i) {
        const Card* card = hand.getCard(i);
        if (!card) {
            continue;
        }

        float cardX = handStartX + i * (cardWidth + cardSpacing);

        sf::RectangleShape cardRect(sf::Vector2f(cardWidth, cardHeight));
        cardRect.setPosition(cardX, handY);

        bool canAfford = playerSteam >= card->getSteamCost();
        bool isSelected = static_cast<int>(i) == selectedCardIndex;

        if (isSelected) {
            cardRect.setFillColor(sf::Color(100, 150, 255, 200));
            cardRect.setOutlineColor(sf::Color::Yellow);
            cardRect.setOutlineThickness(3.0f);
        } else if (canAfford) {
            cardRect.setFillColor(sf::Color(60, 80, 60, 180));
            cardRect.setOutlineColor(sf::Color::White);
            cardRect.setOutlineThickness(1.0f);
        } else {
            cardRect.setFillColor(sf::Color(80, 60, 60, 180));
            cardRect.setOutlineColor(sf::Color(128, 128, 128));
            cardRect.setOutlineThickness(1.0f);
        }

        window.draw(cardRect);

        sf::Text nameText;
        nameText.setFont(font);
        nameText.setCharacterSize(14);
        nameText.setFillColor(sf::Color::White);
        nameText.setString(card->getName());
        sf::FloatRect nameBounds = nameText.getLocalBounds();
        nameText.setPosition(cardX + (cardWidth - nameBounds.width) / 2.0f, handY + 10.0f);
        window.draw(nameText);

        sf::Text costText;
        costText.setFont(font);
        costText.setCharacterSize(16);
        costText.setFillColor(canAfford ? sf::Color::Cyan : sf::Color::Red);
        costText.setString("Steam: " + std::to_string(card->getSteamCost()));
        sf::FloatRect costBounds = costText.getLocalBounds();
        costText.setPosition(cardX + (cardWidth - costBounds.width) / 2.0f, handY + cardHeight - 25.0f);
        window.draw(costText);

        sf::Text typeText;
        typeText.setFont(font);
        typeText.setCharacterSize(12);
        typeText.setFillColor(sf::Color::Yellow);
        std::string typeStr = (card->getCardType() == CardType::PIECE_CARD) ? "Piece" : "Effect";
        typeText.setString(typeStr);
        sf::FloatRect typeBounds = typeText.getLocalBounds();
        typeText.setPosition(cardX + (cardWidth - typeBounds.width) / 2.0f, handY + 35.0f);
        window.draw(typeText);
    }
}

} // namespace BayouBonanza
