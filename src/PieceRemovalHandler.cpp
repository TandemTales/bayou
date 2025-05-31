#include "../include/PieceRemovalHandler.h"
#include "../include/HealthTracker.h"
// #include "../include/King.h" // Removed - using data-driven approach with PieceFactory
#include "../include/GameBoard.h"

namespace BayouBonanza {

// Initialize static members
RemovalEventCallback PieceRemovalHandler::eventCallback = nullptr;

void PieceRemovalHandler::registerEventCallback(RemovalEventCallback callback) {
    eventCallback = callback;
}

bool PieceRemovalHandler::removePiece(GameBoard& board, const Position& position) {
    if (!board.isValidPosition(position.x, position.y)) {
        return false;
    }
    
    auto& square = board.getSquare(position.x, position.y);
    auto piece = square.getPiece();
    
    if (!piece) {
        return false; // No piece to remove
    }
    
    // Create a temporary shared_ptr wrapper for HealthTracker compatibility
    std::shared_ptr<Piece> piecePtr(piece, [](Piece*){});  // No-op deleter
    bool defeated = HealthTracker::isDefeated(piecePtr);
    
    if (defeated) {
        // Fire the defeated event before removing the piece
        fireRemovalEvent(position, piecePtr, RemovalEvent::PIECE_DEFEATED);
        
        // Check if the piece is a king (for win condition)
        if (piece->getPieceType() == PieceType::KING) {
            fireRemovalEvent(position, piecePtr, RemovalEvent::KING_DEFEATED);
        }
        
        // Remove piece from board
        square.setPiece(nullptr);
        
        // Fire the removed event
        fireRemovalEvent(position, piecePtr, RemovalEvent::PIECE_REMOVED);
        
        // Recalculate control values if necessary
        board.recalculateControlValues();
        
        return true;
    }
    
    return false;
}

std::vector<Position> PieceRemovalHandler::removeDefeatedPieces(GameBoard& board) {
    std::vector<Position> removedPositions;
    
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Position pos(x, y);
            if (removePiece(board, pos)) {
                removedPositions.push_back(pos);
            }
        }
    }
    
    return removedPositions;
}

bool PieceRemovalHandler::isKingDefeated(const GameBoard& board, const Position& position) {
    if (!board.isValidPosition(position.x, position.y)) {
        return false;
    }
    
    auto& square = board.getSquare(position.x, position.y);
    auto piece = square.getPiece();
    
    if (piece && piece->getPieceType() == PieceType::KING) {
        // Create a temporary shared_ptr wrapper for HealthTracker compatibility
        std::shared_ptr<Piece> piecePtr(piece, [](Piece*){});  // No-op deleter
        return HealthTracker::isDefeated(piecePtr);
    }
    
    return false;
}

bool PieceRemovalHandler::checkForDefeatedKings(const GameBoard& board, PlayerSide& winningSide) {
    bool kingDefeated = false;
    
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Position pos(x, y);
            auto& square = board.getSquare(x, y);
            auto piece = square.getPiece();
            
            if (piece && piece->getPieceType() == PieceType::KING) {
                // Create a temporary shared_ptr wrapper for HealthTracker compatibility
                std::shared_ptr<Piece> piecePtr(piece, [](Piece*){});  // No-op deleter
                if (HealthTracker::isDefeated(piecePtr)) {
                    kingDefeated = true;
                    // The winner is the opposite side of the defeated king
                    winningSide = (piece->getSide() == PlayerSide::PLAYER_ONE) ? 
                                  PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
                    break;
                }
            }
        }
        if (kingDefeated) break;
    }
    
    return kingDefeated;
}

void PieceRemovalHandler::fireRemovalEvent(const Position& position, std::shared_ptr<Piece> piece, RemovalEvent event) {
    if (eventCallback) {
        eventCallback(position, piece, event);
    }
}

} // namespace BayouBonanza
