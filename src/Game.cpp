#include "Game.h"
#include "Menu.h"
#include "CardFactory.h"
#include "Square.h"
#include "HandRenderer.h"
#include "NetworkProtocol.h"
#include "CardPlayValidator.h"
#include "InfluenceSystem.h"
#include "Piece.h"
#include "DeckEditor.h" // Include the new DeckEditor header
#include <iostream>

using namespace BayouBonanza;

Game* Game::s_pInstance = nullptr;

Game::Game()
    : m_window(sf::VideoMode(1280, 720), "Bayou Bonanza"),
      m_graphicsManager(m_window),
      m_gameHasStarted(false),
      m_myPlayerSide(PlayerSide::NEUTRAL),
      m_inputManager(m_window, m_gameState, m_gameHasStarted, m_myPlayerSide, m_graphicsManager),
      m_returnToMenuRequested(false),
      m_myCurrentRating(0),
      m_gameStartReceived(false)
{
    s_pInstance = this;
    GameOverDetector::registerWinConditionCallback(Game::gameOverCallback);
}

Game::~Game() {
    s_pInstance = nullptr;
}

void Game::gameOverCallback(PlayerSide winner, const std::string& description) {
    if (s_pInstance) {
        s_pInstance->onWinCondition(winner, description);
    }
}

void Game::run() {
    if (!initialize()) {
        cleanup();
        return;
    }
    while (m_window.isOpen()) {
        m_gameHasStarted = false;
        m_returnToMenuRequested = false;
        if(m_uiManager) m_uiManager->hideEndScreen();
        m_gameState = GameState();

        MainMenuOption menuChoice = MainMenuOption::NONE;
        if (!m_gameStartReceived) {
            do {
                // menuChoice = runMainMenu(...)
                if (menuChoice == MainMenuOption::DECK_EDITOR) {
                    DeckEditor editor(m_window, m_graphicsManager, *m_networkManager, m_globalFont, m_myCollection, m_myDeck, m_pieceDefManager);
                    editor.run();
                } else if (menuChoice == MainMenuOption::PLAY_AI) {
                    showPlaceholderScreen(m_window, m_graphicsManager, "Play vs AI Coming Soon", m_globalFont);
                }
            } while (m_window.isOpen() && menuChoice != MainMenuOption::PLAY_HUMAN && !m_gameStartReceived);
        }

        if (!m_window.isOpen()) break;

        if (m_gameStartReceived || menuChoice == MainMenuOption::PLAY_HUMAN) {
            runGameLoop();
        }
    }
    cleanup();
}

// ... other Game methods remain the same ...

bool Game::initialize() { /* ... */ return true; }
void Game::runGameLoop() { /* ... */ }
void Game::processEvents() { /* ... */ }
void Game::processNetworkMessages() { /* ... */ }
void Game::handleGameStart(sf::Packet& packet) { /* ... */ }
void Game::update(float dt) { /* ... */ }
void Game::render() { /* ... */ }
void Game::onWinCondition(PlayerSide winner, const std::string& description) { /* ... */ }
void Game::cleanup() { /* ... */ }
void Game::renderBoard() { /* ... */ }
void Game::renderHighlights() { /* ... */ }
void Game::renderPieces() { /* ... */ }
void Game::renderPiece(const Piece* piece, sf::Vector2f position) { /* ... */ }
void Game::renderDraggedCard() { /* ... */ }
