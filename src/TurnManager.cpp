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
    } else if (!gameState.isActionAllowedInPhase(ActionType::MOVE_PIECE)) {
        result.success = false;
        result.message = "Piece movement is not allowed in the current phase";
    } else {
        // Process the move using game rules
        MoveResult moveResult = gameRules.processMove(gameState, move);
        
        // Convert MoveResult to ActionResult
        switch (moveResult) {
            case MoveResult::SUCCESS:
                result.success = true;
                result.message = "Move successful. Turn ended.";
                // Auto-end turn after successful move
                gameState.nextPhase();
                break;
                
            case MoveResult::PIECE_DESTROYED:
                result.success = true;
                result.message = "Enemy piece destroyed. Turn ended.";
                // Auto-end turn after successful move
                gameState.nextPhase();
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
    
    // Check if card play is allowed in the current phase
    if (!gameState.isActionAllowedInPhase(ActionType::PLAY_CARD)) {
        result.success = false;
        result.message = "Card play is not allowed in the current phase";
        
        // Call the callback if provided
        if (callback) {
            callback(result);
        }
        return;
    }
    
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
            result.message = "Card played successfully. Turn ended.";
            
            // Auto-end turn after successful card play
            gameState.nextPhase();
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
    
    // End the current turn by advancing to the next player's DRAW phase
    // Keep advancing phases until we reach the next player's turn
    PlayerSide originalPlayer = gameState.getActivePlayer();
    do {
        gameState.nextPhase();
    } while (gameState.getActivePlayer() == originalPlayer && 
             gameState.getGamePhase() != GamePhase::GAME_OVER);
    
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

void TurnManager::nextPhase(ActionCallback callback) {
    ActionResult result;
    
    // Check if phase advancement is allowed
    if (!gameState.isActionAllowedInPhase(ActionType::END_TURN)) {
        result.success = false;
        result.message = "Cannot advance phase in current game state";
    } else {
        GamePhase oldPhase = gameState.getGamePhase();
        PlayerSide oldPlayer = gameState.getActivePlayer();
        
        // Advance to the next phase
        gameState.nextPhase();
        
        GamePhase newPhase = gameState.getGamePhase();
        PlayerSide newPlayer = gameState.getActivePlayer();
        
        result.success = true;
        
        // Create appropriate message based on phase transition
        if (newPlayer != oldPlayer) {
            result.message = "Turn ended. It's now " + 
                std::string(newPlayer == PlayerSide::PLAYER_ONE ? "Player 1" : "Player 2") + 
                "'s turn (Draw Phase).";
        } else {
            std::string phaseStr;
            switch (newPhase) {
                case GamePhase::DRAW: phaseStr = "Draw"; break;
                case GamePhase::PLAY: phaseStr = "Play"; break;
                case GamePhase::MOVE: phaseStr = "Move"; break;
                case GamePhase::GAME_OVER: phaseStr = "Game Over"; break;
                default: phaseStr = "Unknown"; break;
            }
            result.message = "Advanced to " + phaseStr + " phase.";
        }
        
        // Check if the game is over
        if (checkGameOver()) {
            result.message += " Game over!";
        }
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
