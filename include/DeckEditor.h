#pragma once

#include <SFML/Graphics.hpp>
#include "GraphicsManager.h"
#include "CardCollection.h"
#include "Deck.h"
#include "PieceDefinitionManager.h"
#include "NetworkManager.h"

namespace BayouBonanza {

class DeckEditor {
public:
    DeckEditor(sf::RenderWindow& window, GraphicsManager& graphicsManager, NetworkManager& networkManager, const sf::Font& font, CardCollection& collection, Deck& deck, PieceDefinitionManager& defManager);
    ~DeckEditor();

    void run();

private:
    void processEvents();
    void update();
    void render();
    void sendDeckToServer();

    sf::RenderWindow& m_window;
    GraphicsManager& m_graphicsManager;
    NetworkManager& m_networkManager;
    const sf::Font& m_font;
    CardCollection& m_collection;
    Deck& m_deck;
    PieceDefinitionManager& m_defManager;

    // Deck editor specific state
    bool m_dragging;
    bool m_actualDrag;
    size_t m_dragIndex;
    sf::Vector2f m_dragOffset;
    sf::Vector2f m_dragStartPos;

    float m_collectionScroll;

    std::string m_statusMessage;
    sf::Color m_statusColor;
    sf::Clock m_statusClock;
};

} // namespace BayouBonanza
