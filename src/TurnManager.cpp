#include "TurnManager.h"
#include "CardPlayValidator.h" // For PlayResult
#include <iostream>

namespace BayouBonanza {

TurnManager::TurnManager(GameState& gameState, GameRules& gameRules)
    : gameState(gameState), gameRules(gameRules) {
}

void TurnManager::startNewGame() {
    // Initialize the game with starting pieces and state
    gameRules.initializeGame(gameState);
}

void TurnManager::processMoveAction(const Move& move, ActionCallback callback) {
    ActionResult result;
    
    // Check if it's the correct player's turn
    if (move.getPiece() && move.getPiece()->getSide() != gameState.getActivePlayer()) {
        result.success = false;
        result.message = "It's not your turn";
    } else {
        // Process the move using game rules
        MoveResult moveResult = gameRules.processMove(gameState, move);
        
        // Convert MoveResult to ActionResult
        switch (moveResult) {
            case MoveResult::SUCCESS:
                result.success = true;
                result.message = "Move successful";
                // Switch to the next player after a successful move
                gameRules.endTurn(gameState);
                break;
                
            case MoveResult::PIECE_DESTROYED:
                result.success = true;
                result.message = "Enemy piece destroyed";
                // Switch to the next player after a successful move
                gameRules.endTurn(gameState);
                break;
                
            case MoveResult::KING_CAPTURED:
                result.success = true;
                result.message = "King captured! Game over.";
                // Don't switch players when the game is over
                break;
                
            case MoveResult::INVALID_MOVE:
                result.success = false;
                result.message = "Invalid move";
                break;
                
            case MoveResult::ERROR:
            default:
                result.success = false;
                result.message = "Error executing move";
                break;
        }
    }
    
    // Update game state based on action result
    updateGameState(result);
    
    // Call the callback if provided
    if (callback) {
        callback(result);
    }
}

void TurnManager::processPlayCardAction(int cardIndex, const Position& position, ActionCallback callback) {
    ActionResult result;
    
    // Check if it's the correct player's turn
    PlayerSide activePlayer = gameState.getActivePlayer();
    
    // Debug: Show hand information
    const Hand& hand = gameState.getHand(activePlayer);
    std::cout << "DEBUG: Player " << (activePlayer == PlayerSide::PLAYER_ONE ? "1" : "2") 
              << " hand size: " << hand.size() << ", trying to play card index: " << cardIndex << std::endl;
    
    for (size_t i = 0; i < hand.size(); ++i) {
        const Card* card = hand.getCard(i);
        if (card) {
            std::cout << "DEBUG: Hand[" << i << "]: " << card->getName() 
                      << " (cost: " << card->getSteamCost() << ")" << std::endl;
        } else {
            std::cout << "DEBUG: Hand[" << i << "]: NULL CARD" << std::endl;
        }
    }
    
    // Validate card index
    if (cardIndex < 0 || static_cast<size_t>(cardIndex) >= gameState.getHand(activePlayer).size()) {
        std::cout << "DEBUG: Card index " << cardIndex << " is invalid for hand size " << hand.size() << std::endl;
        result.success = false;
        result.message = "Invalid card index";
    } else {
        std::cout << "DEBUG: Card index " << cardIndex << " is valid, proceeding with card play" << std::endl;
        // Use GameState's playCardWithResult method which uses CardPlayValidator internally
        PlayResult playResult = gameState.playCardWithResult(activePlayer, static_cast<size_t>(cardIndex), position);
        
        // Convert PlayResult to ActionResult
        if (playResult.success) {
            result.success = true;
            result.message = "Card played successfully";
            
            // End the turn after a successful card play
            gameRules.endTurn(gameState);
        } else {
            result.success = false;
            result.message = playResult.errorMessage;
        }
    }
    
    // Update game state based on action result
    updateGameState(result);
    
    // Call the callback if provided
    if (callback) {
        callback(result);
    }
}

void TurnManager::endCurrentTurn(ActionCallback callback) {
    ActionResult result;
    
    // End the current turn
    gameRules.endTurn(gameState);
    
    result.success = true;
    result.message = "Turn ended";
    
    // Check if the game is over after ending the turn
    if (checkGameOver()) {
        result.message += ". Game over!";
    } else {
        result.message += ". It's now " + 
            std::string(gameState.getActivePlayer() == PlayerSide::PLAYER_ONE ? "Player 1" : "Player 2") + 
            "'s turn.";
    }
    
    // Call the callback if provided
    if (callback) {
        callback(result);
    }
}

bool TurnManager::isGameOver() const {
    return gameRules.isGameOver(gameState);
}

const GameState& TurnManager::getGameState() const {
    return gameState;
}

PlayerSide TurnManager::getActivePlayer() const {
    return gameState.getActivePlayer();
}

int TurnManager::getTurnNumber() const {
    return gameState.getTurnNumber();
}

bool TurnManager::updateGameState(const ActionResult& actionResult) {
    // Check if the game is over
    bool gameOver = checkGameOver();
    
    // If the game is over, no further state updates
    if (gameOver) {
        return false;
    }
    
    // Game continues
    return true;
}

bool TurnManager::checkGameOver() {
    return gameRules.isGameOver(gameState);
}

} // namespace BayouBonanza
