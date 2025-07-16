#include "GraphicsManager.h"
#include "GameBoard.h"
#include <algorithm>

namespace BayouBonanza {

GraphicsManager::GraphicsManager(sf::RenderWindow& window) 
    : window(window), scaleFactor(1.0f), viewOffset(0.0f, 0.0f) {
    
    // Initialize the game view with base resolution
    gameView.setSize(BASE_WIDTH, BASE_HEIGHT);
    gameView.setCenter(BASE_WIDTH / 2.0f, BASE_HEIGHT / 2.0f);
    
    // Calculate initial view parameters
    calculateViewParameters();
}

void GraphicsManager::updateView() {
    calculateViewParameters();
}

void GraphicsManager::calculateViewParameters() {
    sf::Vector2u windowSize = window.getSize();
    float windowWidth = static_cast<float>(windowSize.x);
    float windowHeight = static_cast<float>(windowSize.y);
    
    // Calculate the window's aspect ratio
    float windowAspectRatio = windowWidth / windowHeight;
    
    // Calculate scale factor to fit the base resolution into the window
    // while maintaining aspect ratio
    if (windowAspectRatio > BASE_ASPECT_RATIO) {
        // Window is wider than base aspect ratio - fit to height
        scaleFactor = windowHeight / BASE_HEIGHT;
        float scaledWidth = BASE_WIDTH * scaleFactor;
        viewOffset.x = (windowWidth - scaledWidth) / 2.0f;
        viewOffset.y = 0.0f;
    } else {
        // Window is taller than base aspect ratio - fit to width
        scaleFactor = windowWidth / BASE_WIDTH;
        float scaledHeight = BASE_HEIGHT * scaleFactor;
        viewOffset.x = 0.0f;
        viewOffset.y = (windowHeight - scaledHeight) / 2.0f;
    }
    
    // Set up the viewport for letterboxing
    sf::FloatRect viewport;
    viewport.left = viewOffset.x / windowWidth;
    viewport.top = viewOffset.y / windowHeight;
    viewport.width = (BASE_WIDTH * scaleFactor) / windowWidth;
    viewport.height = (BASE_HEIGHT * scaleFactor) / windowHeight;
    
    gameView.setViewport(viewport);
}

sf::Vector2f GraphicsManager::screenToGame(const sf::Vector2i& screenPos) const {
    // Convert screen coordinates to game coordinates
    sf::Vector2f worldPos = window.mapPixelToCoords(screenPos, gameView);
    return worldPos;
}

sf::Vector2i GraphicsManager::gameToScreen(const sf::Vector2f& gamePos) const {
    // Convert game coordinates to screen coordinates
    sf::Vector2i screenPos = window.mapCoordsToPixel(gamePos, gameView);
    return screenPos;
}

sf::Vector2i GraphicsManager::gameToBoard(const sf::Vector2f& gamePos) const {
    BoardRenderParams params = getBoardRenderParams();

    int boardX = static_cast<int>((gamePos.x - params.boardStartX) / params.squareSize);
    int boardY = static_cast<int>((gamePos.y - params.boardStartY) / params.squareSize);

    if (boardX >= 0 && boardX < GameBoard::BOARD_SIZE &&
        boardY >= 0 && boardY < GameBoard::BOARD_SIZE) {
        return sf::Vector2i(boardX, boardY);
    }

    return sf::Vector2i(-1, -1);
}

sf::Vector2f GraphicsManager::boardToGame(int boardX, int boardY) const {
    BoardRenderParams params = getBoardRenderParams();
    float gameX = params.boardStartX + boardX * params.squareSize;
    float gameY = params.boardStartY + boardY * params.squareSize;
    return sf::Vector2f(gameX, gameY);
}

GraphicsManager::BoardRenderParams GraphicsManager::getBoardRenderParams() const {
    BoardRenderParams params;
    
    // Use 80% of the base height for the board size
    params.boardSize = BASE_HEIGHT * 0.8f;
    params.squareSize = params.boardSize / GameBoard::BOARD_SIZE;
    
    // Position the board higher up to make room for taller cards
    params.boardStartX = (BASE_WIDTH - params.boardSize) / 2.0f;
    params.boardStartY = (BASE_HEIGHT - params.boardSize) / 2.0f - 60.0f; // Move board up by 60 pixels
    
    return params;
}

void GraphicsManager::applyView() {
    window.setView(gameView);
}

} // namespace BayouBonanza 