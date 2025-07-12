#include "CardPlayValidator.h"
#include "GameBoard.h"
#include "Square.h"
#include "GameOverDetector.h"
#include <algorithm>
#include <iostream>

namespace BayouBonanza {

ValidationResult CardPlayValidator::validateCardPlay(const GameState& gameState, PlayerSide player, size_t handIndex) {
    // Check game state validity
    auto gameStateResult = validateGameState(gameState, player);
    if (!gameStateResult.isValid) {
        return gameStateResult;
    }
    
    // Check hand index validity
    const Hand& hand = gameState.getHand(player);
    if (handIndex >= hand.size()) {
        return ValidationResult(false, ValidationError::INVALID_HAND_INDEX, 
                              "Hand index " + std::to_string(handIndex) + " is out of bounds");
    }
    
    // Get the card
    const Card* card = hand.getCard(handIndex);
    if (!card) {
        return ValidationResult(false, ValidationError::CARD_NOT_FOUND,
                              "No card found at hand index " + std::to_string(handIndex));
    }

    // During setup only victory piece cards may be played
    if (gameState.getGamePhase() == GamePhase::SETUP) {
        const PieceCard* pc = dynamic_cast<const PieceCard*>(card);
        if (!pc || !Square::globalPieceFactory ||
            !Square::globalPieceFactory->isVictoryPiece(pc->getPieceType())) {
            return ValidationResult(false, ValidationError::GAME_STATE_INVALID,
                                  "Only victory piece cards can be played during setup");
        }
        GameOverDetector detector;
        if (detector.hasVictoryPieces(gameState, player)) {
            return ValidationResult(false, ValidationError::GAME_STATE_INVALID,
                                  "Victory piece already in play for this player");
        }
    }
    
    // Check steam cost
    if (gameState.getSteam(player) < card->getSteamCost()) {
        return ValidationResult(false, ValidationError::INSUFFICIENT_STEAM,
                              "Insufficient steam: need " + std::to_string(card->getSteamCost()) + 
                              ", have " + std::to_string(gameState.getSteam(player)));
    }
    
    // Check if card can be played using its own validation
    if (!card->canPlay(gameState, player)) {
        return ValidationResult(false, ValidationError::CARD_CANNOT_BE_PLAYED,
                              "Card '" + card->getName() + "' cannot be played in current game state");
    }
    
    return ValidationResult(true, ValidationError::NONE, "Card can be played");
}

ValidationResult CardPlayValidator::validateTargetedCardPlay(const GameState& gameState, PlayerSide player, 
                                                           size_t handIndex, const Position& targetPosition) {
    // First validate basic card play
    auto basicResult = validateCardPlay(gameState, player, handIndex);
    if (!basicResult.isValid) {
        return basicResult;
    }
    
    // Check if target position is within bounds
    if (!isValidBoardPosition(targetPosition)) {
        return ValidationResult(false, ValidationError::INVALID_TARGET,
                              "Target position (" + std::to_string(targetPosition.x) + ", " +
                              std::to_string(targetPosition.y) + ") is out of bounds");
    }

    if (gameState.getGamePhase() == GamePhase::SETUP) {
        const GameBoard& board = gameState.getBoard();
        const Square& sq = board.getSquare(targetPosition.x, targetPosition.y);
        if (sq.getControlledBy() != player || !sq.isEmpty()) {
            return ValidationResult(false, ValidationError::INVALID_PLACEMENT,
                                  "Square not controlled by player or occupied");
        }
    }
    
    // Get the card and validate specific targeting
    const Hand& hand = gameState.getHand(player);
    const Card* card = hand.getCard(handIndex);
    

    
    switch (card->getCardType()) {
        case CardType::PIECE_CARD: {
            const PieceCard* pieceCard = dynamic_cast<const PieceCard*>(card);
            if (!pieceCard) {
                return ValidationResult(false, ValidationError::UNKNOWN_CARD_TYPE,
                                      "Failed to cast to PieceCard");
            }
            return validatePiecePlacement(gameState, player, pieceCard, targetPosition);
        }
        case CardType::EFFECT_CARD: {
            const EffectCard* effectCard = dynamic_cast<const EffectCard*>(card);
            if (!effectCard) {
                return ValidationResult(false, ValidationError::UNKNOWN_CARD_TYPE,
                                      "Failed to cast to EffectCard");
            }
            return validateEffectTarget(gameState, player, effectCard, targetPosition);
        }
        default:
            return ValidationResult(false, ValidationError::UNKNOWN_CARD_TYPE,
                                  "Unknown card type for targeted play");
    }
}

ValidationResult CardPlayValidator::validatePiecePlacement(const GameState& gameState, PlayerSide player,
                                                          const PieceCard* pieceCard, const Position& position) {
    if (!pieceCard) {
        return ValidationResult(false, ValidationError::CARD_NOT_FOUND, "PieceCard is null");
    }
    
    // During setup, placement validity is checked separately
    if (gameState.getGamePhase() != GamePhase::SETUP) {
        if (!pieceCard->isValidPlacement(gameState, player, position)) {
            return ValidationResult(false, ValidationError::INVALID_PLACEMENT,
                                  "Position (" + std::to_string(position.x) + ", " +
                                  std::to_string(position.y) + ") is not valid for piece placement");
        }
    }
    
    return ValidationResult(true, ValidationError::NONE, "Piece placement is valid");
}

ValidationResult CardPlayValidator::validateEffectTarget(const GameState& gameState, PlayerSide player,
                                                        const EffectCard* effectCard, const Position& position) {
    if (!effectCard) {
        return ValidationResult(false, ValidationError::CARD_NOT_FOUND, "EffectCard is null");
    }
    
    // Use the effect card's own validation
    if (!effectCard->isValidTarget(gameState, player, position)) {
        return ValidationResult(false, ValidationError::INVALID_TARGET,
                              "Position (" + std::to_string(position.x) + ", " + 
                              std::to_string(position.y) + ") is not a valid target for this effect");
    }
    
    return ValidationResult(true, ValidationError::NONE, "Effect target is valid");
}

std::vector<Position> CardPlayValidator::getValidPlacements(const GameState& gameState, PlayerSide player,
                                                           const PieceCard* pieceCard) {
    if (!pieceCard) {
        return {};
    }
    
    return pieceCard->getValidPlacements(gameState, player);
}

std::vector<Position> CardPlayValidator::getValidTargets(const GameState& gameState, PlayerSide player,
                                                        const EffectCard* effectCard) {
    if (!effectCard) {
        return {};
    }
    
    return effectCard->getValidTargets(gameState, player);
}

PlayResult CardPlayValidator::executeCardPlay(GameState& gameState, PlayerSide player, size_t handIndex,
                                             const Position& targetPosition) {
    // If a target position is specified, use targeted play
    if (targetPosition.x != -1 && targetPosition.y != -1) {
        return executeTargetedCardPlay(gameState, player, handIndex, targetPosition);
    }
    
    // Validate the card play
    auto validation = validateCardPlay(gameState, player, handIndex);
    if (!validation.isValid) {
        return PlayResult(false, validation.error, validation.errorMessage);
    }
    
    // Get the card and its cost
    Hand& hand = gameState.getHand(player);
    const Card* card = hand.getCard(handIndex);
    int steamCost = card->getSteamCost();
    
    // Remove the card from hand first
    auto cardToPlay = hand.removeCardAt(handIndex);
    if (!cardToPlay) {
        return PlayResult(false, ValidationError::CARD_NOT_FOUND, 
                         "Failed to remove card from hand", false, false);
    }
    
    // Deduct steam cost
    if (!gameState.spendSteam(player, steamCost)) {
        // Rollback: return card to hand
        hand.addCard(std::move(cardToPlay));
        return PlayResult(false, ValidationError::INSUFFICIENT_STEAM,
                         "Failed to spend steam", false, false);
    }
    
    // Execute the card's effect
    bool playSuccess = cardToPlay->play(gameState, player);
    
    if (!playSuccess) {
        // Rollback: refund steam and return card to hand
        gameState.addSteam(player, steamCost);
        hand.addCard(std::move(cardToPlay));
        return PlayResult(false, ValidationError::CARD_CANNOT_BE_PLAYED,
                         "Card play execution failed", false, false);
    }
    
    // Success - card was played and removed from hand
    return PlayResult(true, ValidationError::NONE, "Card played successfully", true, true);
}

PlayResult CardPlayValidator::executeTargetedCardPlay(GameState& gameState, PlayerSide player, 
                                                     size_t handIndex, const Position& targetPosition) {
    // Validate the targeted card play
    auto validation = validateTargetedCardPlay(gameState, player, handIndex, targetPosition);
    if (!validation.isValid) {
        return PlayResult(false, validation.error, validation.errorMessage);
    }
    
    // Get the card and its cost
    Hand& hand = gameState.getHand(player);
    const Card* card = hand.getCard(handIndex);
    int steamCost = card->getSteamCost();
    
    // Remove the card from hand first
    auto cardToPlay = hand.removeCardAt(handIndex);
    if (!cardToPlay) {
        return PlayResult(false, ValidationError::CARD_NOT_FOUND, 
                         "Failed to remove card from hand", false, false);
    }
    
    // Deduct steam cost
    if (!gameState.spendSteam(player, steamCost)) {
        // Rollback: return card to hand
        hand.addCard(std::move(cardToPlay));
        return PlayResult(false, ValidationError::INSUFFICIENT_STEAM,
                         "Failed to spend steam", false, false);
    }
    
    // Execute the card's targeted effect
    bool playSuccess = false;
    
    switch (cardToPlay->getCardType()) {
        case CardType::PIECE_CARD: {
            PieceCard* pieceCard = dynamic_cast<PieceCard*>(cardToPlay.get());
            if (pieceCard) {
                playSuccess = pieceCard->playAtPosition(gameState, player, targetPosition);
            }
            break;
        }
        case CardType::EFFECT_CARD: {
            EffectCard* effectCard = dynamic_cast<EffectCard*>(cardToPlay.get());
            if (effectCard) {
                playSuccess = effectCard->playAtTarget(gameState, player, targetPosition);
            }
            break;
        }
        default:
            // For other card types, use basic play method
            playSuccess = cardToPlay->play(gameState, player);
            break;
    }
    
    if (!playSuccess) {
        // Rollback: refund steam and return card to hand
        gameState.addSteam(player, steamCost);
        hand.addCard(std::move(cardToPlay));
        return PlayResult(false, ValidationError::CARD_CANNOT_BE_PLAYED,
                         "Targeted card play execution failed", false, false);
    }
    
    // Success - card was played and removed from hand
    return PlayResult(true, ValidationError::NONE, "Targeted card played successfully", true, true);
}

ValidationResult CardPlayValidator::validateGameState(const GameState& gameState, PlayerSide player) {
    // Check if game is over
    if (gameState.getGameResult() != GameResult::IN_PROGRESS) {
        return ValidationResult(false, ValidationError::GAME_STATE_INVALID,
                              "Game is over, cannot play cards");
    }
    
    // Check if it's the player's turn
    if (gameState.getActivePlayer() != player) {
        return ValidationResult(false, ValidationError::GAME_STATE_INVALID,
                              "It is not this player's turn");
    }
    
    // Check if game phase allows card play
    GamePhase phase = gameState.getGamePhase();
    if (phase != GamePhase::PLAY) {
        return ValidationResult(false, ValidationError::GAME_STATE_INVALID,
                              "Current game phase does not allow card play");
    }
    
    return ValidationResult(true, ValidationError::NONE, "Game state allows card play");
}

std::string CardPlayValidator::getErrorMessage(ValidationError error) {
    switch (error) {
        case ValidationError::NONE:
            return "No error";
        case ValidationError::INSUFFICIENT_STEAM:
            return "Insufficient steam to play this card";
        case ValidationError::INVALID_HAND_INDEX:
            return "Invalid hand index specified";
        case ValidationError::CARD_NOT_FOUND:
            return "Card not found at specified index";
        case ValidationError::NO_VALID_TARGETS:
            return "No valid targets available for this card";
        case ValidationError::INVALID_TARGET:
            return "Invalid target position specified";
        case ValidationError::INVALID_PLACEMENT:
            return "Invalid placement position for piece";
        case ValidationError::GAME_STATE_INVALID:
            return "Game state does not allow card play";
        case ValidationError::CARD_CANNOT_BE_PLAYED:
            return "Card cannot be played in current situation";
        case ValidationError::UNKNOWN_CARD_TYPE:
            return "Unknown or unsupported card type";
        default:
            return "Unknown validation error";
    }
}

bool CardPlayValidator::isValidBoardPosition(const Position& position) {
    return position.x >= 0 && position.x < 8 && position.y >= 0 && position.y < 8;
}

void CardPlayValidator::rollbackCardPlay(GameState& gameState, PlayerSide player, 
                                        std::unique_ptr<Card> card, int steamCost, size_t handIndex) {
    // Refund steam
    gameState.addSteam(player, steamCost);
    
    // Return card to hand
    Hand& hand = gameState.getHand(player);
    hand.addCard(std::move(card));
    
    // Note: We can't restore the exact hand index since the hand structure may have changed
    // The card will be added to the end of the hand
}

} // namespace BayouBonanza 