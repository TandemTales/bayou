#pragma once

#include <SFML/Graphics.hpp>

namespace BayouBonanza {

class GraphicsManager {
public:
    // Base resolution (16:9 aspect ratio)
    static constexpr float BASE_WIDTH = 1280.0f;
    static constexpr float BASE_HEIGHT = 720.0f;
    static constexpr float BASE_ASPECT_RATIO = BASE_WIDTH / BASE_HEIGHT;

    GraphicsManager(sf::RenderWindow& window);
    
    // Update the view when window is resized
    void updateView();
    
    // Get the current scale factor
    float getScaleFactor() const { return scaleFactor; }
    
    // Get the game view (scaled and letterboxed)
    const sf::View& getGameView() const { return gameView; }
    
    // Convert screen coordinates to game coordinates
    sf::Vector2f screenToGame(const sf::Vector2i& screenPos) const;
    
    // Convert game coordinates to screen coordinates
    sf::Vector2i gameToScreen(const sf::Vector2f& gamePos) const;

    // Convert game coordinates to board coordinates
    sf::Vector2i gameToBoard(const sf::Vector2f& gamePos) const;
    
    // Get the game board rendering parameters
    struct BoardRenderParams {
        float boardSize;
        float squareSize;
        float boardStartX;
        float boardStartY;
    };
    
    BoardRenderParams getBoardRenderParams() const;
    
    // Apply the game view to the window
    void applyView();

private:
    sf::RenderWindow& window;
    sf::View gameView;
    float scaleFactor;
    sf::Vector2f viewOffset;
    
    void calculateViewParameters();
};

} // namespace BayouBonanza 