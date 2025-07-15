#include "MoveExecutor.h"
#include "GameBoard.h"
#include "InfluenceSystem.h"
// #include "King.h" // Removed - using data-driven approach with PieceFactory
#include "GameState.h"
#include "Square.h"
#include <numeric>

namespace BayouBonanza {

bool MoveExecutor::validateMove(const GameState& gameState, const Move& move) const {
    const GameBoard& board = gameState.getBoard();
    std::shared_ptr<Piece> piece = move.getPiece();
    
    // Ensure the piece exists
    if (!piece) {
        return false;
    }
    
    // Check if it's the correct player's turn
    if (piece->getSide() != gameState.getActivePlayer()) {
        return false;
    }

    // Cannot move if stunned
    if (piece->isStunned()) {
        return false;
    }
    
    // Check if the piece is at the expected starting position
    if (piece->getPosition() != move.getFrom()) {
        return false;
    }
    
    // Check if the move is valid for this piece type
    if (!piece->isValidMove(board, move.getTo())) {
        return false;
    }
    
    return true;
}

MoveResult MoveExecutor::executeMove(GameState& gameState, const Move& move) {
    // Validate the move first
    if (!validateMove(gameState, move)) {
        return MoveResult::INVALID_MOVE;
    }
    
    GameBoard& board = gameState.getBoard();
    std::shared_ptr<Piece> piece = move.getPiece();
    const Position& from = move.getFrom();
    const Position& to = move.getTo();
    
    // Get the target square
    Square& fromSquare = board.getSquare(from.x, from.y);
    Square& toSquare = board.getSquare(to.x, to.y);
    
    // If the target square has a piece, resolve combat
    if (!toSquare.isEmpty()) {
        Piece* targetPiece = toSquare.getPiece();
        
        // Check if the target piece belongs to the opponent
        if (targetPiece->getSide() != piece->getSide()) {
            // Create a shared_ptr wrapper for targetPiece temporarily for combat resolution
            // Note: This is a design issue - we should either use all shared_ptr or all raw pointers
            std::shared_ptr<Piece> targetSharedPtr(targetPiece, [](Piece*){});  // No-op deleter since ownership is managed by Square
            
            bool destroyed = resolveCombat(piece, targetSharedPtr, gameState);

            // Apply stun effects
            if (!destroyed) {
                targetPiece->applyStun(2);
            }
            if (piece->getCooldown() > 0) {
                piece->applyStun(piece->getCooldown());
            }

            // Special handling for ranged pieces like the Archer
            if (piece->isRanged()) {
                if (destroyed) {
                    // Check if the destroyed piece was a king BEFORE removing it
                    bool wasKing = targetPiece->isVictoryPiece();
                    PlayerSide targetSide = targetPiece->getSide();
                    
                    // Remove the defeated piece from the board
                    toSquare.setPiece(nullptr);

                    if (wasKing) {
                        if (targetSide == PlayerSide::PLAYER_ONE) {
                            gameState.setGameResult(GameResult::PLAYER_TWO_WIN);
                        } else {
                            gameState.setGameResult(GameResult::PLAYER_ONE_WIN);
                        }
                        recalculateBoardControl(gameState);
                        return MoveResult::KING_CAPTURED;
                    }
                }

                // Recalculate board control after ranged attack
                recalculateBoardControl(gameState);
                return MoveResult::SUCCESS;
            }

            if (!destroyed) {
                if (!piece->isRanged() && !piece->canJump()) {
                    int dx = to.x - from.x;
                    int dy = to.y - from.y;
                    int gcd_val = std::gcd(std::abs(dx), std::abs(dy));
                    if (gcd_val > 1) {
                        int step_x = dx / gcd_val;
                        int step_y = dy / gcd_val;
                        Position before = {to.x - step_x, to.y - step_y};
                        if (board.isValidPosition(before.x, before.y)) {
                            Square& beforeSquare = board.getSquare(before.x, before.y);
                            if (beforeSquare.isEmpty()) {
                                auto movingPiece = fromSquare.extractPiece();
                                if (movingPiece) {
                                    movingPiece->setPosition(before);
                                    movingPiece->setHasMoved(true);
                                    beforeSquare.setPiece(std::move(movingPiece));
                                    recalculateBoardControl(gameState);
                                    return MoveResult::SUCCESS;
                                }
                            }
                        }
                    }
                }
                return MoveResult::SUCCESS;
            }

            // Check if the destroyed piece was a victory piece BEFORE removing it
            bool wasVictoryPiece = targetPiece->isVictoryPiece();
            PlayerSide targetSide = targetPiece->getSide();
            
            // Remove the defeated piece from the board
            toSquare.setPiece(nullptr);

            // Only set game result if a victory piece was destroyed
            if (wasVictoryPiece) {
                // Set game result based on which victory piece was captured
                if (targetSide == PlayerSide::PLAYER_ONE) {
                    gameState.setGameResult(GameResult::PLAYER_TWO_WIN);
                } else {
                    gameState.setGameResult(GameResult::PLAYER_ONE_WIN);
                }

                // Extract the piece from the source square properly
                std::unique_ptr<Piece> movingPiece = fromSquare.extractPiece();
                if (!movingPiece) {
                    return MoveResult::INVALID_MOVE; // Piece was not at expected location
                }

                // Move the attacking piece to the target square
                movingPiece->setPosition(to);
                movingPiece->setHasMoved(true);
                toSquare.setPiece(std::move(movingPiece));

                recalculateBoardControl(gameState);
                return MoveResult::KING_CAPTURED;
            }
            
            // If a non-victory piece was destroyed, just move the attacking piece
            std::unique_ptr<Piece> movingPiece = fromSquare.extractPiece();
            if (!movingPiece) {
                return MoveResult::INVALID_MOVE; // Piece was not at expected location
            }

            // Move the attacking piece to the target square
            movingPiece->setPosition(to);
            movingPiece->setHasMoved(true);
            toSquare.setPiece(std::move(movingPiece));

            recalculateBoardControl(gameState);
            return MoveResult::PIECE_DESTROYED;
        } else {
            // Cannot move to a square occupied by own piece
            return MoveResult::INVALID_MOVE;
        }
    }
    
    // Normal move to empty square or after capture
    std::unique_ptr<Piece> movingPiece = fromSquare.extractPiece();
    if (!movingPiece) {
        return MoveResult::INVALID_MOVE; // Piece was not at expected location
    }
    
    // Move the piece to the target square
    movingPiece->setPosition(to);
    movingPiece->setHasMoved(true);
    toSquare.setPiece(std::move(movingPiece));
    
    // Recalculate board control after the move
    recalculateBoardControl(gameState);
    
    return MoveResult::SUCCESS;
}

std::vector<Move> MoveExecutor::getValidMoves(const GameState& gameState, std::shared_ptr<Piece> piece) const {
    std::vector<Move> validMoves;
    
    if (!piece) {
        return validMoves;
    }
    
    // Get the current position of the piece
    Position from = piece->getPosition();
    
    // Get all valid target positions for this piece
    std::vector<Position> targets = piece->getValidMoves(gameState.getBoard());
    
    // Create Move objects for each valid target
    for (const Position& to : targets) {
        validMoves.emplace_back(piece, from, to);
    }
    
    return validMoves;
}

bool MoveExecutor::resolveCombat(std::shared_ptr<Piece> attacker, std::shared_ptr<Piece> defender, GameState& gameState) {
    // Apply attacker's damage to defender
    bool destroyed = defender->takeDamage(attacker->getAttack());
    
    // Return whether the defender was destroyed
    return destroyed;
}

void MoveExecutor::recalculateBoardControl(GameState& gameState) {
    GameBoard& board = gameState.getBoard();
    
    // Use the new InfluenceSystem to calculate board influence and control
    InfluenceSystem::calculateBoardInfluence(board);
    
    // After recalculating control, update steam for each player
    int player1ControlledSquares = 0;
    int player2ControlledSquares = 0;
    
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Square& square = board.getSquare(x, y);
            PlayerSide controller = InfluenceSystem::getControllingPlayer(square);
            
            if (controller == PlayerSide::PLAYER_ONE) {
                player1ControlledSquares++;
            } else if (controller == PlayerSide::PLAYER_TWO) {
                player2ControlledSquares++;
            }
        }
    }
}

} // namespace BayouBonanza
