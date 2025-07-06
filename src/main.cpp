#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Network.hpp> // Added for networking
#include <iostream>
#include <string>
#include <memory>
#include <cctype> // Required for std::tolower
#include <cmath> // Added for std::sqrt

#include "GameBoard.h"
#include "GameState.h"
#include "Square.h"       // For Square::setGlobalPieceFactory
#include "Move.h"      // For Move and its sf::Packet operators
#include "NetworkProtocol.h" // For MessageType enum and operators
#include "PlayerSide.h"  // For PlayerSide enum
#include "InputManager.h" // New input manager
#include "GraphicsManager.h" // New graphics manager
#include "GameInitializer.h"
#include "PieceFactory.h"
#include "CardCollection.h"
#include "CardFactory.h"
#include "PieceDefinitionManager.h"
#include "TurnManager.h"
#include "InfluenceSystem.h" // Added for InfluenceSystem
#include "GameOverDetector.h"
#include "CardPlayValidator.h"
#include "PieceData.h" // For Position
#include "PieceCard.h"
#include "EffectCard.h"
#include <map>
// #include "King.h" // Removed - using data-driven approach with PieceFactory


// The actual sf::Packet operators are now defined with their respective classes.

using namespace BayouBonanza;

enum class MainMenuOption {
    DECK_EDITOR,
    PLAY_HUMAN,
    PLAY_AI,
    NONE
};

// Client-specific global variables
PlayerSide myPlayerSide = PlayerSide::NEUTRAL; // Default, will be assigned by server
bool gameHasStarted = false;
std::string uiMessage = "Connecting..."; // For displaying messages like "Waiting for opponent"
sf::Text uiMessageText;
sf::Font globalFont; // Loaded once

// Global PieceFactory for deserialization
PieceDefinitionManager globalPieceDefManager;
std::unique_ptr<PieceFactory> globalPieceFactory;

// UI Elements for Usernames/Ratings
sf::Text localPlayerUsernameText;
sf::Text localPlayerRatingText;
sf::Text remotePlayerUsernameText;
sf::Text remotePlayerRatingText;

// UI Element for Local Player Steam
sf::Text localPlayerSteamText;

// UI Element for Phase Information
sf::Text phaseText;

// Global variables for game state
GameState gameState;
GameInitializer gameInitializer;
GameOverDetector gameOverDetector;
std::string winMessage = "";
bool showWinMessage = false;

std::map<std::string, sf::Texture> pieceTextures;

// Player collection and deck
CardCollection myCollection;
Deck myDeck;

// Global variables to store GameStart data received in main menu
bool gameStartReceived = false;
std::string gameStartP1Username, gameStartP2Username;
int gameStartP1Rating = 0, gameStartP2Rating = 0;
sf::Packet gameStartPacketData;

// Win condition notification callback
void onWinCondition(PlayerSide winner, const std::string& description) {
    winMessage = description;
    showWinMessage = true;
    std::cout << "WIN CONDITION: " << description << std::endl;
}

// Function to recreate pieces after deserialization without resetting game state
void recreatePiecesAfterDeserialization(GameState& gameState) {
    // The issue is that Square deserialization loses pieces due to PieceFactory access
    // Instead of resetting the entire game state, we'll recreate pieces using a standard layout
    // This is a temporary workaround until we fix the Square deserialization properly
    
    GameBoard& board = gameState.getBoard();
    
    // Clear the board first
    board.resetBoard();
    
    // Recreate the standard starting position using PieceFactory
    if (globalPieceFactory) {
        // Player 1 pieces (bottom of board)
        // Back row
        board.getSquare(0, 7).setPiece(globalPieceFactory->createPiece("Sweetykins", PlayerSide::PLAYER_ONE));
        board.getSquare(1, 7).setPiece(globalPieceFactory->createPiece("Automatick", PlayerSide::PLAYER_ONE));
        board.getSquare(2, 7).setPiece(globalPieceFactory->createPiece("Sidewinder", PlayerSide::PLAYER_ONE));
        board.getSquare(3, 7).setPiece(globalPieceFactory->createPiece("ScarlettGlumpkin", PlayerSide::PLAYER_ONE));
        board.getSquare(4, 7).setPiece(globalPieceFactory->createPiece("TinkeringTom", PlayerSide::PLAYER_ONE));
        board.getSquare(5, 7).setPiece(globalPieceFactory->createPiece("Rustbucket", PlayerSide::PLAYER_ONE));
        board.getSquare(6, 7).setPiece(globalPieceFactory->createPiece("Automatick", PlayerSide::PLAYER_ONE));
        board.getSquare(7, 7).setPiece(globalPieceFactory->createPiece("Sweetykins", PlayerSide::PLAYER_ONE));
        
        // Pawn row
        for (int x = 0; x < 8; x++) {
            board.getSquare(x, 6).setPiece(globalPieceFactory->createPiece("Sentroid", PlayerSide::PLAYER_ONE));
        }
        
        // Player 2 pieces (top of board)
        // Back row
        board.getSquare(0, 0).setPiece(globalPieceFactory->createPiece("Sweetykins", PlayerSide::PLAYER_TWO));
        board.getSquare(1, 0).setPiece(globalPieceFactory->createPiece("Automatick", PlayerSide::PLAYER_TWO));
        board.getSquare(2, 0).setPiece(globalPieceFactory->createPiece("Rustbucket", PlayerSide::PLAYER_TWO));
        board.getSquare(3, 0).setPiece(globalPieceFactory->createPiece("ScarlettGlumpkin", PlayerSide::PLAYER_TWO));
        board.getSquare(4, 0).setPiece(globalPieceFactory->createPiece("TinkeringTom", PlayerSide::PLAYER_TWO));
        board.getSquare(5, 0).setPiece(globalPieceFactory->createPiece("Sidewinder", PlayerSide::PLAYER_TWO));
        board.getSquare(6, 0).setPiece(globalPieceFactory->createPiece("Automatick", PlayerSide::PLAYER_TWO));
        board.getSquare(7, 0).setPiece(globalPieceFactory->createPiece("Sweetykins", PlayerSide::PLAYER_TWO));
        
        // Pawn row
        for (int x = 0; x < 8; x++) {
            board.getSquare(x, 1).setPiece(globalPieceFactory->createPiece("Sentroid", PlayerSide::PLAYER_TWO));
        }
        
        // Set piece positions
        for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
                const Square& square = board.getSquare(x, y);
                if (!square.isEmpty()) {
                    Piece* piece = square.getPiece();
                    piece->setPosition(Position(x, y));
                }
            }
        }
    }
}

// Function to print board state to console for debugging
void printBoardState(const GameState& gameState, PlayerSide localPlayer = PlayerSide::NEUTRAL) {
    const GameBoard& board = gameState.getBoard();
    std::cout << "Current board state:" << std::endl;
    for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
            const Square& square = board.getSquare(x, y);
            if (square.isEmpty()) {
                std::cout << ". ";
            } else {
                Piece* piece = square.getPiece();
                std::cout << piece->getSymbol() << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
    
    // Print local player's hand if specified
    if (localPlayer != PlayerSide::NEUTRAL) {
        const Hand& hand = gameState.getHand(localPlayer);
        std::cout << "=== MY HAND DEBUG ===" << std::endl;
        std::cout << "My Hand (" << hand.size() << " cards, " 
                  << gameState.getSteam(localPlayer) << " steam):" << std::endl;
        for (size_t i = 0; i < hand.size(); ++i) {
            const Card* card = hand.getCard(i);
            if (card) {
                std::cout << "  [" << i << "] " << card->getName() 
                          << " (Cost: " << card->getSteamCost() << ")" << std::endl;
            }
        }
        std::cout << "=====================" << std::endl;
    }
}

// Function to render discrete health bar as a grid
void renderHealthBar(sf::RenderWindow& window, const Piece* piece, float squareX, float squareY, float squareSize) {
    if (!piece) return;
    
    int currentHealth = piece->getHealth();
    int maxHealth = piece->getMaxHealth(); // Get original max health from stats
    
    if (maxHealth <= 0) return; // Avoid division by zero
    
    // Calculate health bar dimensions
    float healthBarWidth = squareSize * 0.3f; // 30% of square width
    float healthBarHeight = squareSize * 0.15f; // 15% of square height
    
    // Position in bottom left corner of the piece square
    float healthBarX = squareX + squareSize * 0.05f; // Small margin from left edge
    float healthBarY = squareY + squareSize - healthBarHeight - squareSize * 0.05f; // Small margin from bottom
    
    // Calculate optimal grid dimensions to avoid gaps
    int gridCols, gridRows;
    
    if (maxHealth == 1) {
        gridCols = 1;
        gridRows = 1;
    } else if (maxHealth <= 4) {
        // For 2-4 health, use a single row
        gridCols = maxHealth;
        gridRows = 1;
    } else if (maxHealth <= 6) {
        // For 5-6 health, use 2 rows with balanced distribution
        if (maxHealth == 5) {
            gridCols = 3; // 3 cells top, 2 cells bottom
            gridRows = 2;
        } else { // maxHealth == 6
            gridCols = 3; // 3 cells top, 3 cells bottom
            gridRows = 2;
        }
    } else if (maxHealth <= 9) {
        // For 7-9 health, use 3 rows
        gridCols = 3;
        gridRows = 3;
    } else {
        // For 10+ health, use a more rectangular layout
        // Calculate the most square-like arrangement
        gridRows = static_cast<int>(std::sqrt(maxHealth));
        gridCols = (maxHealth + gridRows - 1) / gridRows; // Ceiling division
        
        // Adjust to prefer wider layouts for better visual balance
        if (gridRows > 3) {
            gridRows = 3;
            gridCols = (maxHealth + gridRows - 1) / gridRows;
        }
    }
    
    // For layouts that would have gaps, redistribute cells to fill rows completely
    if (maxHealth > gridCols && maxHealth % gridCols != 0) {
        // Calculate how many cells would be in the last row
        int cellsInLastRow = maxHealth % gridCols;
        
        // If the last row would be less than half full, redistribute
        if (cellsInLastRow > 0 && cellsInLastRow < (gridCols / 2)) {
            // Try a different column count that divides more evenly
            for (int testCols = gridCols - 1; testCols >= 2; testCols--) {
                int testRows = (maxHealth + testCols - 1) / testCols;
                if (testRows <= 3) { // Don't exceed 3 rows for visual clarity
                    gridCols = testCols;
                    gridRows = testRows;
                    break;
                }
            }
        }
    }
    
    // Calculate individual cell size
    float cellWidth = healthBarWidth / gridCols;
    float cellHeight = healthBarHeight / gridRows;
    
    // Draw background for health bar area
    sf::RectangleShape background(sf::Vector2f(healthBarWidth, healthBarHeight));
    background.setPosition(healthBarX, healthBarY);
    background.setFillColor(sf::Color(0, 0, 0, 100)); // Semi-transparent black background
    background.setOutlineThickness(1.0f);
    background.setOutlineColor(sf::Color(255, 255, 255, 150)); // Light outline
    window.draw(background);
    
    // Draw individual health cells with smart positioning
    for (int i = 0; i < maxHealth; ++i) {
        int row = i / gridCols;
        int col = i % gridCols;
        
        // For the last row, center the cells if there are fewer than gridCols
        int cellsInThisRow = std::min(gridCols, maxHealth - row * gridCols);
        float rowStartOffset = 0.0f;
        
        if (cellsInThisRow < gridCols && row == gridRows - 1) {
            // Center the cells in the last row
            rowStartOffset = (gridCols - cellsInThisRow) * cellWidth / 2.0f;
        }
        
        float cellX = healthBarX + rowStartOffset + col * cellWidth;
        float cellY = healthBarY + row * cellHeight;
        
        sf::RectangleShape cell(sf::Vector2f(cellWidth - 1.0f, cellHeight - 1.0f)); // Small gap between cells
        cell.setPosition(cellX, cellY);
        
        // Color based on current health
        if (i < currentHealth) {
            // Healthy cell - use green with intensity based on health percentage
            float healthRatio = static_cast<float>(currentHealth) / maxHealth;
            if (healthRatio > 0.75f) {
                cell.setFillColor(sf::Color(0, 255, 0, 200)); // Bright green for high health
            } else if (healthRatio > 0.5f) {
                cell.setFillColor(sf::Color(255, 255, 0, 200)); // Yellow for medium health
            } else if (healthRatio > 0.25f) {
                cell.setFillColor(sf::Color(255, 165, 0, 200)); // Orange for low health
            } else {
                cell.setFillColor(sf::Color(255, 0, 0, 200)); // Red for critical health
            }
        } else {
            // Damaged/missing health cell
            cell.setFillColor(sf::Color(100, 100, 100, 150)); // Gray for missing health
        }
        
        cell.setOutlineThickness(0.5f);
        cell.setOutlineColor(sf::Color(255, 255, 255, 100)); // Light outline for each cell
        window.draw(cell);
    }
}

// Function to render the attack value in the bottom right corner of a piece square
void renderAttackValue(sf::RenderWindow& window, const Piece* piece, float squareX, float squareY, float squareSize) {
    if (!piece) return;

    sf::Text attackText;
    attackText.setFont(globalFont);
    attackText.setString(std::to_string(piece->getAttack()));
    attackText.setCharacterSize(static_cast<unsigned int>(squareSize * 0.2f));
    attackText.setFillColor(sf::Color::White);

    sf::FloatRect bounds = attackText.getLocalBounds();
    attackText.setOrigin(bounds.left + bounds.width, bounds.top + bounds.height);

    float offset = squareSize * 0.05f; // Small margin from the square edges
    attackText.setPosition(squareX + squareSize - offset, squareY + squareSize - offset);

    window.draw(attackText);
}

// Function to render player's hand of cards
void renderPlayerHand(sf::RenderWindow& window, const GameState& gameState, PlayerSide player, 
                     const GraphicsManager& graphicsManager, int selectedCardIndex = -1) {
    const Hand& hand = gameState.getHand(player);
    if (hand.size() == 0) return;
    
    auto boardParams = graphicsManager.getBoardRenderParams();
    int playerSteam = gameState.getSteam(player);
    
    // Card dimensions and positioning
    float cardWidth = 120.0f;
    float cardHeight = 120.0f; // Shortened card height
    float cardSpacing = 10.0f;
    float totalHandWidth = hand.size() * cardWidth + (hand.size() - 1) * cardSpacing;
    
    // Position cards below the board, centered
    float handStartX = (GraphicsManager::BASE_WIDTH - totalHandWidth) / 2.0f;
    float handY = boardParams.boardStartY + boardParams.boardSize + 10.0f; // Reduced spacing to fit cards
    
    for (size_t i = 0; i < hand.size(); ++i) {
        const Card* card = hand.getCard(i);
        if (!card) continue;
        
        float cardX = handStartX + i * (cardWidth + cardSpacing);
        
        // Card background
        sf::RectangleShape cardRect(sf::Vector2f(cardWidth, cardHeight));
        cardRect.setPosition(cardX, handY);
        
        // Color based on playability and selection
        bool canAfford = playerSteam >= card->getSteamCost();
        bool isSelected = static_cast<int>(i) == selectedCardIndex;
        
        if (isSelected) {
            cardRect.setFillColor(sf::Color(100, 150, 255, 200)); // Bright blue for selected
            cardRect.setOutlineColor(sf::Color::Yellow);
            cardRect.setOutlineThickness(3.0f);
        } else if (canAfford) {
            cardRect.setFillColor(sf::Color(60, 80, 60, 180)); // Green-ish for playable
            cardRect.setOutlineColor(sf::Color::White);
            cardRect.setOutlineThickness(1.0f);
        } else {
            cardRect.setFillColor(sf::Color(80, 60, 60, 180)); // Red-ish for unaffordable
            cardRect.setOutlineColor(sf::Color(128, 128, 128)); // Gray
            cardRect.setOutlineThickness(1.0f);
        }
        
        window.draw(cardRect);
        
        // Card name
        sf::Text nameText;
        nameText.setFont(globalFont);
        nameText.setCharacterSize(14);
        nameText.setFillColor(sf::Color::White);
        nameText.setString(card->getName());
        
        // Center the name text horizontally
        sf::FloatRect nameBounds = nameText.getLocalBounds();
        nameText.setPosition(cardX + (cardWidth - nameBounds.width) / 2.0f, handY + 10.0f);
        window.draw(nameText);
        
        // Steam cost
        sf::Text costText;
        costText.setFont(globalFont);
        costText.setCharacterSize(16);
        costText.setFillColor(canAfford ? sf::Color::Cyan : sf::Color::Red);
        costText.setString("Steam: " + std::to_string(card->getSteamCost()));
        
        sf::FloatRect costBounds = costText.getLocalBounds();
        costText.setPosition(cardX + (cardWidth - costBounds.width) / 2.0f, handY + cardHeight - 25.0f);
        window.draw(costText);
        
        // Card type indicator
        sf::Text typeText;
        typeText.setFont(globalFont);
        typeText.setCharacterSize(12);
        typeText.setFillColor(sf::Color::Yellow);
        
        std::string typeStr = (card->getCardType() == CardType::PIECE_CARD) ? "Piece" : "Effect";
        typeText.setString(typeStr);
        
        sf::FloatRect typeBounds = typeText.getLocalBounds();
        typeText.setPosition(cardX + (cardWidth - typeBounds.width) / 2.0f, handY + 35.0f);
        window.draw(typeText);
        

    }
}

// Simple username login screen
std::string runLoginScreen(sf::RenderWindow& window, GraphicsManager& graphicsManager) {
    std::string username;

    sf::Text promptText;
    promptText.setFont(globalFont);
    promptText.setCharacterSize(32);
    promptText.setFillColor(sf::Color::White);
    promptText.setString("Enter Username");

    sf::Text inputText;
    inputText.setFont(globalFont);
    inputText.setCharacterSize(28);
    inputText.setFillColor(sf::Color::Cyan);

    sf::RectangleShape inputBox(sf::Vector2f(400.f, 50.f));
    inputBox.setFillColor(sf::Color(30, 30, 30));
    inputBox.setOutlineColor(sf::Color::White);
    inputBox.setOutlineThickness(2.f);

    bool done = false;
    while (window.isOpen() && !done) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return "";
            } else if (event.type == sf::Event::Resized) {
                graphicsManager.updateView();
            } else if (event.type == sf::Event::TextEntered) {
                if (event.text.unicode == 8) { // Backspace
                    if (!username.empty()) username.pop_back();
                } else if (event.text.unicode == 13 || event.text.unicode == 10) { // Enter
                    if (!username.empty()) done = true;
                } else if (event.text.unicode < 128 && std::isprint(event.text.unicode)) {
                    username += static_cast<char>(event.text.unicode);
                }
            }
        }

        graphicsManager.applyView();
        window.clear(sf::Color(10, 50, 20));

        float centerX = GraphicsManager::BASE_WIDTH / 2.f;
        float centerY = GraphicsManager::BASE_HEIGHT / 2.f;

        sf::FloatRect promptBounds = promptText.getLocalBounds();
        promptText.setPosition(centerX - promptBounds.width / 2.f, centerY - 80.f);

        inputBox.setPosition(centerX - inputBox.getSize().x / 2.f, centerY - inputBox.getSize().y / 2.f);

        inputText.setString(username);
        inputText.setPosition(inputBox.getPosition().x + 10.f, inputBox.getPosition().y + 10.f);

        window.draw(promptText);
        window.draw(inputBox);
        window.draw(inputText);
        window.display();
    }

    return username;
}

void showPlaceholderScreen(sf::RenderWindow& window, GraphicsManager& graphicsManager, const std::string& message) {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            } else if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonPressed) {
                return;
            } else if (event.type == sf::Event::Resized) {
                graphicsManager.updateView();
            }
        }

        graphicsManager.applyView();
        window.clear(sf::Color(10, 50, 20));

        sf::Text text;
        text.setFont(globalFont);
        text.setString(message + "\n(Press any key)");
        text.setCharacterSize(32);
        text.setFillColor(sf::Color::White);

        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        text.setPosition(GraphicsManager::BASE_WIDTH / 2.f, GraphicsManager::BASE_HEIGHT / 2.f);

        window.draw(text);
        window.display();
    }
}

void runDeckEditor(sf::RenderWindow& window, GraphicsManager& graphicsManager, sf::TcpSocket& socket) {
    size_t selectedCollection = 0;
    size_t selectedDeck = 0;
    bool editingDeck = false; // false=select from collection, true=deck
    
    // Status message system
    std::string statusMessage = "";
    sf::Color statusColor = sf::Color::Green;
    sf::Clock statusClock;
    const float STATUS_DISPLAY_TIME = 2.0f; // Show status for 2 seconds

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
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    // Deck changes are now auto-saved, so we can exit immediately
                    return;
                } else if (event.key.code == sf::Keyboard::Tab) {
                    editingDeck = !editingDeck;
                } else if (event.key.code == sf::Keyboard::Up) {
                    if (editingDeck && selectedDeck > 0) --selectedDeck;
                    if (!editingDeck && selectedCollection > 0) --selectedCollection;
                } else if (event.key.code == sf::Keyboard::Down) {
                    if (editingDeck && selectedDeck + 1 < myDeck.size()) ++selectedDeck;
                    if (!editingDeck && selectedCollection + 1 < myCollection.size()) ++selectedCollection;
                } else if (event.key.code == sf::Keyboard::Enter) {
                    bool deckChanged = false;
                    if (editingDeck) {
                        if (selectedDeck < myDeck.size()) {
                            myDeck.removeCardAt(selectedDeck);
                            deckChanged = true;
                        }
                    } else {
                        if (myDeck.size() < Deck::DECK_SIZE && selectedCollection < myCollection.size()) {
                            const Card* c = myCollection.getCard(selectedCollection);
                            if (c) {
                                myDeck.addCard(c->clone());
                                deckChanged = true;
                            }
                        }
                    }
                    
                    // Auto-save deck changes to database
                    if (deckChanged) {
                        sf::Packet pkt;
                        pkt << MessageType::SaveDeck << myDeck.serialize();
                        if (socket.send(pkt) == sf::Socket::Done) {
                            std::cout << "Deck changes auto-saved to server" << std::endl;
                            statusMessage = "Saving deck...";
                            statusColor = sf::Color::Yellow;
                            statusClock.restart();
                        } else {
                            std::cerr << "Failed to auto-save deck changes" << std::endl;
                            statusMessage = "Failed to save deck!";
                            statusColor = sf::Color::Red;
                            statusClock.restart();
                        }
                    }
                }
            } else if (event.type == sf::Event::Resized) {
                graphicsManager.updateView();
            }
        }

        graphicsManager.applyView();
        window.clear(sf::Color(10, 50, 20));

        float y = 50.f;
        sf::Text header;
        header.setFont(globalFont);
        header.setCharacterSize(28);
        header.setFillColor(sf::Color::Yellow);
        header.setString("Collection (TAB to switch)");
        header.setPosition(40.f, y);
        window.draw(header);
        y += 30.f;
        for (size_t i=0;i<myCollection.size();++i) {
            sf::Text t; t.setFont(globalFont); t.setCharacterSize(20);
            const Card* c = myCollection.getCard(i);
            t.setString(c?c->getName():"?");
            t.setFillColor(!editingDeck && i==selectedCollection?sf::Color::Cyan:sf::Color::White);
            t.setPosition(40.f, y+i*22.f);
            window.draw(t);
        }

        float x2 = GraphicsManager::BASE_WIDTH/2.f + 40.f;
        y = 50.f;
        header.setString("Deck (Enter to remove)");
        header.setPosition(x2, y);
        window.draw(header);
        y += 30.f;
        for (size_t i=0;i<myDeck.size();++i) {
            sf::Text t; t.setFont(globalFont); t.setCharacterSize(20);
            const Card* c = myDeck.getCard(i);
            t.setString(c?c->getName():"?");
            t.setFillColor(editingDeck && i==selectedDeck?sf::Color::Cyan:sf::Color::White);
            t.setPosition(x2, y+i*22.f);
            window.draw(t);
        }

        // Draw status message if active
        if (!statusMessage.empty() && statusClock.getElapsedTime().asSeconds() < STATUS_DISPLAY_TIME) {
            sf::Text statusText;
            statusText.setFont(globalFont);
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

MainMenuOption runMainMenu(sf::RenderWindow& window, GraphicsManager& graphicsManager, sf::TcpSocket& socket) {
    int selected = 0;
    const char* optionTexts[] = {"Deck Editor", "Play Against Human", "Play Against AI"};
    const int optionCount = 3;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return MainMenuOption::NONE;
            } else if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    selected = (selected + optionCount - 1) % optionCount;
                } else if (event.key.code == sf::Keyboard::Down) {
                    selected = (selected + 1) % optionCount;
                } else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Return) {
                    switch (selected) {
                        case 0: return MainMenuOption::DECK_EDITOR;
                        case 1: 
                            {
                                // Send matchmaking request to server
                                sf::Packet matchmakingPacket;
                                matchmakingPacket << MessageType::RequestMatchmaking;
                                if (socket.send(matchmakingPacket) == sf::Socket::Done) {
                                    std::cout << "Matchmaking request sent to server" << std::endl;
                                } else {
                                    std::cerr << "Failed to send matchmaking request" << std::endl;
                                }
                                // Don't return immediately - wait for server response
                            }
                            break;
                        case 2: return MainMenuOption::PLAY_AI;
                    }
                }
            } else if (event.type == sf::Event::Resized) {
                graphicsManager.updateView();
            }
        }

        // Process network messages while in main menu
        sf::Packet receivedPacket;
        sf::Socket::Status status = socket.receive(receivedPacket);
        if (status == sf::Socket::Done) {
            std::cout << "Received packet from server in main menu" << std::endl;
            MessageType messageType;
            if (receivedPacket >> messageType) {
                std::cout << "Message type: " << static_cast<int>(messageType) << std::endl;
                switch (messageType) {
                    case MessageType::PlayerAssignment:
                        {
                            uint8_t side_uint8;
                            if (receivedPacket >> side_uint8) {
                                myPlayerSide = static_cast<PlayerSide>(side_uint8);
                                std::cout << "Assigned player side: Player " << (myPlayerSide == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
                            }
                        }
                        break;
                    case MessageType::CardCollectionData:
                        {
                            std::string data;
                            if (receivedPacket >> data) {
                                myCollection.deserialize(data);
                                std::cout << "Collection received with " << myCollection.size() << " cards" << std::endl;
                            }
                        }
                        break;
                    case MessageType::DeckData:
                        {
                            std::string data;
                            if (receivedPacket >> data) {
                                myDeck.deserialize(data);
                                std::cout << "Deck received with " << myDeck.size() << " cards" << std::endl;
                            }
                        }
                        break;
                    case MessageType::WaitingForOpponent:
                        std::cout << "Waiting for opponent..." << std::endl;
                        break;
                    case MessageType::GameStart:
                        {
                            std::cout << "Game start received in main menu - storing packet data and transitioning to game!" << std::endl;
                            // Store the entire packet data for processing in the main game loop
                            gameStartPacketData = receivedPacket;
                            gameStartReceived = true;
                            std::cout << "GameStart packet data stored for processing in main game loop" << std::endl;
                            return MainMenuOption::PLAY_HUMAN;
                        }
                        break;
                    default:
                        std::cout << "Received unhandled message type in main menu: " << static_cast<int>(messageType) << std::endl;
                        break;
                }
            }
        }

        graphicsManager.applyView();
        window.clear(sf::Color(10, 50, 20));

        sf::Text title;
        title.setFont(globalFont);
        title.setString("Main Menu");
        title.setCharacterSize(40);
        title.setFillColor(sf::Color::White);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.left + titleBounds.width / 2.f, titleBounds.top + titleBounds.height / 2.f);
        title.setPosition(GraphicsManager::BASE_WIDTH / 2.f, GraphicsManager::BASE_HEIGHT / 2.f - 120.f);
        window.draw(title);

        for (int i = 0; i < optionCount; ++i) {
            sf::Text option;
            option.setFont(globalFont);
            option.setString(optionTexts[i]);
            option.setCharacterSize(28);
            option.setFillColor(i == selected ? sf::Color::Yellow : sf::Color::White);

            sf::FloatRect b = option.getLocalBounds();
            option.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
            option.setPosition(GraphicsManager::BASE_WIDTH / 2.f, GraphicsManager::BASE_HEIGHT / 2.f - 20.f + i * 50.f);
            window.draw(option);
        }

        window.display();
    }

    return MainMenuOption::NONE;
}

int main()
{
    // Create the main window with a default size - GraphicsManager will handle scaling
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Bayou Bonanza");
    window.setFramerateLimit(60);

    // Initialize graphics manager for resolution scaling
    GraphicsManager graphicsManager(window);

    // Load font early for login screen and UI
    if (!globalFont.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        std::cerr << "Error loading font from assets/fonts/Roboto-Regular.ttf\n";
        return -1;
    }

    // Initialize global PieceFactory for deserialization
    if (!globalPieceDefManager.loadDefinitions("assets/data/pieces.json")) {
        std::cerr << "FATAL: Could not load piece definitions from assets/data/pieces.json" << std::endl;
        return -1;
    }
    globalPieceFactory = std::make_unique<PieceFactory>(globalPieceDefManager);

    // Initialize CardFactory for card operations
    CardFactory::initialize();

    // Load piece textures
    for (const auto& typeName : globalPieceDefManager.getAllPieceTypeNames()) {
        const PieceStats* stats = globalPieceDefManager.getPieceStats(typeName);
        if (!stats) continue;
        if (!stats->spritePath.empty()) {
            sf::Texture tex;
            if (tex.loadFromFile(std::string("assets/") + stats->spritePath)) {
                pieceTextures[typeName] = tex;
            }
        }
    }

    // Set the global PieceFactory for Square deserialization
    Square::setGlobalPieceFactory(globalPieceFactory.get());

    // Display login screen for username input
    std::string username = runLoginScreen(window, graphicsManager);
    if (username.empty()) {
        return 0; // Window closed before entering username
    }

    // Network Socket
    sf::TcpSocket socket;
    const unsigned short PORT = 50000;
    const std::string SERVER_IP = "127.0.0.1"; // localhost

    std::cout << "Attempting to connect to server " << SERVER_IP << ":" << PORT << std::endl;
    if (socket.connect(SERVER_IP, PORT, sf::seconds(5)) != sf::Socket::Done) {
        std::cerr << "Error: Could not connect to the server." << std::endl;
        uiMessage = "Failed to connect to server.";
        // No return -1 yet, let the window open to display the message
    } else {
        std::cout << "Connected to server!" << std::endl;
        
        // Send UserLogin message
        sf::Packet loginPacket;
        loginPacket << MessageType::UserLogin << username;
        if (socket.send(loginPacket) != sf::Socket::Done) {
            std::cerr << "Error: Failed to send login packet." << std::endl;
            uiMessage = "Failed to send login info.";
            // Consider closing socket or handling error more robustly
        } else {
            std::cout << "Login packet sent with username: " << username << std::endl;
            uiMessage = "Login sent! Waiting for assignment...";
        }
    }
    socket.setBlocking(false); // Use non-blocking mode for game loop

    // Show main menu
    MainMenuOption menuChoice = MainMenuOption::NONE;
    do {
        menuChoice = runMainMenu(window, graphicsManager, socket);
        if (menuChoice == MainMenuOption::DECK_EDITOR) {
            runDeckEditor(window, graphicsManager, socket);
        } else if (menuChoice == MainMenuOption::PLAY_AI) {
            showPlaceholderScreen(window, graphicsManager, "Play vs AI Coming Soon");
        }
    } while (window.isOpen() && menuChoice != MainMenuOption::PLAY_HUMAN);
    if (!window.isOpen()) {
        return 0;
    }

    // Initialize UI text elements
    uiMessageText.setFont(globalFont);
    uiMessageText.setCharacterSize(24);
    uiMessageText.setFillColor(sf::Color::White);
    uiMessageText.setPosition(10.f, 10.f);
    uiMessageText.setString(uiMessage);

    localPlayerUsernameText.setFont(globalFont);
    localPlayerUsernameText.setCharacterSize(18);
    localPlayerUsernameText.setFillColor(sf::Color::Cyan);

    localPlayerRatingText.setFont(globalFont);
    localPlayerRatingText.setCharacterSize(16);
    localPlayerRatingText.setFillColor(sf::Color::White);

    remotePlayerUsernameText.setFont(globalFont);
    remotePlayerUsernameText.setCharacterSize(18);
    remotePlayerUsernameText.setFillColor(sf::Color::Yellow);

    remotePlayerRatingText.setFont(globalFont);
    remotePlayerRatingText.setCharacterSize(16);
    remotePlayerRatingText.setFillColor(sf::Color::White);

    localPlayerSteamText.setFont(globalFont);
    localPlayerSteamText.setCharacterSize(16);
    localPlayerSteamText.setFillColor(sf::Color::White);

    phaseText.setFont(globalFont);
    phaseText.setCharacterSize(20);
    phaseText.setFillColor(sf::Color::Yellow);
    phaseText.setPosition(10.f, 35.f); // Position below the main UI message

    // Initialize game state
    GameState gameState;
    
    // Register win condition callback
    GameOverDetector::registerWinConditionCallback(onWinCondition);

    // Initialize input manager with graphics manager
    InputManager inputManager(window, socket, gameState, gameHasStarted, myPlayerSide, graphicsManager);

    sf::Clock frameClock;

    // Main game loop
    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();
        
        // Process stored GameStart data if received in main menu
        if (gameStartReceived && !gameHasStarted) {
            std::cout << "Processing stored GameStart packet data from main menu" << std::endl;
            
            // Deserialize the stored packet data
            std::string p1_username, p2_username;
            int p1_rating, p2_rating;
            
            if (gameStartPacketData >> p1_username >> p1_rating >> p2_username >> p2_rating >> gameState) {
                gameHasStarted = true;
                printBoardState(gameState, myPlayerSide); // Keep this for debugging

                // Determine local and remote player data
                if (myPlayerSide == PlayerSide::PLAYER_ONE) {
                    localPlayerUsernameText.setString("You: " + p1_username);
                    localPlayerRatingText.setString("Rating: " + std::to_string(p1_rating));
                    remotePlayerUsernameText.setString("Opponent: " + p2_username);
                    remotePlayerRatingText.setString("Rating: " + std::to_string(p2_rating));
                } else if (myPlayerSide == PlayerSide::PLAYER_TWO) {
                    localPlayerUsernameText.setString("You: " + p2_username);
                    localPlayerRatingText.setString("Rating: " + std::to_string(p2_rating));
                    remotePlayerUsernameText.setString("Opponent: " + p1_username);
                    remotePlayerRatingText.setString("Rating: " + std::to_string(p1_rating));
                }
                uiMessage = "Game started!"; 
                std::cout << uiMessage << " P1: " << p1_username << " (" << p1_rating << "), P2: " << p2_username << " (" << p2_rating << ")" << std::endl;

                // Set positions for username/rating texts (using base resolution coordinates)
                // Account for phase text by adding extra spacing
                localPlayerUsernameText.setPosition(10.f, uiMessageText.getPosition().y + 55.f);
                localPlayerRatingText.setPosition(10.f, localPlayerUsernameText.getPosition().y + 20.f);
                localPlayerSteamText.setPosition(10.f, localPlayerRatingText.getPosition().y + 20.f);
                
                // Update local player steam display
                int localPlayerSteam = gameState.getSteam(myPlayerSide);
                localPlayerSteamText.setString("Steam: " + std::to_string(localPlayerSteam));
                
                // Position remote player text on the right side
                float remoteTextWidthEstimate = 200.f; 
                remotePlayerUsernameText.setPosition(GraphicsManager::BASE_WIDTH - remoteTextWidthEstimate - 10.f, uiMessageText.getPosition().y + 55.f);
                remotePlayerRatingText.setPosition(GraphicsManager::BASE_WIDTH - remoteTextWidthEstimate - 10.f, remotePlayerUsernameText.getPosition().y + 20.f);
                
                std::cout << "Successfully processed stored GameStart data" << std::endl;
            } else {
                std::cerr << "Error deserializing stored GameStart packet data" << std::endl;
            }
            
            // Clear the flag so we don't process it again
            gameStartReceived = false;
        }
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                // Update graphics manager when window is resized
                graphicsManager.updateView();
            } else if (gameHasStarted && myPlayerSide == gameState.getActivePlayer()) {
                // Only handle input if it's the player's turn
                inputManager.handleEvent(event);
            }
        }

        // --- Network Receive ---
        sf::Packet receivedPacket;
        sf::Socket::Status status = socket.receive(receivedPacket);
        if (status == sf::Socket::Done) {
            std::cout << "Received packet from server" << std::endl;
            MessageType messageType;
            if (receivedPacket >> messageType) {
                std::cout << "Message type: " << static_cast<int>(messageType) << std::endl;
                switch (messageType) {
                    case MessageType::PlayerAssignment:
                        uint8_t side_uint8;
                        if (receivedPacket >> side_uint8) {
                            myPlayerSide = static_cast<PlayerSide>(side_uint8);
                            uiMessage = "Assigned player side: Player ";
                            uiMessage += (myPlayerSide == PlayerSide::PLAYER_ONE ? "One" : "Two");
                            std::cout << uiMessage << std::endl;
                        } else { std::cerr << "Error deserializing PlayerAssignment data." << std::endl; }
                        break;
                    case MessageType::CardCollectionData:
                        {
                            std::string data;
                            if (receivedPacket >> data) {
                                myCollection.deserialize(data);
                                std::cout << "Collection received with " << myCollection.size() << " cards" << std::endl;
                            }
                        }
                        break;
                    case MessageType::DeckData:
                        {
                            std::string data;
                            if (receivedPacket >> data) {
                                myDeck.deserialize(data);
                                std::cout << "Deck received with " << myDeck.size() << " cards" << std::endl;
                            }
                        }
                        break;
                    case MessageType::WaitingForOpponent:
                        uiMessage = "Waiting for opponent...";
                        std::cout << uiMessage << std::endl;
                        break;
                    case MessageType::GameStart:
                        {
                            std::string p1_username, p2_username;
                            int p1_rating, p2_rating;

                            if (receivedPacket >> p1_username >> p1_rating >> p2_username >> p2_rating >> gameState) {
                                gameHasStarted = true;
                                printBoardState(gameState, myPlayerSide); // Keep this for debugging

                                // Determine local and remote player data
                                if (myPlayerSide == PlayerSide::PLAYER_ONE) {
                                    localPlayerUsernameText.setString("You: " + p1_username);
                                    localPlayerRatingText.setString("Rating: " + std::to_string(p1_rating));
                                    remotePlayerUsernameText.setString("Opponent: " + p2_username);
                                    remotePlayerRatingText.setString("Rating: " + std::to_string(p2_rating));
                                } else if (myPlayerSide == PlayerSide::PLAYER_TWO) {
                                    localPlayerUsernameText.setString("You: " + p2_username);
                                    localPlayerRatingText.setString("Rating: " + std::to_string(p2_rating));
                                    remotePlayerUsernameText.setString("Opponent: " + p1_username);
                                    remotePlayerRatingText.setString("Rating: " + std::to_string(p1_rating));
                                }
                                uiMessage = "Game started!"; 
                                std::cout << uiMessage << " P1: " << p1_username << " (" << p1_rating << "), P2: " << p2_username << " (" << p2_rating << ")" << std::endl;

                                // Set positions for username/rating texts (using base resolution coordinates)
                                // Account for phase text by adding extra spacing
                                localPlayerUsernameText.setPosition(10.f, uiMessageText.getPosition().y + 55.f);
                                localPlayerRatingText.setPosition(10.f, localPlayerUsernameText.getPosition().y + 20.f);
                                localPlayerSteamText.setPosition(10.f, localPlayerRatingText.getPosition().y + 20.f);
                                
                                // Update local player steam display
                                int localPlayerSteam = gameState.getSteam(myPlayerSide);
                                localPlayerSteamText.setString("Steam: " + std::to_string(localPlayerSteam));
                                
                                // Position remote player text on the right side
                                float remoteTextWidthEstimate = 200.f; 
                                remotePlayerUsernameText.setPosition(GraphicsManager::BASE_WIDTH - remoteTextWidthEstimate - 10.f, uiMessageText.getPosition().y + 55.f);
                                remotePlayerRatingText.setPosition(GraphicsManager::BASE_WIDTH - remoteTextWidthEstimate - 10.f, remotePlayerUsernameText.getPosition().y + 20.f);

                            } else {
                                std::cerr << "Error deserializing GameStart data (with user info)." << std::endl;
                            }
                        }
                        break;
                    case MessageType::GameStateUpdate:
                        if (gameHasStarted) {
                            if (receivedPacket >> gameState) { // Deserialize the updated GameState
                                // uiMessage = (myPlayerSide == gameState.getActivePlayer() ? "Your turn" : "Opponent's turn");
                                std::cout << "GameState updated. Turn: " << gameState.getTurnNumber() << std::endl;
                                printBoardState(gameState, myPlayerSide);
                                
                                // Update local player steam display
                                int localPlayerSteam = gameState.getSteam(myPlayerSide);
                                localPlayerSteamText.setString("Steam: " + std::to_string(localPlayerSteam));
                            } else { std::cerr << "Error deserializing GameStateUpdate." << std::endl; }
                        }
                        break;
                    case MessageType::MoveRejected: // Optional
                        uiMessage = "Move rejected by server.";
                        std::cout << uiMessage << std::endl;
                        
                        // Reset input manager state
                        inputManager.resetInputState();
                        break;
                    case MessageType::CardPlayRejected: // Optional
                        uiMessage = "Card play rejected by server.";
                        std::cout << uiMessage << std::endl;
                        
                        // Reset card selection state in input manager
                        inputManager.resetCardSelection();
                        break;
                    case MessageType::Error: // Example, server might send string
                        // std::string errorMessage;
                        // if (receivedPacket >> errorMessage) {
                        //     uiMessage = "Server error: " + errorMessage;
                        //     std::cerr << uiMessage << std::endl;
                        // } else { std::cerr << "Error deserializing Error message data." << std::endl; }
                        break;
                    default:
                        std::cout << "Received unhandled/unknown message type: " << static_cast<int>(messageType) << std::endl;
                }
            }
        } else if (status == sf::Socket::NotReady) {
            // No data received, non-blocking socket, this is normal
        } else if (status == sf::Socket::Disconnected) {
            uiMessage = "Connection to server lost.";
            std::cerr << uiMessage << std::endl;
            window.close(); // Or handle reconnection
        } else if (status == sf::Socket::Error) {
            std::cerr << "Network error receiving data." << std::endl;
            // Potentially handle disconnection or error
        }
        // --- End Network Receive ---
        
        // Apply the game view for proper scaling
        graphicsManager.applyView();
        
        // Clear screen with a dark green background (bayou-like)
        window.clear(sf::Color(10, 50, 20));
        
        // Draw game elements here (based on the local gameState, now updated by server)
        
        // Update UI message
        if (gameHasStarted) {
            std::string phaseStr;
            switch (gameState.getGamePhase()) {
                case GamePhase::SETUP: phaseStr = "Setup"; break;
                case GamePhase::DRAW: phaseStr = "Drawing"; break;
                case GamePhase::PLAY: phaseStr = "Action"; break;
                case GamePhase::MOVE: phaseStr = "Action"; break; // Legacy support
                case GamePhase::GAME_OVER: phaseStr = "Game Over"; break;
                default: phaseStr = "Unknown"; break;
            }
            
            // Update turn message (without phase)
            uiMessage = (myPlayerSide == gameState.getActivePlayer() ? "Your turn (Player " : "Opponent's turn (Player ");
            uiMessage += (gameState.getActivePlayer() == PlayerSide::PLAYER_ONE ? "One" : "Two");
            uiMessage += ")";
            
            // Update phase message separately (no instructions needed since actions auto-end turns)
            std::string phaseMessage = phaseStr + " Phase";
            phaseText.setString(phaseMessage);
        }
        uiMessageText.setString(uiMessage);
        window.draw(uiMessageText);
        
        // Draw phase text if game has started
        if (gameHasStarted) {
            window.draw(phaseText);
        }

        // Draw username/rating info if game has started
        if (gameHasStarted) {
            window.draw(localPlayerUsernameText);
            window.draw(localPlayerRatingText);
            window.draw(remotePlayerUsernameText);
            window.draw(remotePlayerRatingText);
            window.draw(localPlayerSteamText);
        }

        // --- Game Board Rendering ---
        const GameBoard& board = gameState.getBoard();
        auto boardParams = graphicsManager.getBoardRenderParams();

        sf::Color lightSquareColor(170, 210, 130);
        sf::Color darkSquareColor(100, 150, 80);

        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                sf::Vector2f squarePos = graphicsManager.boardToGame(x, y);
                sf::RectangleShape squareShape(sf::Vector2f(boardParams.squareSize, boardParams.squareSize));
                squareShape.setPosition(squarePos);

                // Alternate colors for chessboard pattern
                if ((x + y) % 2 == 0) {
                    squareShape.setFillColor(lightSquareColor);
                } else {
                    squareShape.setFillColor(darkSquareColor);
                }
                window.draw(squareShape);
            }
        }
        // --- End Game Board Rendering ---

        // --- Control Visualization ---
        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                const Square& square = board.getSquare(x, y);
                PlayerSide controller = InfluenceSystem::getControllingPlayer(square);
                
                if (controller != PlayerSide::NEUTRAL) {
                    sf::Vector2f squarePos = graphicsManager.boardToGame(x, y);
                    sf::RectangleShape controlIndicator(sf::Vector2f(boardParams.squareSize, boardParams.squareSize));
                    controlIndicator.setPosition(squarePos);
                    
                    if (controller == PlayerSide::PLAYER_ONE) {
                        // More visible blue tint for Player One control
                        controlIndicator.setFillColor(sf::Color(0, 100, 255, 120)); // Blue with higher alpha
                    } else if (controller == PlayerSide::PLAYER_TWO) {
                        // More visible red tint for Player Two control
                        controlIndicator.setFillColor(sf::Color(255, 50, 0, 120)); // Red with higher alpha
                    }
                    
                    window.draw(controlIndicator);
                }
            }
        }
        // --- End Control Visualization ---

        // --- Move Highlighting ---
        std::vector<Position> highlightSquares;
        const Piece* highlightPiece = nullptr;
        if (inputManager.isPieceSelected() && inputManager.getSelectedPiece()) {
            highlightPiece = inputManager.getSelectedPiece();
        } else {
            sf::Vector2i mouseScreen = sf::Mouse::getPosition(window);
            sf::Vector2f mouseGame = graphicsManager.screenToGame(mouseScreen);
            sf::Vector2i boardCoords = graphicsManager.gameToBoard(mouseGame);
            if (boardCoords.x >= 0 && boardCoords.y >= 0) {
                const Square& hovered = board.getSquare(boardCoords.x, boardCoords.y);
                if (!hovered.isEmpty()) {
                    highlightPiece = hovered.getPiece();
                }
            }
        }

        if (highlightPiece) {
            highlightSquares = highlightPiece->getValidMoves(board);
        }

        for (const Position& pos : highlightSquares) {
            sf::Vector2f squarePos = graphicsManager.boardToGame(pos.x, pos.y);
            sf::RectangleShape highlightRect(sf::Vector2f(boardParams.squareSize, boardParams.squareSize));
            highlightRect.setPosition(squarePos);
            highlightRect.setFillColor(sf::Color(255, 255, 0, 120));
            window.draw(highlightRect);
        }
        // --- End Move Highlighting ---

        // --- Piece Rendering ---
        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                // Skip drawing the selected piece here if it's being dragged
                if (inputManager.isPieceSelected() && 
                    x == inputManager.getOriginalSquareCoords().x && 
                    y == inputManager.getOriginalSquareCoords().y) {
                    continue; 
                }

                const Square& square = board.getSquare(x, y);
                if (!square.isEmpty()) {
                    Piece* piece = square.getPiece();
                    auto texIt = pieceTextures.find(piece->getTypeName());
                    if (texIt != pieceTextures.end()) {
                        sf::Vector2f piecePos = graphicsManager.boardToGame(x, y);
                        sf::Sprite spr(texIt->second);
                        spr.setPosition(piecePos);
                        
                        // Scale based on actual texture dimensions
                        sf::Vector2u textureSize = texIt->second.getSize();
                        float scaleX = boardParams.squareSize / static_cast<float>(textureSize.x);
                        float scaleY = boardParams.squareSize / static_cast<float>(textureSize.y);
                        if (piece->getSide() == PlayerSide::PLAYER_TWO) {
                            spr.setOrigin(static_cast<float>(textureSize.x), 0.f);
                            spr.setScale(-scaleX, scaleY);
                        } else {
                            spr.setScale(scaleX, scaleY);
                        }
                        window.draw(spr);
                    }

                    // Render health bar
                    renderHealthBar(window, piece,
                                    piecePos.x,
                                    piecePos.y,
                                    boardParams.squareSize);

                    // Render attack value
                    renderAttackValue(window, piece,
                                     piecePos.x,
                                     piecePos.y,
                                     boardParams.squareSize);
                }
            }
        }

        // Draw the selected piece at the mouse cursor position if it's being dragged
        if (inputManager.isPieceSelected() && inputManager.getSelectedPiece()) {
            sf::Vector2f mouseOffset = inputManager.getMouseOffset();
            sf::Vector2f currentMousePosition = inputManager.getCurrentMousePosition();
            float draggedPieceX = currentMousePosition.x - mouseOffset.x;
            float draggedPieceY = currentMousePosition.y - mouseOffset.y;

            Piece* draggedPiece = inputManager.getSelectedPiece();
            auto texIt = pieceTextures.find(draggedPiece->getTypeName());
            if (texIt != pieceTextures.end()) {
                sf::Sprite spr(texIt->second);
                spr.setPosition(draggedPieceX, draggedPieceY);

                // Scale based on actual texture dimensions
                sf::Vector2u textureSize = texIt->second.getSize();
                float scaleX = boardParams.squareSize / static_cast<float>(textureSize.x);
                float scaleY = boardParams.squareSize / static_cast<float>(textureSize.y);
                if (draggedPiece->getSide() == PlayerSide::PLAYER_TWO) {
                    spr.setOrigin(static_cast<float>(textureSize.x), 0.f);
                    spr.setScale(-scaleX, scaleY);
                } else {
                    spr.setScale(scaleX, scaleY);
                }
                window.draw(spr);
            }
            
            // Render health bar for the dragged piece
            renderHealthBar(window, draggedPiece, draggedPieceX, draggedPieceY, boardParams.squareSize);

            // Render attack value for the dragged piece
            renderAttackValue(window, draggedPiece, draggedPieceX, draggedPieceY, boardParams.squareSize);
        }
        // --- End Piece Rendering ---
        
        // --- Card Hand Rendering ---
        if (gameHasStarted) {
            int selectedCard = inputManager.isCardSelected() ? inputManager.getSelectedCardIndex() : -1;
            renderPlayerHand(window, gameState, myPlayerSide, graphicsManager, selectedCard);

            // Draw dragged card on top of everything
            if (inputManager.isCardSelected() && inputManager.isWaitingForCardTarget()) {
                const Hand& hand = gameState.getHand(myPlayerSide);
                const Card* card = hand.getCard(inputManager.getSelectedCardIndex());
                if (card) {
                    sf::Vector2f mouseOffset = inputManager.getMouseOffset();
                    sf::Vector2f currentMousePosition = inputManager.getCurrentMousePosition();
                    float cardX = currentMousePosition.x - mouseOffset.x;
                    float cardY = currentMousePosition.y - mouseOffset.y;

                    float cardWidth = 120.f;
                    float cardHeight = 120.f;

                    sf::RectangleShape cardRect(sf::Vector2f(cardWidth, cardHeight));
                    cardRect.setPosition(cardX, cardY);
                    cardRect.setFillColor(sf::Color(60, 80, 60, 200));
                    cardRect.setOutlineColor(sf::Color::Yellow);
                    cardRect.setOutlineThickness(2.f);
                    window.draw(cardRect);

                    sf::Text nameText;
                    nameText.setFont(globalFont);
                    nameText.setCharacterSize(14);
                    nameText.setFillColor(sf::Color::White);
                    nameText.setString(card->getName());
                    sf::FloatRect nameBounds = nameText.getLocalBounds();
                    nameText.setPosition(cardX + (cardWidth - nameBounds.width) / 2.f, cardY + 10.f);
                    window.draw(nameText);

                    sf::Text costText;
                    costText.setFont(globalFont);
                    costText.setCharacterSize(16);
                    costText.setFillColor(sf::Color::Cyan);
                    costText.setString("Steam: " + std::to_string(card->getSteamCost()));
                    sf::FloatRect costBounds = costText.getLocalBounds();
                    costText.setPosition(cardX + (cardWidth - costBounds.width) / 2.f, cardY + cardHeight - 25.f);
                    window.draw(costText);
                }
            }
        }
        // --- End Card Hand Rendering ---
        
        // --- Win Message Display ---
        if (showWinMessage && !winMessage.empty()) {
            // Create a semi-transparent overlay
            sf::RectangleShape overlay(sf::Vector2f(GraphicsManager::BASE_WIDTH, GraphicsManager::BASE_HEIGHT));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);
            
            // Create win message text
            sf::Text winText;
            winText.setFont(globalFont);
            winText.setString(winMessage);
            winText.setCharacterSize(48);
            winText.setFillColor(sf::Color::Yellow);
            winText.setStyle(sf::Text::Bold);
            
            // Center the text
            sf::FloatRect textBounds = winText.getLocalBounds();
            winText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                             textBounds.top + textBounds.height / 2.0f);
            winText.setPosition(GraphicsManager::BASE_WIDTH / 2.0f, GraphicsManager::BASE_HEIGHT / 2.0f);
            
            window.draw(winText);
            
            // Add instruction text
            sf::Text instructionText;
            instructionText.setFont(globalFont);
            instructionText.setString("Press any key to continue...");
            instructionText.setCharacterSize(24);
            instructionText.setFillColor(sf::Color::White);
            
            sf::FloatRect instrBounds = instructionText.getLocalBounds();
            instructionText.setOrigin(instrBounds.left + instrBounds.width / 2.0f,
                                     instrBounds.top + instrBounds.height / 2.0f);
            instructionText.setPosition(GraphicsManager::BASE_WIDTH / 2.0f, GraphicsManager::BASE_HEIGHT / 2.0f + 80.0f);
            
            window.draw(instructionText);
        }
        // --- End Win Message Display ---
        
        // Update the window
        window.display();
    }
    
    return 0;
}
