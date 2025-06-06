#include "PieceCard.h"
#include "GameState.h"
#include "GameBoard.h"
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"
#include <algorithm>

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
    
    // Check placement rules based on piece type and player
    int defaultRow = getDefaultPlacementRow(player);
    
    // For most pieces, allow placement on the player's back two rows
    if (player == PlayerSide::PLAYER_ONE) {
        // Player 1 starts at bottom (rows 6-7)
        return position.y >= 6;
    } else {
        // Player 2 starts at top (rows 0-1)
        return position.y <= 1;
    }
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
    
    // Create the piece using the existing piece factory system
    std::string typeName = pieceType;
    
    // Get the piece definition manager and factory from game state
    // Note: This is a simplified approach - in a real implementation,
    // the factory would be accessible through the game state
    try {
        // For now, we'll create a basic piece manually
        // This should be replaced with proper factory integration
        
        // Get the square and place the piece
        GameBoard& board = gameState.getBoard();
        Square& square = board.getSquare(position.x, position.y);
        
        // Create a basic piece with default stats
        // This is a temporary implementation until proper factory integration
        PieceStats defaultStats;
        defaultStats.health = 100;
        defaultStats.attack = 50;
        defaultStats.symbol = typeName.substr(0, 1); // First letter as symbol
        
        auto piece = std::make_unique<Piece>(player, defaultStats);
        piece->setPosition(position);
        
        // Place the piece on the board
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
    detail += "\nPlacement: Can be placed on your starting rows";
    
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