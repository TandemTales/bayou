#include "DeckEditorScreen.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Network/Packet.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

#include "Card.h"
#include "NetworkProtocol.h"
#include "PieceCard.h"

namespace BayouBonanza {

void runDeckEditor(sf::RenderWindow& window,
                   GraphicsManager& graphicsManager,
                   sf::TcpSocket& socket,
                   CardCollection& collection,
                   Deck& deck,
                   const sf::Font& font,
                   const PieceDefinitionManager& pieceDefinitions) {
    // --- Layout constants ---
    const float CARD_W = 100.f;
    const float CARD_H = 140.f;
    const float CARD_SPACING = 10.f;
    const int   COLLECTION_COLS = 10;
    const float ROW_HEIGHT = CARD_H + CARD_SPACING;

    // New layout: Collection is now a single horizontally scrollable row
    float collectionY = 35.f; // More spacing at the top
    float collectionStartX = 30.f; // More margin from left edge
    // Calculate width to show exactly 11 cards (no partial cards)
    const int VISIBLE_COLLECTION_CARDS = 11;
    float collectionAreaWidth = VISIBLE_COLLECTION_CARDS * CARD_W + (VISIBLE_COLLECTION_CARDS - 1) * CARD_SPACING;
    
    // Deck area moved up since collection takes less vertical space
    float deckY = collectionY + CARD_H + 50.f; // More spacing between collection and deck
    float deckStartX = (GraphicsManager::BASE_WIDTH - (CARD_W * 10 + CARD_SPACING * 9)) / 2.f;
    
    // Victory area moved up with more space available
    float victoryY = deckY + ROW_HEIGHT * 2 + 35.f; // More spacing between deck and victory
    float victoryWidth = CARD_W * Deck::VICTORY_SIZE + CARD_SPACING * (Deck::VICTORY_SIZE - 1);
    float victoryStartX = deckStartX + ((CARD_W * 10 + CARD_SPACING * 9) - victoryWidth) / 2.f;

    float collectionScroll = 0.f; // Now horizontal scroll

    // Helper function to wrap text within card bounds
    auto drawWrappedText = [&](const std::string& text, float cardX, float cardY, sf::Color color) {
        const float TEXT_MARGIN = 5.f;
        const float MAX_TEXT_WIDTH = CARD_W - (TEXT_MARGIN * 2);
        const float MAX_TEXT_HEIGHT = CARD_H - (TEXT_MARGIN * 2);
        const int FONT_SIZE = 10; // Smaller font size
        const float LINE_SPACING = FONT_SIZE + 2;
        
        sf::Text testText;
        testText.setFont(font);
        testText.setCharacterSize(FONT_SIZE);
        
        std::vector<std::string> lines;
        std::string currentLine = "";
        std::istringstream words(text);
        std::string word;
        
        // Word wrapping logic
        while (words >> word) {
            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
            testText.setString(testLine);
            
            if (testText.getLocalBounds().width <= MAX_TEXT_WIDTH) {
                currentLine = testLine;
            } else {
                if (!currentLine.empty()) {
                    lines.push_back(currentLine);
                    currentLine = word;
                } else {
                    // Single word is too long, truncate it
                    lines.push_back(word.substr(0, std::min(word.length(), size_t(8))) + "...");
                    currentLine = "";
                }
            }
        }
        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }
        
        // Draw the wrapped lines
        float totalTextHeight = lines.size() * LINE_SPACING;
        float startY = cardY + (CARD_H - totalTextHeight) / 2.f;
        
        for (size_t lineIdx = 0; lineIdx < lines.size() && (lineIdx * LINE_SPACING) < MAX_TEXT_HEIGHT; ++lineIdx) {
            sf::Text lineText;
            lineText.setFont(font);
            lineText.setCharacterSize(FONT_SIZE);
            lineText.setFillColor(color);
            lineText.setString(lines[lineIdx]);
            
            sf::FloatRect bounds = lineText.getLocalBounds();
            lineText.setPosition(cardX + (CARD_W - bounds.width) / 2.f, startY + lineIdx * LINE_SPACING);
            window.draw(lineText);
        }
    };

    // Drag state for collection cards
    bool dragging = false;
    bool actualDrag = false;
    size_t dragIndex = 0;
    sf::Vector2f dragOffset(0.f, 0.f);
    sf::Vector2f dragStartPos(0.f, 0.f);

    // Deck slot click state
    bool deckClick = false;
    int deckClickIndex = -1;
    sf::Vector2f deckClickPos(0.f,0.f);
    bool victoryClick = false;
    int victoryClickIndex = -1;
    sf::Vector2f victoryClickPos(0.f,0.f);

    // Status message system
    std::string statusMessage = "";
    sf::Color statusColor = sf::Color::Green;
    sf::Clock statusClock;
    const float STATUS_DISPLAY_TIME = 2.0f; // Show status for 2 seconds

    auto sendDeckToServer = [&]() {
        sf::Packet pkt;
        pkt << MessageType::SaveDeck << deck.serialize();
        if (socket.send(pkt) == sf::Socket::Done) {
            statusMessage = "Saving deck...";
            statusColor = sf::Color::Yellow;
            statusClock.restart();
            std::cout << "Deck changes auto-saved to server" << std::endl;
        } else {
            statusMessage = "Failed to save deck!";
            statusColor = sf::Color::Red;
            statusClock.restart();
            std::cerr << "Failed to auto-save deck changes" << std::endl;
        }
    };

    auto isVictoryCard = [&](const Card* card) -> bool {
        auto pc = dynamic_cast<const PieceCard*>(card);
        if (!pc) return false;
        const PieceStats* stats = pieceDefinitions.getPieceStats(pc->getPieceType());
        return stats && stats->isVictoryPiece;
    };

    auto collectionIndexAt = [&](const sf::Vector2f& pos) -> int {
        // Check if click is within collection area bounds
        if (pos.y < collectionY || pos.y > collectionY + CARD_H) return -1;
        if (pos.x < collectionStartX || pos.x > collectionStartX + collectionAreaWidth) return -1;
        
        // Calculate which card was clicked based on horizontal position and scroll
        int idx = static_cast<int>((pos.x - collectionStartX + collectionScroll) / (CARD_W + CARD_SPACING));
        return (idx >= 0 && static_cast<size_t>(idx) < collection.size()) ? idx : -1;
    };

    auto deckSlotIndexAt = [&](const sf::Vector2f& pos) -> int {
        float totalWidth = CARD_W * 10 + CARD_SPACING * 9;
        if (pos.x < deckStartX || pos.x > deckStartX + totalWidth) return -1;
        if (pos.y < deckY || pos.y > deckY + ROW_HEIGHT * 2 - CARD_SPACING) return -1;
        int col = static_cast<int>((pos.x - deckStartX) / (CARD_W + CARD_SPACING));
        int row = static_cast<int>((pos.y - deckY) / ROW_HEIGHT);
        int idx = row * 10 + col;
        return (idx >= 0 && idx < static_cast<int>(Deck::DECK_SIZE)) ? idx : -1;
    };

    auto victorySlotIndexAt = [&](const sf::Vector2f& pos) -> int {
        if (pos.x < victoryStartX || pos.x > victoryStartX + victoryWidth) return -1;
        if (pos.y < victoryY || pos.y > victoryY + CARD_H) return -1;
        int col = static_cast<int>((pos.x - victoryStartX) / (CARD_W + CARD_SPACING));
        return (col >= 0 && col < static_cast<int>(Deck::VICTORY_SIZE)) ? col : -1;
    };

    while (window.isOpen()) {
        // Process network messages first
        sf::Packet receivedPacket;
        sf::Socket::Status status = socket.receive(receivedPacket);
        if (status == sf::Socket::Done) {
            MessageType messageType;
            if (receivedPacket >> messageType) {
                switch (messageType) {
                    case MessageType::DeckSaved:
                        std::cout << "Deck save confirmed by server" << std::endl;
                        statusMessage = "Deck saved successfully!";
                        statusColor = sf::Color::Green;
                        statusClock.restart();
                        break;
                    case MessageType::Error:
                        {
                            std::string errorMsg;
                            if (receivedPacket >> errorMsg) {
                                std::cerr << "Server error: " << errorMsg << std::endl;
                                statusMessage = "Error: " + errorMsg;
                                statusColor = sf::Color::Red;
                                statusClock.restart();
                            }
                        }
                        break;
                    default:
                        std::cout << "Received unhandled message in deck editor: " << static_cast<int>(messageType) << std::endl;
                        break;
                }
            }
        }

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            } else if (event.type == sf::Event::Resized) {
                graphicsManager.updateView();
            } else if (event.type == sf::Event::MouseWheelScrolled) {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    // Horizontal scrolling for collection - scroll by card width + spacing
                    collectionScroll += event.mouseWheelScroll.delta * (CARD_W + CARD_SPACING);
                }
            } else if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i sm(event.mouseButton.x, event.mouseButton.y);
                    sf::Vector2f gm = graphicsManager.screenToGame(sm);
                    int cIdx = collectionIndexAt(gm);
                    if (cIdx >= 0) {
                        dragging = true;
                        actualDrag = false;
                        dragIndex = static_cast<size_t>(cIdx);
                        dragStartPos = gm;
                        float colX = collectionStartX + cIdx * (CARD_W + CARD_SPACING) - collectionScroll;
                        float colY = collectionY;
                        dragOffset = sf::Vector2f(gm.x - colX, gm.y - colY);
                    } else {
                        int dIdx = deckSlotIndexAt(gm);
                        if (dIdx >= 0) {
                            deckClick = true;
                            deckClickIndex = dIdx;
                            deckClickPos = gm;
                        } else {
                            int vIdx = victorySlotIndexAt(gm);
                            if (vIdx >= 0) {
                                victoryClick = true;
                                victoryClickIndex = vIdx;
                                victoryClickPos = gm;
                            }
                        }
                    }
                }
            } else if (event.type == sf::Event::MouseMoved) {
                if (dragging) {
                    sf::Vector2i sm = sf::Mouse::getPosition(window);
                    sf::Vector2f gm = graphicsManager.screenToGame(sm);
                    if (!actualDrag) {
                        if (std::hypot(gm.x - dragStartPos.x, gm.y - dragStartPos.y) > 5.f)
                            actualDrag = true;
                    }
                    dragStartPos = gm; // keep last position for drawing
                }
            } else if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i sm(event.mouseButton.x, event.mouseButton.y);
                    sf::Vector2f gm = graphicsManager.screenToGame(sm);
                    if (dragging) {
                        if (actualDrag) {
                            int dIdx = deckSlotIndexAt(gm);
                            int vIdx = victorySlotIndexAt(gm);
                            const Card* c = collection.getCard(dragIndex);
                            if (c) {
                                if (dIdx >= 0 && !isVictoryCard(c) && deck.size() < Deck::DECK_SIZE) {
                                    deck.addCard(c->clone());
                                    if (!deck.isValidForEditing()) {
                                        deck.removeCardAt(deck.size()-1);
                                        statusMessage = "Max copies reached";
                                        statusColor = sf::Color::Red;
                                        statusClock.restart();
                                    } else {
                                        sendDeckToServer();
                                    }
                                } else if (vIdx >= 0 && isVictoryCard(c)) {
                                    if (deck.setVictoryCardAt(vIdx, c->clone())) {
                                        if (!deck.isValidForEditing()) {
                                            deck.removeVictoryCardAt(vIdx);
                                            statusMessage = "Invalid victory card";
                                            statusColor = sf::Color::Red;
                                            statusClock.restart();
                                        } else {
                                            sendDeckToServer();
                                        }
                                    } else {
                                        // If setVictoryCardAt fails, try adding to next available slot
                                        if (deck.addVictoryCard(c->clone())) {
                                            if (!deck.isValidForEditing()) {
                                                deck.removeVictoryCardAt(deck.victoryCount()-1);
                                                statusMessage = "Invalid victory card";
                                                statusColor = sf::Color::Red;
                                                statusClock.restart();
                                            } else {
                                                sendDeckToServer();
                                            }
                                        } else {
                                            statusMessage = "Victory slots full";
                                            statusColor = sf::Color::Red;
                                            statusClock.restart();
                                        }
                                    }
                                }
                            }
                        } else { // treat as click
                            const Card* c = collection.getCard(dragIndex);
                            if (c) {
                                if (!isVictoryCard(c) && deck.size() < Deck::DECK_SIZE) {
                                    deck.addCard(c->clone());
                                    if (!deck.isValidForEditing()) {
                                        deck.removeCardAt(deck.size()-1);
                                        statusMessage = "Max copies reached";
                                        statusColor = sf::Color::Red;
                                        statusClock.restart();
                                    } else {
                                        sendDeckToServer();
                                    }
                                } else if (isVictoryCard(c)) {
                                    // For clicks, add to the first available slot
                                    size_t targetSlot = deck.victoryCount();
                                    deck.insertVictoryCardAt(targetSlot, c->clone());
                                    if (!deck.isValidForEditing()) {
                                        deck.removeVictoryCardAt(targetSlot);
                                        statusMessage = "Invalid victory card";
                                        statusColor = sf::Color::Red;
                                        statusClock.restart();
                                    } else {
                                        sendDeckToServer();
                                    }
                                }
                            }
                        }
                        dragging = false;
                        actualDrag = false;
                    } else if (deckClick) {
                        if (std::hypot(gm.x - deckClickPos.x, gm.y - deckClickPos.y) < 5.f) {
                            if (deckClickIndex >= 0 && deckClickIndex < static_cast<int>(deck.size())) {
                                deck.removeCardAt(deckClickIndex);
                                sendDeckToServer();
                            }
                        }
                        deckClick = false;
                        deckClickIndex = -1;
                    } else if (victoryClick) {
                        if (std::hypot(gm.x - victoryClickPos.x, gm.y - victoryClickPos.y) < 5.f) {
                            if (victoryClickIndex >= 0 && victoryClickIndex < static_cast<int>(deck.victoryCount())) {
                                deck.removeVictoryCardAt(victoryClickIndex);
                                sendDeckToServer();
                            }
                        }
                        victoryClick = false;
                        victoryClickIndex = -1;
                    }
                }
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    return;
                }
            }
        }

        // Calculate horizontal scroll limits for collection
        float totalCollectionWidth = collection.size() * (CARD_W + CARD_SPACING) - CARD_SPACING;
        float maxScroll = std::max(0.f, totalCollectionWidth - collectionAreaWidth);
        if (collectionScroll < 0.f) collectionScroll = 0.f;
        if (collectionScroll > maxScroll) collectionScroll = maxScroll;

        graphicsManager.applyView();
        window.clear(sf::Color(10, 50, 20));

        // --- Render collection background and scroll area ---
        // Adjust background width for equal padding on both sides
        sf::RectangleShape collectionBg(sf::Vector2f(collectionAreaWidth + 10.f, CARD_H + 10.f));
        collectionBg.setPosition(collectionStartX - 5.f, collectionY - 5.f);
        collectionBg.setFillColor(sf::Color(20, 40, 20, 100));
        collectionBg.setOutlineColor(sf::Color::White);
        collectionBg.setOutlineThickness(1.f);
        window.draw(collectionBg);

        // Collection label
        sf::Text collectionLabel;
        collectionLabel.setFont(font);
        collectionLabel.setCharacterSize(16);
        collectionLabel.setFillColor(sf::Color::White);
        collectionLabel.setString("Collection (Scroll with mouse wheel)");
        collectionLabel.setPosition(collectionStartX, collectionY - 25.f);
        window.draw(collectionLabel);

        // Horizontal scroll indicator
        if (totalCollectionWidth > collectionAreaWidth) {
            float scrollBarWidth = collectionAreaWidth * 0.8f;
            float scrollBarHeight = 4.f;
            float scrollBarX = collectionStartX + (collectionAreaWidth - scrollBarWidth) / 2.f;
            float scrollBarY = collectionY + CARD_H + 8.f;
            
            // Scroll bar background
            sf::RectangleShape scrollBg(sf::Vector2f(scrollBarWidth, scrollBarHeight));
            scrollBg.setPosition(scrollBarX, scrollBarY);
            scrollBg.setFillColor(sf::Color(100, 100, 100, 150));
            window.draw(scrollBg);
            
            // Scroll bar thumb
            float thumbWidth = (collectionAreaWidth / totalCollectionWidth) * scrollBarWidth;
            float thumbX = scrollBarX + (collectionScroll / totalCollectionWidth) * scrollBarWidth;
            sf::RectangleShape scrollThumb(sf::Vector2f(thumbWidth, scrollBarHeight));
            scrollThumb.setPosition(thumbX, scrollBarY);
            scrollThumb.setFillColor(sf::Color::White);
            window.draw(scrollThumb);
        }

        // --- Render collection (horizontal single row) ---
        for (size_t i = 0; i < collection.size(); ++i) {
            float x = collectionStartX + i * (CARD_W + CARD_SPACING) - collectionScroll;
            float y = collectionY;
            
            // Only render cards that are visible in the collection area
            if (x + CARD_W < collectionStartX || x > collectionStartX + collectionAreaWidth) continue;

            sf::RectangleShape rect(sf::Vector2f(CARD_W, CARD_H));
            rect.setPosition(x, y);
            rect.setFillColor(sf::Color(60,80,60,200));
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(1.f);
            window.draw(rect);

            const Card* c = collection.getCard(i);
            if (c) {
                // Helper function to wrap text within card bounds
                auto drawWrappedText = [&](const std::string& text, float cardX, float cardY, sf::Color color) {
                    const float TEXT_MARGIN = 5.f;
                    const float MAX_TEXT_WIDTH = CARD_W - (TEXT_MARGIN * 2);
                    const float MAX_TEXT_HEIGHT = CARD_H - (TEXT_MARGIN * 2);
                    const int FONT_SIZE = 10; // Smaller font size
                    const float LINE_SPACING = FONT_SIZE + 2;
                    
                    sf::Text testText;
                    testText.setFont(font);
                    testText.setCharacterSize(FONT_SIZE);
                    
                    std::vector<std::string> lines;
                    std::string currentLine = "";
                    std::istringstream words(text);
                    std::string word;
                    
                    // Word wrapping logic
                    while (words >> word) {
                        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                        testText.setString(testLine);
                        
                        if (testText.getLocalBounds().width <= MAX_TEXT_WIDTH) {
                            currentLine = testLine;
                        } else {
                            if (!currentLine.empty()) {
                                lines.push_back(currentLine);
                                currentLine = word;
                            } else {
                                // Single word is too long, truncate it
                                lines.push_back(word.substr(0, std::min(word.length(), size_t(8))) + "...");
                                currentLine = "";
                            }
                        }
                    }
                    if (!currentLine.empty()) {
                        lines.push_back(currentLine);
                    }
                    
                    // Draw the wrapped lines
                    float totalTextHeight = lines.size() * LINE_SPACING;
                    float startY = cardY + (CARD_H - totalTextHeight) / 2.f;
                    
                    for (size_t lineIdx = 0; lineIdx < lines.size() && (lineIdx * LINE_SPACING) < MAX_TEXT_HEIGHT; ++lineIdx) {
                        sf::Text lineText;
                        lineText.setFont(font);
                        lineText.setCharacterSize(FONT_SIZE);
                        lineText.setFillColor(color);
                        lineText.setString(lines[lineIdx]);
                        
                        sf::FloatRect bounds = lineText.getLocalBounds();
                        lineText.setPosition(cardX + (CARD_W - bounds.width) / 2.f, startY + lineIdx * LINE_SPACING);
                        window.draw(lineText);
                    }
                };
                
                drawWrappedText(c->getName(), x, y, sf::Color::White);
            }
        }

        // --- Render deck slots ---
        // Deck label
        sf::Text deckLabel;
        deckLabel.setFont(font);
        deckLabel.setCharacterSize(16);
        deckLabel.setFillColor(sf::Color::White);
        deckLabel.setString("Deck (20 cards)");
        deckLabel.setPosition(deckStartX, deckY - 25.f);
        window.draw(deckLabel);

        for (int i = 0; i < static_cast<int>(Deck::DECK_SIZE); ++i) {
            int row = i / 10;
            int col = i % 10;
            float x = deckStartX + col * (CARD_W + CARD_SPACING);
            float y = deckY + row * ROW_HEIGHT;
            sf::RectangleShape slot(sf::Vector2f(CARD_W, CARD_H));
            slot.setPosition(x, y);
            slot.setFillColor(sf::Color(30,30,30,180));
            slot.setOutlineColor(sf::Color::White);
            slot.setOutlineThickness(1.f);
            window.draw(slot);

            if (i < static_cast<int>(deck.size())) {
                const Card* c = deck.getCard(i);
                if (c) {
                    // Helper function to wrap text within card bounds
                    auto drawWrappedText = [&](const std::string& text, float cardX, float cardY, sf::Color color) {
                        const float TEXT_MARGIN = 5.f;
                        const float MAX_TEXT_WIDTH = CARD_W - (TEXT_MARGIN * 2);
                        const float MAX_TEXT_HEIGHT = CARD_H - (TEXT_MARGIN * 2);
                        const int FONT_SIZE = 10; // Smaller font size
                        const float LINE_SPACING = FONT_SIZE + 2;
                        
                        sf::Text testText;
                        testText.setFont(font);
                        testText.setCharacterSize(FONT_SIZE);
                        
                        std::vector<std::string> lines;
                        std::string currentLine = "";
                        std::istringstream words(text);
                        std::string word;
                        
                        // Word wrapping logic
                        while (words >> word) {
                            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                            testText.setString(testLine);
                            
                            if (testText.getLocalBounds().width <= MAX_TEXT_WIDTH) {
                                currentLine = testLine;
                            } else {
                                if (!currentLine.empty()) {
                                    lines.push_back(currentLine);
                                    currentLine = word;
                                } else {
                                    // Single word is too long, truncate it
                                    lines.push_back(word.substr(0, std::min(word.length(), size_t(8))) + "...");
                                    currentLine = "";
                                }
                            }
                        }
                        if (!currentLine.empty()) {
                            lines.push_back(currentLine);
                        }
                        
                        // Draw the wrapped lines
                        float totalTextHeight = lines.size() * LINE_SPACING;
                        float startY = cardY + (CARD_H - totalTextHeight) / 2.f;
                        
                        for (size_t lineIdx = 0; lineIdx < lines.size() && (lineIdx * LINE_SPACING) < MAX_TEXT_HEIGHT; ++lineIdx) {
                            sf::Text lineText;
                            lineText.setFont(font);
                            lineText.setCharacterSize(FONT_SIZE);
                            lineText.setFillColor(color);
                            lineText.setString(lines[lineIdx]);
                            
                            sf::FloatRect bounds = lineText.getLocalBounds();
                            lineText.setPosition(cardX + (CARD_W - bounds.width) / 2.f, startY + lineIdx * LINE_SPACING);
                            window.draw(lineText);
                        }
                    };
                    
                    drawWrappedText(c->getName(), x, y, sf::Color::Yellow);
                }
            }
        }

        // --- Render victory slots ---
        // Victory label
        sf::Text victoryLabel;
        victoryLabel.setFont(font);
        victoryLabel.setCharacterSize(16);
        victoryLabel.setFillColor(sf::Color::White);
        victoryLabel.setString("Victory Pieces (4 cards)");
        victoryLabel.setPosition(victoryStartX, victoryY - 25.f);
        window.draw(victoryLabel);

        for (int i = 0; i < static_cast<int>(Deck::VICTORY_SIZE); ++i) {
            float x = victoryStartX + i * (CARD_W + CARD_SPACING);
            float y = victoryY;
            sf::RectangleShape slot(sf::Vector2f(CARD_W, CARD_H));
            slot.setPosition(x, y);
            slot.setFillColor(sf::Color(60,30,30,180));
            slot.setOutlineColor(sf::Color::White);
            slot.setOutlineThickness(1.f);
            window.draw(slot);

            const Card* c = deck.getVictoryCard(i);
            if (c) {
                if (c) {
                    auto drawWrappedText = [&](const std::string& text, float cardX, float cardY, sf::Color color) {
                        const float TEXT_MARGIN = 5.f;
                        const float MAX_TEXT_WIDTH = CARD_W - (TEXT_MARGIN * 2);
                        const float MAX_TEXT_HEIGHT = CARD_H - (TEXT_MARGIN * 2);
                        const int FONT_SIZE = 10;
                        const float LINE_SPACING = FONT_SIZE + 2;
                        sf::Text testText;
                        testText.setFont(font);
                        testText.setCharacterSize(FONT_SIZE);
                        std::vector<std::string> lines;
                        std::string currentLine = "";
                        std::istringstream words(text);
                        std::string word;
                        while (words >> word) {
                            std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                            testText.setString(testLine);
                            if (testText.getLocalBounds().width <= MAX_TEXT_WIDTH) {
                                currentLine = testLine;
                            } else {
                                if (!currentLine.empty()) {
                                    lines.push_back(currentLine);
                                    currentLine = word;
                                } else {
                                    lines.push_back(word.substr(0, std::min(word.length(), size_t(8))) + "...");
                                    currentLine = "";
                                }
                            }
                        }
                        if (!currentLine.empty()) {
                            lines.push_back(currentLine);
                        }
                        float totalTextHeight = lines.size() * LINE_SPACING;
                        float startY = cardY + (CARD_H - totalTextHeight) / 2.f;
                        for (size_t lineIdx = 0; lineIdx < lines.size() && (lineIdx * LINE_SPACING) < MAX_TEXT_HEIGHT; ++lineIdx) {
                            sf::Text lineText;
                            lineText.setFont(font);
                            lineText.setCharacterSize(FONT_SIZE);
                            lineText.setFillColor(color);
                            lineText.setString(lines[lineIdx]);
                            sf::FloatRect bounds = lineText.getLocalBounds();
                            lineText.setPosition(cardX + (CARD_W - bounds.width) / 2.f, startY + lineIdx * LINE_SPACING);
                            window.draw(lineText);
                        }
                    };

                    drawWrappedText(c->getName(), x, y, sf::Color::Cyan);
                }
            }
        }

        // Draw dragged card if any
        if (dragging && actualDrag) {
            const Card* c = collection.getCard(dragIndex);
            if (c) {
                sf::Vector2i sm = sf::Mouse::getPosition(window);
                sf::Vector2f gm = graphicsManager.screenToGame(sm);
                float x = gm.x - dragOffset.x;
                float y = gm.y - dragOffset.y;
                sf::RectangleShape rect(sf::Vector2f(CARD_W, CARD_H));
                rect.setPosition(x, y);
                rect.setFillColor(sf::Color(60,80,60,200));
                rect.setOutlineColor(sf::Color::Yellow);
                rect.setOutlineThickness(2.f);
                window.draw(rect);

                // Helper function to wrap text within card bounds
                auto drawWrappedText = [&](const std::string& text, float cardX, float cardY, sf::Color color) {
                    const float TEXT_MARGIN = 5.f;
                    const float MAX_TEXT_WIDTH = CARD_W - (TEXT_MARGIN * 2);
                    const float MAX_TEXT_HEIGHT = CARD_H - (TEXT_MARGIN * 2);
                    const int FONT_SIZE = 10; // Smaller font size
                    const float LINE_SPACING = FONT_SIZE + 2;
                    
                    sf::Text testText;
                    testText.setFont(font);
                    testText.setCharacterSize(FONT_SIZE);
                    
                    std::vector<std::string> lines;
                    std::string currentLine = "";
                    std::istringstream words(text);
                    std::string word;
                    
                    // Word wrapping logic
                    while (words >> word) {
                        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                        testText.setString(testLine);
                        
                        if (testText.getLocalBounds().width <= MAX_TEXT_WIDTH) {
                            currentLine = testLine;
                        } else {
                            if (!currentLine.empty()) {
                                lines.push_back(currentLine);
                                currentLine = word;
                            } else {
                                // Single word is too long, truncate it
                                lines.push_back(word.substr(0, std::min(word.length(), size_t(8))) + "...");
                                currentLine = "";
                            }
                        }
                    }
                    if (!currentLine.empty()) {
                        lines.push_back(currentLine);
                    }
                    
                    // Draw the wrapped lines
                    float totalTextHeight = lines.size() * LINE_SPACING;
                    float startY = cardY + (CARD_H - totalTextHeight) / 2.f;
                    
                    for (size_t lineIdx = 0; lineIdx < lines.size() && (lineIdx * LINE_SPACING) < MAX_TEXT_HEIGHT; ++lineIdx) {
                        sf::Text lineText;
                        lineText.setFont(font);
                        lineText.setCharacterSize(FONT_SIZE);
                        lineText.setFillColor(color);
                        lineText.setString(lines[lineIdx]);
                        
                        sf::FloatRect bounds = lineText.getLocalBounds();
                        lineText.setPosition(cardX + (CARD_W - bounds.width) / 2.f, startY + lineIdx * LINE_SPACING);
                        window.draw(lineText);
                    }
                };
                
                drawWrappedText(c->getName(), x, y, sf::Color::White);
            }
        }

        // Draw status message if active
        if (!statusMessage.empty() && statusClock.getElapsedTime().asSeconds() < STATUS_DISPLAY_TIME) {
            sf::Text statusText;
            statusText.setFont(font);
            statusText.setString(statusMessage);
            statusText.setCharacterSize(24);
            statusText.setFillColor(statusColor);
            
            // Position at top center of screen
            sf::FloatRect textBounds = statusText.getLocalBounds();
            statusText.setOrigin(textBounds.left + textBounds.width / 2.f, textBounds.top + textBounds.height / 2.f);
            statusText.setPosition(GraphicsManager::BASE_WIDTH / 2.f, 20.f);
            
            // Draw semi-transparent background
            sf::RectangleShape statusBg(sf::Vector2f(textBounds.width + 20.f, textBounds.height + 10.f));
            statusBg.setFillColor(sf::Color(0, 0, 0, 150));
            statusBg.setOrigin((textBounds.width + 20.f) / 2.f, (textBounds.height + 10.f) / 2.f);
            statusBg.setPosition(GraphicsManager::BASE_WIDTH / 2.f, 20.f);
            
            window.draw(statusBg);
            window.draw(statusText);
        } else if (statusClock.getElapsedTime().asSeconds() >= STATUS_DISPLAY_TIME) {
            // Clear status message after display time
            statusMessage = "";
        }

        window.display();
    }
}
} // namespace BayouBonanza
