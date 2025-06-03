#pragma once

#include "GameBoard.h"
#include "PlayerSide.h"
#include "PieceData.h"

namespace BayouBonanza {

// Forward declarations
class Square;

/**
 * @brief System for calculating piece influence on squares and determining square control
 * 
 * This class provides static methods to calculate how pieces influence adjacent squares
 * and determine which player controls each square on the board.
 * 
 * Control is persistent - once a player gains control of a square, they retain it
 * until another player gains MORE influence over that square.
 */
class InfluenceSystem {
public:
    /**
     * @brief Calculate influence for all pieces on the board and update square control
     * 
     * This is the main method that resets all influence values, calculates influence
     * for each piece, and updates square control using sticky control logic.
     * 
     * @param board The game board to calculate influence for
     */
    static void calculateBoardInfluence(GameBoard& board);
    
    /**
     * @brief Calculate influence for a single piece at the given position
     * 
     * @param board The game board
     * @param piecePos Position of the piece to calculate influence for
     */
    static void calculatePieceInfluence(GameBoard& board, Position piecePos);
    
    /**
     * @brief Calculate influence for all pieces on the board
     * 
     * @param board The game board to calculate influence for
     */
    static void calculateAllPieceInfluence(GameBoard& board);
    
    /**
     * @brief Reset all influence values to 0 (does NOT reset persistent control)
     * 
     * @param board The game board to reset influence values for
     */
    static void resetInfluenceValues(GameBoard& board);
    
    /**
     * @brief Update square control based on current influence using sticky control logic
     * 
     * Control only changes when another player has MORE influence than the current controller.
     * 
     * @param board The game board to update control for
     */
    static void updateSquareControlFromInfluence(GameBoard& board);
    
    /**
     * @brief Determine square control (legacy method, calls updateSquareControlFromInfluence)
     * 
     * @param board The game board to determine control for
     */
    static void determineSquareControl(GameBoard& board);
    
    /**
     * @brief Get the player that controls a specific square
     * 
     * @param square The square to check
     * @return PlayerSide that controls the square (NEUTRAL if no one controls it)
     */
    static PlayerSide getControllingPlayer(const Square& square);
};

} // namespace BayouBonanza 