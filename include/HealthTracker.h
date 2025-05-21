#ifndef BAYOU_BONANZA_HEALTH_TRACKER_H
#define BAYOU_BONANZA_HEALTH_TRACKER_H

#include "Piece.h"
#include "GameBoard.h"
#include <memory>
#include <functional>

namespace BayouBonanza {

// Health status categories for pieces
enum class HealthStatus {
    HEALTHY,    // Full health
    INJURED,    // Below 75% health
    CRITICAL,   // Below 25% health
    DEFEATED    // No health remaining
};

// Event types for health changes
enum class HealthEvent {
    DAMAGE_TAKEN,
    HEALTH_RESTORED,
    STATUS_CHANGED,
    DEFEATED
};

// Event callback type for health events
using HealthEventCallback = std::function<void(std::shared_ptr<Piece>, HealthEvent, int)>;

class HealthTracker {
public:
    // Register a callback function for health events
    static void registerEventCallback(HealthEventCallback callback);
    
    // Get current health status category of a piece
    static HealthStatus getHealthStatus(std::shared_ptr<Piece> piece);
    
    // Get health percentage (0-100) of a piece based on initial and current health
    static int getHealthPercentage(std::shared_ptr<Piece> piece);
    
    // Apply damage to a piece and trigger appropriate events
    static bool applyDamage(std::shared_ptr<Piece> piece, int damage);
    
    // Restore health to a piece and trigger appropriate events
    static void restoreHealth(std::shared_ptr<Piece> piece, int amount);
    
    // Check if piece is defeated (health <= 0)
    static bool isDefeated(std::shared_ptr<Piece> piece);
    
    // Check the entire board for defeated pieces
    static void checkBoardForDefeatedPieces(GameBoard& board, 
                                           std::function<void(const Position&)> onPieceDefeated);

private:
    static HealthEventCallback eventCallback;
    
    // Internal helper to fire events
    static void fireHealthEvent(std::shared_ptr<Piece> piece, HealthEvent event, int value);
};

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_HEALTH_TRACKER_H
