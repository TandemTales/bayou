#include "GameRendering.h"
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <algorithm>
#include <cmath>

namespace BayouBonanza {

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

void renderAttackValue(sf::RenderWindow& window, const Piece* piece, float squareX, float squareY, float squareSize, const sf::Font& font) {
    if (!piece) return;

    sf::Text attackText;
    attackText.setFont(font);
    attackText.setString(std::to_string(piece->getAttack()));
    attackText.setCharacterSize(static_cast<unsigned int>(squareSize * 0.2f));
    attackText.setFillColor(sf::Color::White);

    sf::FloatRect bounds = attackText.getLocalBounds();
    attackText.setOrigin(bounds.left + bounds.width, bounds.top + bounds.height);

    float offset = squareSize * 0.05f; // Small margin from the square edges
    attackText.setPosition(squareX + squareSize - offset, squareY + squareSize - offset);

    window.draw(attackText);
}
} // namespace BayouBonanza
