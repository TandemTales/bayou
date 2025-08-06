#include "Menu.h"
#include "NetworkProtocol.h"
#include "GameState.h"
#include <iostream>

namespace BayouBonanza {

std::string runLoginScreen(sf::RenderWindow& window,
                           GraphicsManager& graphicsManager,
                           const sf::Font& font) {
    std::string username;

    sf::Text promptText;
    promptText.setFont(font);
    promptText.setCharacterSize(32);
    promptText.setFillColor(sf::Color::White);
    promptText.setString("Enter Username");

    sf::Text inputText;
    inputText.setFont(font);
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
                if (event.text.unicode == 8) {
                    if (!username.empty()) username.pop_back();
                } else if (event.text.unicode == 13 || event.text.unicode == 10) {
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

void showPlaceholderScreen(sf::RenderWindow& window,
                           GraphicsManager& graphicsManager,
                           const std::string& message,
                           const sf::Font& font) {
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
        text.setFont(font);
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
                           const sf::Font& font) {
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
                        case 1: {
                            sf::Packet matchmakingPacket;
                            matchmakingPacket << MessageType::RequestMatchmaking;
                            if (socket.send(matchmakingPacket) == sf::Socket::Done) {
                                std::cout << "Matchmaking request sent to server" << std::endl;
                            } else {
                                std::cerr << "Failed to send matchmaking request" << std::endl;
                            }
                            break;
                        }
                        case 2:
                            return MainMenuOption::PLAY_AI;
                    }
                }
            } else if (event.type == sf::Event::Resized) {
                graphicsManager.updateView();
            }
        }

        sf::Packet receivedPacket;
        sf::Socket::Status status = socket.receive(receivedPacket);
        if (status == sf::Socket::Done) {
            MessageType messageType;
            if (receivedPacket >> messageType) {
                switch (messageType) {
                    case MessageType::PlayerAssignment: {
                        uint8_t side_uint8;
                        if (receivedPacket >> side_uint8) {
                            playerSide = static_cast<PlayerSide>(side_uint8);
                            std::cout << "Assigned player side: Player "
                                      << (playerSide == PlayerSide::PLAYER_ONE ? "One" : "Two")
                                      << std::endl;
                        }
                        break;
                    }
                    case MessageType::CardCollectionData: {
                        std::string data;
                        if (receivedPacket >> data) {
                            collection.deserialize(data);
                            std::cout << "Collection received with " << collection.size() << " cards" << std::endl;
                        }
                        break;
                    }
                    case MessageType::DeckData: {
                        std::string data;
                        if (receivedPacket >> data) {
                            deck.deserialize(data);
                            std::cout << "Deck received with " << deck.size() << " cards" << std::endl;
                        }
                        break;
                    }
                    case MessageType::WaitingForOpponent:
                        std::cout << "Waiting for opponent..." << std::endl;
                        break;
                    case MessageType::GameStart: {
                        std::cout << "Game start received in main menu - storing packet data and transitioning to game!" << std::endl;
                        gameStartPacketData.clear();
                        gameStartPacketData << MessageType::GameStart;
                        std::string p1_username, p2_username;
                        int p1_rating, p2_rating;
                        GameState tempGameState;
                        if (receivedPacket >> p1_username >> p1_rating >> p2_username >> p2_rating >> tempGameState) {
                            gameStartPacketData << p1_username << p1_rating << p2_username << p2_rating << tempGameState;
                            gameStartReceived = true;
                            return MainMenuOption::PLAY_HUMAN;
                        } else {
                            std::cerr << "Error: Failed to deserialize GameStart data in main menu" << std::endl;
                        }
                        break;
                    }
                    default:
                        std::cout << "Received unhandled message type in main menu: "
                                  << static_cast<int>(messageType) << std::endl;
                        break;
                }
            }
        }

        graphicsManager.applyView();
        window.clear(sf::Color(10, 50, 20));

        sf::Text title;
        title.setFont(font);
        title.setString("Main Menu");
        title.setCharacterSize(40);
        title.setFillColor(sf::Color::White);
        sf::FloatRect titleBounds = title.getLocalBounds();
        title.setOrigin(titleBounds.left + titleBounds.width / 2.f,
                         titleBounds.top + titleBounds.height / 2.f);
        title.setPosition(GraphicsManager::BASE_WIDTH / 2.f,
                           GraphicsManager::BASE_HEIGHT / 2.f - 120.f);
        window.draw(title);

        sf::Text playerInfo;
        playerInfo.setFont(font);
        playerInfo.setString("Player: " + username + " | Rating: " + std::to_string(currentRating));
        playerInfo.setCharacterSize(20);
        playerInfo.setFillColor(sf::Color::Cyan);
        sf::FloatRect playerBounds = playerInfo.getLocalBounds();
        playerInfo.setOrigin(playerBounds.left + playerBounds.width / 2.f,
                             playerBounds.top + playerBounds.height / 2.f);
        playerInfo.setPosition(GraphicsManager::BASE_WIDTH / 2.f,
                                GraphicsManager::BASE_HEIGHT / 2.f - 80.f);
        window.draw(playerInfo);

        for (int i = 0; i < optionCount; ++i) {
            sf::Text option;
            option.setFont(font);
            option.setString(optionTexts[i]);
            option.setCharacterSize(28);
            option.setFillColor(i == selected ? sf::Color::Yellow : sf::Color::White);
            sf::FloatRect b = option.getLocalBounds();
            option.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f);
            option.setPosition(GraphicsManager::BASE_WIDTH / 2.f,
                                GraphicsManager::BASE_HEIGHT / 2.f - 20.f + i * 50.f);
            window.draw(option);
        }

        window.display();
    }

    return MainMenuOption::NONE;
}

} // namespace BayouBonanza
