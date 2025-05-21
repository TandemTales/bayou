#pragma once

#include <memory>
#include <vector>
#include "Move.h"
#include "GameState.h"

namespace BayouBonanza {

/**
 * @brief Result of a move execution
 */
enum class MoveResult {
    SUCCESS,          // Move was executed successfully
    INVALID_MOVE,     // Move is invalid
    PIECE_DESTROYED,  // Target piece was destroyed
    KING_CAPTURED,    // A king was captured (game ending condition)
    ERROR             // An error occurred during execution
};

/**
 * @brief Class responsible for executing moves and handling combat
 */
class MoveExecutor {
public:
    /**
     * @brief Default constructor
     */
    MoveExecutor() = default;
    
    /**
     * @brief Validate a move
     * 
     * @param gameState Current game state
     * @param move The move to validate
     * @return true if the move is valid
     */
    bool validateMove(const GameState& gameState, const Move& move) const;
    
    /**
     * @brief Execute a move
     * 
     * This method handles moving pieces, combat resolution, and updates
     * the game state accordingly.
     * 
     * @param gameState Game state to modify
     * @param move The move to execute
     * @return Result of the move execution
     */
    MoveResult executeMove(GameState& gameState, const Move& move);
    
    /**
     * @brief Get all valid moves for a piece
     * 
     * @param gameState Current game state
     * @param piece The piece to check
     * @return Vector of valid moves
     */
    std::vector<Move> getValidMoves(const GameState& gameState, std::shared_ptr<Piece> piece) const;
    
    /**
     * @brief Handle combat between two pieces
     * 
     * @param attacker The attacking piece
     * @param defender The defending piece
     * @param gameState Game state to update
     * @return true if the defender was destroyed
     */
    bool resolveCombat(std::shared_ptr<Piece> attacker, std::shared_ptr<Piece> defender, GameState& gameState);
    
    /**
     * @brief Recalculate board control
     * 
     * Updates the control values for all squares on the board based on
     * the current piece positions and influence.
     * 
     * @param gameState Game state to update
     */
    void recalculateBoardControl(GameState& gameState);
};

} // namespace BayouBonanza
