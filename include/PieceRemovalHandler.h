#ifndef BAYOU_BONANZA_PIECE_REMOVAL_HANDLER_H
#define BAYOU_BONANZA_PIECE_REMOVAL_HANDLER_H

#include "GameBoard.h"
#include "Piece.h"
#include <memory>
#include <vector>
#include <functional>

namespace BayouBonanza {

// Types of events that can occur during piece removal
enum class RemovalEvent {
    PIECE_DEFEATED,    // Piece has been defeated (health <= 0)
    PIECE_REMOVED,     // Piece has been removed from the board
    KING_DEFEATED      // A king has been defeated (special case for win condition)
};

// Callback type for removal events
using RemovalEventCallback = std::function<void(const Position&, std::shared_ptr<Piece>, RemovalEvent)>;

class PieceRemovalHandler {
public:
    // Register a callback for removal events
    static void registerEventCallback(RemovalEventCallback callback);
    
    // Remove a defeated piece from the board
    static bool removePiece(GameBoard& board, const Position& position);
    
    // Remove all defeated pieces from the board
    static std::vector<Position> removeDefeatedPieces(GameBoard& board);
    
    // Check if a piece at a position is a king and defeated
    static bool isKingDefeated(const GameBoard& board, const Position& position);
    
    // Check the entire board for defeated kings
    static bool checkForDefeatedKings(const GameBoard& board, PlayerSide& winningSide);

private:
    static RemovalEventCallback eventCallback;
    
    // Fire a removal event to any registered callbacks
    static void fireRemovalEvent(const Position& position, std::shared_ptr<Piece> piece, RemovalEvent event);
};

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_PIECE_REMOVAL_HANDLER_H
