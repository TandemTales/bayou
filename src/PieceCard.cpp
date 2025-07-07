#include "PieceCard.h"
#include "GameState.h"
#include "GameBoard.h"
#include "Square.h"
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"
#include "InfluenceSystem.h"
#include <algorithm>
#include <iostream>

namespace BayouBonanza {

PieceCard::PieceCard(int id, const std::string& name, const std::string& description,
                     int steamCost, const std::string& pieceType, CardRarity rarity)
    : Card(id, name, description, steamCost, CardType::PIECE_CARD, rarity),
      pieceType(pieceType) {
}

const std::string& PieceCard::getPieceType() const {
    return pieceType;
}

bool PieceCard::canPlay(const GameState& gameState, PlayerSide player) const {
    // Check if player has enough steam
    if (gameState.getSteam(player) < steamCost) {
        return false;
    }
    
    // Check if there are any valid placement positions
    auto validPositions = getValidPlacements(gameState, player);
    return !validPositions.empty();
}

bool PieceCard::play(GameState& gameState, PlayerSide player) const {
    // Get valid placements
    auto validPositions = getValidPlacements(gameState, player);
    if (validPositions.empty()) {
        return false;
    }
    
    // For automatic placement, choose the first valid position
    // In a real game, this would be chosen by the player
    return playAtPosition(gameState, player, validPositions[0]);
}

bool PieceCard::isValidPlacement(const GameState& gameState, PlayerSide player, const Position& position) const {
    const GameBoard& board = gameState.getBoard();
    

    
    // Check if position is within board bounds
    if (position.x < 0 || position.x >= 8 || position.y < 0 || position.y >= 8) {
        return false;
    }
    
    // Check if the square is empty
    if (!board.getSquare(position.x, position.y).isEmpty()) {
        return false;
    }
    
    // Use territorial rules for card placement:
    // Player One can place pieces on their half (rows 4-7)
    // Player Two can place pieces on their half (rows 0-3)
    bool validTerritory = false;
    if (player == PlayerSide::PLAYER_ONE) {
        validTerritory = (position.y >= 4 && position.y <= 7);
    } else if (player == PlayerSide::PLAYER_TWO) {
        validTerritory = (position.y >= 0 && position.y <= 3);
    }
    

    
    return validTerritory;
}

std::vector<Position> PieceCard::getValidPlacements(const GameState& gameState, PlayerSide player) const {
    std::vector<Position> validPositions;
    
    // Check all positions on the board
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Position pos{x, y};
            if (isValidPlacement(gameState, player, pos)) {
                validPositions.push_back(pos);
            }
        }
    }
    
    return validPositions;
}

bool PieceCard::playAtPosition(GameState& gameState, PlayerSide player, const Position& position) const {
    // Validate the placement
    if (!isValidPlacement(gameState, player, position)) {
        return false;
    }
    
    // Note: Steam cost is handled by the caller (CardPlayValidator::executeCardPlay)
    // Don't spend steam here to avoid double-spending
    

    
    // Use the global piece factory to create the piece with proper stats and movement rules
    if (!Square::globalPieceFactory) {
        return false;
    }
    
    try {
        // Create the piece using the proper factory
        auto piece = Square::globalPieceFactory->createPiece(pieceType, player);
        
        if (!piece) {
            return false;
        }
        
        // Set the piece's position
        piece->setPosition(position);
        
        // Get the square and place the piece
        GameBoard& board = gameState.getBoard();
        Square& square = board.getSquare(position.x, position.y);
        square.setPiece(std::move(piece));
        
        return true;
    } catch (...) {
        // If piece creation fails, return false
        // Note: Steam refund is handled by the caller if needed
        return false;
    }
}

std::string PieceCard::getDetailedDescription() const {
    std::string detail = Card::getDetailedDescription();
    
    // Add piece type information
    detail += "\nPiece Type: " + pieceType;
    detail += "\nPlacement: Can be placed on empty squares controlled by you";
    
    return detail;
}

std::unique_ptr<Card> PieceCard::clone() const {
    return std::make_unique<PieceCard>(id, name, description, steamCost, pieceType, rarity);
}

int PieceCard::getDefaultPlacementRow(PlayerSide player) const {
    // Player 1 (bottom) uses rows 6-7, Player 2 (top) uses rows 0-1
    return (player == PlayerSide::PLAYER_ONE) ? 7 : 0;
}

} // namespace BayouBonanza 