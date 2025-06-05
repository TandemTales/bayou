#include "EffectCard.h"
#include "GameState.h"
#include "GameBoard.h"
#include "Square.h"
#include <algorithm>

namespace BayouBonanza {

EffectCard::EffectCard(int id, const std::string& name, const std::string& description,
                       int steamCost, const Effect& effect, CardRarity rarity)
    : Card(id, name, description, steamCost, CardType::EFFECT_CARD, rarity), 
      effect(effect) {
}

const Effect& EffectCard::getEffect() const {
    return effect;
}

bool EffectCard::canPlay(const GameState& gameState, PlayerSide player) const {
    // Check if player has enough steam
    if (gameState.getSteam(player) < steamCost) {
        return false;
    }
    
    // Check if there are valid targets based on the effect type
    switch (effect.targetType) {
        case TargetType::SINGLE_PIECE:
        case TargetType::BOARD_AREA: {
            auto validTargets = getValidTargets(gameState, player);
            return !validTargets.empty();
        }
        case TargetType::ALL_FRIENDLY:
        case TargetType::ALL_ENEMY:
        case TargetType::ALL_PIECES: {
            // These effects can always be played (they might just have no effect)
            return true;
        }
        case TargetType::SELF_PLAYER:
        case TargetType::ENEMY_PLAYER: {
            // Player-targeted effects can always be played
            return true;
        }
        default:
            return false;
    }
}

bool EffectCard::play(GameState& gameState, PlayerSide player) const {
    // Check if we can play the card
    if (!canPlay(gameState, player)) {
        return false;
    }
    
    // Note: Steam cost is handled by the caller (CardPlayValidator::executeCardPlay)
    // Don't spend steam here to avoid double-spending
    
    bool effectApplied = false;
    
    // Apply effect based on target type
    switch (effect.targetType) {
        case TargetType::SINGLE_PIECE: {
            // For single piece effects, target the first valid piece
            auto validTargets = getValidTargets(gameState, player);
            if (!validTargets.empty()) {
                effectApplied = playAtTarget(gameState, player, validTargets[0]);
            }
            break;
        }
        case TargetType::ALL_FRIENDLY: {
            GameBoard& board = gameState.getBoard();
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    Square& square = board.getSquare(x, y);
                    if (!square.isEmpty()) {
                        Piece* piece = square.getPiece();
                        if (piece->getSide() == player) {
                            if (applyEffectToPiece(piece, player)) {
                                effectApplied = true;
                            }
                        }
                    }
                }
            }
            break;
        }
        case TargetType::ALL_ENEMY: {
            GameBoard& board = gameState.getBoard();
            PlayerSide enemySide = (player == PlayerSide::PLAYER_ONE) ? PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    Square& square = board.getSquare(x, y);
                    if (!square.isEmpty()) {
                        Piece* piece = square.getPiece();
                        if (piece->getSide() == enemySide) {
                            if (applyEffectToPiece(piece, player)) {
                                effectApplied = true;
                            }
                        }
                    }
                }
            }
            break;
        }
        case TargetType::ALL_PIECES: {
            GameBoard& board = gameState.getBoard();
            for (int x = 0; x < 8; x++) {
                for (int y = 0; y < 8; y++) {
                    Square& square = board.getSquare(x, y);
                    if (!square.isEmpty()) {
                        Piece* piece = square.getPiece();
                        if (applyEffectToPiece(piece, player)) {
                            effectApplied = true;
                        }
                    }
                }
            }
            break;
        }
        case TargetType::SELF_PLAYER: {
            effectApplied = applyEffectToPlayer(gameState, player, player);
            break;
        }
        case TargetType::ENEMY_PLAYER: {
            PlayerSide enemySide = (player == PlayerSide::PLAYER_ONE) ? PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
            effectApplied = applyEffectToPlayer(gameState, enemySide, player);
            break;
        }
        default:
            break;
    }
    
    // If no effect was applied, return false
    // Note: Steam refund is handled by the caller if needed
    if (!effectApplied) {
        return false;
    }
    
    return true;
}

bool EffectCard::isValidTarget(const GameState& gameState, PlayerSide player, const Position& position) const {
    const GameBoard& board = gameState.getBoard();
    
    // Check if position is within bounds
    if (position.x < 0 || position.x >= 8 || position.y < 0 || position.y >= 8) {
        return false;
    }
    
    const Square& square = board.getSquare(position.x, position.y);
    
    // For piece-targeting effects, check if there's a piece and if it's a valid target
    if (effect.targetType == TargetType::SINGLE_PIECE || effect.targetType == TargetType::BOARD_AREA) {
        if (square.isEmpty()) {
            return false;
        }
        
        Piece* piece = square.getPiece();
        PlayerSide pieceSide = piece->getSide();
        
        // Check if we can target this piece based on the effect type
        if (canTargetFriendly() && pieceSide == player) {
            return true;
        }
        if (canTargetEnemy() && pieceSide != player) {
            return true;
        }
    }
    
    return false;
}

std::vector<Position> EffectCard::getValidTargets(const GameState& gameState, PlayerSide player) const {
    std::vector<Position> validTargets;
    
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Position pos{x, y};
            if (isValidTarget(gameState, player, pos)) {
                validTargets.push_back(pos);
            }
        }
    }
    
    return validTargets;
}

bool EffectCard::playAtTarget(GameState& gameState, PlayerSide player, const Position& position) const {
    if (!isValidTarget(gameState, player, position)) {
        return false;
    }
    
    GameBoard& board = gameState.getBoard();
    Square& square = board.getSquare(position.x, position.y);
    Piece* piece = square.getPiece();
    
    return applyEffectToPiece(piece, player);
}

bool EffectCard::applyEffectToPiece(Piece* piece, PlayerSide player) const {
    if (!piece) {
        return false;
    }
    
    switch (effect.type) {
        case EffectType::HEAL: {
            int currentHealth = piece->getHealth();
            int maxHealth = piece->getMaxHealth();
            int newHealth = std::min(currentHealth + effect.magnitude, maxHealth);
            piece->setHealth(newHealth);
            return newHealth > currentHealth;
        }
        case EffectType::DAMAGE: {
            return piece->takeDamage(effect.magnitude);
        }
        case EffectType::BUFF_ATTACK: {
            // Note: Current Piece class doesn't support temporary stat modifications
            // This would need to be implemented with a status effect system
            // For now, we'll just return true to indicate the effect was "applied"
            return true;
        }
        case EffectType::BUFF_HEALTH: {
            int currentHealth = piece->getHealth();
            piece->setHealth(currentHealth + effect.magnitude);
            return true;
        }
        case EffectType::DEBUFF_ATTACK:
        case EffectType::DEBUFF_HEALTH:
        case EffectType::MOVE_BOOST:
        case EffectType::SHIELD:
        case EffectType::POISON:
        case EffectType::STUN: {
            // These effects would require a status effect system to be properly implemented
            // For now, we'll just return true to indicate the effect was "applied"
            return true;
        }
        default:
            return false;
    }
}

bool EffectCard::applyEffectToPlayer(GameState& gameState, PlayerSide targetPlayer, PlayerSide castingPlayer) const {
    // Apply player-targeted effects (like steam manipulation)
    switch (effect.type) {
        case EffectType::HEAL: {
            // Could be used for steam generation
            gameState.addSteam(targetPlayer, effect.magnitude);
            return true;
        }
        case EffectType::DAMAGE: {
            // Could be used for steam drain
            int currentSteam = gameState.getSteam(targetPlayer);
            int newSteam = std::max(0, currentSteam - effect.magnitude);
            gameState.setSteam(targetPlayer, newSteam);
            return currentSteam > newSteam;
        }
        default:
            return false;
    }
}

std::string EffectCard::getDetailedDescription() const {
    std::string detail = Card::getDetailedDescription();
    
    detail += "\nEffect: " + getEffectTypeName();
    detail += "\nMagnitude: " + std::to_string(effect.magnitude);
    
    if (effect.duration == 0) {
        detail += "\nDuration: Instant";
    } else if (effect.duration == -1) {
        detail += "\nDuration: Permanent";
    } else {
        detail += "\nDuration: " + std::to_string(effect.duration) + " turns";
    }
    
    // Add target type information
    std::string targetStr;
    switch (effect.targetType) {
        case TargetType::SINGLE_PIECE: targetStr = "Single Piece"; break;
        case TargetType::ALL_FRIENDLY: targetStr = "All Friendly Pieces"; break;
        case TargetType::ALL_ENEMY: targetStr = "All Enemy Pieces"; break;
        case TargetType::ALL_PIECES: targetStr = "All Pieces"; break;
        case TargetType::BOARD_AREA: targetStr = "Board Area"; break;
        case TargetType::SELF_PLAYER: targetStr = "Self"; break;
        case TargetType::ENEMY_PLAYER: targetStr = "Enemy Player"; break;
    }
    detail += "\nTarget: " + targetStr;
    
    return detail;
}

std::unique_ptr<Card> EffectCard::clone() const {
    return std::make_unique<EffectCard>(id, name, description, steamCost, effect, rarity);
}

bool EffectCard::canTargetFriendly() const {
    // Beneficial effects can target friendly pieces
    switch (effect.type) {
        case EffectType::HEAL:
        case EffectType::BUFF_ATTACK:
        case EffectType::BUFF_HEALTH:
        case EffectType::MOVE_BOOST:
        case EffectType::SHIELD:
            return true;
        default:
            return false;
    }
}

bool EffectCard::canTargetEnemy() const {
    // Harmful effects can target enemy pieces
    switch (effect.type) {
        case EffectType::DAMAGE:
        case EffectType::DEBUFF_ATTACK:
        case EffectType::DEBUFF_HEALTH:
        case EffectType::POISON:
        case EffectType::STUN:
            return true;
        default:
            return false;
    }
}

std::string EffectCard::getEffectTypeName() const {
    switch (effect.type) {
        case EffectType::HEAL: return "Heal";
        case EffectType::DAMAGE: return "Damage";
        case EffectType::BUFF_ATTACK: return "Attack Buff";
        case EffectType::BUFF_HEALTH: return "Health Buff";
        case EffectType::DEBUFF_ATTACK: return "Attack Debuff";
        case EffectType::DEBUFF_HEALTH: return "Health Debuff";
        case EffectType::MOVE_BOOST: return "Movement Boost";
        case EffectType::SHIELD: return "Shield";
        case EffectType::POISON: return "Poison";
        case EffectType::STUN: return "Stun";
        default: return "Unknown";
    }
}

} // namespace BayouBonanza 