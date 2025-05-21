#pragma once

#include <array>
#include <memory>
#include "Square.h"
#include "PlayerSide.h"

namespace BayouBonanza {

/**
 * @brief Represents the game board as an 8x8 grid
 * 
 * This class provides the main board representation and methods to interact with it.
 * The board is implemented as a 2D array of Square objects.
 */
class GameBoard {
public:
    static constexpr int BOARD_SIZE = 8;

    /**
     * @brief Default constructor, initializes an empty board
     */
    GameBoard();

    /**
     * @brief Get a reference to a square at the specified position
     * 
     * @param x X-coordinate (0-7)
     * @param y Y-coordinate (0-7)
     * @return Square& Reference to the square
     */
    Square& getSquare(int x, int y);
    
    /**
     * @brief Get a const reference to a square at the specified position
     * 
     * @param x X-coordinate (0-7)
     * @param y Y-coordinate (0-7)
     * @return const Square& Const reference to the square
     */
    const Square& getSquare(int x, int y) const;
    
    /**
     * @brief Check if a position is within the board boundaries
     * 
     * @param x X-coordinate
     * @param y Y-coordinate
     * @return true if position is valid
     */
    bool isValidPosition(int x, int y) const;
    
    /**
     * @brief Reset the board to its initial state (empty)
     */
    void resetBoard();
    
    /**
     * @brief Calculate and update control values for each square
     * 
     * This method should be called after each piece move to update
     * the control values of each square on the board.
     */
    void recalculateControlValues();

private:
    std::array<std::array<Square, BOARD_SIZE>, BOARD_SIZE> board;
};

} // namespace BayouBonanza
