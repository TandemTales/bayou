#include "../include/HealthTracker.h"
#include "../include/GameBoard.h"

namespace BayouBonanza {

// Initialize static member
HealthEventCallback HealthTracker::eventCallback = nullptr;

void HealthTracker::registerEventCallback(HealthEventCallback callback) {
    eventCallback = callback;
}

HealthStatus HealthTracker::getHealthStatus(std::shared_ptr<Piece> piece) {
    if (!piece) {
        return HealthStatus::DEFEATED;
    }
    
    int percentage = getHealthPercentage(piece);
    
    if (percentage <= 0) {
        return HealthStatus::DEFEATED;
    } else if (percentage < 25) {
        return HealthStatus::CRITICAL;
    } else if (percentage < 75) {
        return HealthStatus::INJURED;
    } else {
        return HealthStatus::HEALTHY;
    }
}

int HealthTracker::getHealthPercentage(std::shared_ptr<Piece> piece) {
    if (!piece) {
        return 0;
    }
    
    // We would need initial max health here
    // Since we don't have it stored currently, we'll approximate based on piece type
    // A better implementation would store max health in the Piece class
    int currentHealth = piece->getHealth();
    int maxHealth = 10; // Default max health
    
    // Scale to percentage
    return (currentHealth * 100) / maxHealth;
}

bool HealthTracker::applyDamage(std::shared_ptr<Piece> piece, int damage) {
    if (!piece || damage <= 0) {
        return false;
    }
    
    HealthStatus beforeStatus = getHealthStatus(piece);
    
    // Apply damage
    int healthBefore = piece->getHealth();
    bool defeated = piece->takeDamage(damage);
    int healthAfter = piece->getHealth();
    
    // Fire damage taken event
    fireHealthEvent(piece, HealthEvent::DAMAGE_TAKEN, damage);
    
    // Check for status change
    HealthStatus afterStatus = getHealthStatus(piece);
    if (beforeStatus != afterStatus) {
        fireHealthEvent(piece, HealthEvent::STATUS_CHANGED, static_cast<int>(afterStatus));
    }
    
    // Check for defeat
    if (defeated) {
        fireHealthEvent(piece, HealthEvent::DEFEATED, 0);
    }
    
    return defeated;
}

void HealthTracker::restoreHealth(std::shared_ptr<Piece> piece, int amount) {
    if (!piece || amount <= 0) {
        return;
    }
    
    HealthStatus beforeStatus = getHealthStatus(piece);
    
    // Apply healing
    int healthBefore = piece->getHealth();
    piece->setHealth(healthBefore + amount);
    
    // Fire health restored event
    fireHealthEvent(piece, HealthEvent::HEALTH_RESTORED, amount);
    
    // Check for status change
    HealthStatus afterStatus = getHealthStatus(piece);
    if (beforeStatus != afterStatus) {
        fireHealthEvent(piece, HealthEvent::STATUS_CHANGED, static_cast<int>(afterStatus));
    }
}

bool HealthTracker::isDefeated(std::shared_ptr<Piece> piece) {
    return !piece || piece->getHealth() <= 0;
}

void HealthTracker::checkBoardForDefeatedPieces(GameBoard& board, 
                                              std::function<void(const Position&)> onPieceDefeated) {
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            Position pos(x, y);
            auto& square = board.getSquare(x, y);
            
            if (!square.isEmpty()) {
                Piece* piece = square.getPiece();
                if (piece && piece->getHealth() <= 0) {
                    if (onPieceDefeated) {
                        onPieceDefeated(pos);
                    }
                }
            }
        }
    }
}

void HealthTracker::fireHealthEvent(std::shared_ptr<Piece> piece, HealthEvent event, int value) {
    if (eventCallback) {
        eventCallback(piece, event, value);
    }
}

} // namespace BayouBonanza
