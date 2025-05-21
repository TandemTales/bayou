#include "../include/PieceRemovalHandler.h"
#include "../include/HealthTracker.h"
#include "../include/King.h"
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
    
    bool defeated = HealthTracker::isDefeated(piece);
    
    if (defeated) {
        // Fire the defeated event before removing the piece
        fireRemovalEvent(position, piece, RemovalEvent::PIECE_DEFEATED);
        
        // Check if the piece is a king (for win condition)
        if (dynamic_cast<King*>(piece.get())) {
            fireRemovalEvent(position, piece, RemovalEvent::KING_DEFEATED);
        }
        
        // Remove piece from board
        square.setPiece(nullptr);
        
        // Fire the removed event
        fireRemovalEvent(position, piece, RemovalEvent::PIECE_REMOVED);
        
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
    
    if (piece && dynamic_cast<King*>(piece.get())) {
        return HealthTracker::isDefeated(piece);
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
            
            if (piece && dynamic_cast<King*>(piece.get()) && HealthTracker::isDefeated(piece)) {
                kingDefeated = true;
                // The winner is the opposite side of the defeated king
                winningSide = (piece->getSide() == PlayerSide::PLAYER_ONE) ? 
                              PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
                break;
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
