#pragma once

#include "Card.h"
#include "Piece.h"
#include "PieceData.h"

namespace BayouBonanza {

/**
 * @brief Card that spawns a piece on the board
 * 
 * PieceCards allow players to spend steam to place new pieces on the board.
 * The piece type, placement rules, and cost are defined by the card.
 */
class PieceCard : public Card {
public:
    /**
     * @brief Constructor
     * 
     * @param id Unique identifier for this card
     * @param name Display name of the card
     * @param description Text description of the card's effect
     * @param steamCost Amount of steam required to play this card
     * @param pieceType The type of piece this card creates
     * @param rarity The rarity level of this card
     */
    PieceCard(int id, const std::string& name, const std::string& description,
              int steamCost, PieceType pieceType, CardRarity rarity = CardRarity::COMMON);
    
    /**
     * @brief Get the type of piece this card creates
     * 
     * @return The piece type
     */
    PieceType getPieceType() const;
    
    /**
     * @brief Check if this card can be played in the current game state
     * 
     * Validates that:
     * - Player has enough steam
     * - There are valid placement positions available
     * - Game state allows piece placement
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @return true if the card can be played, false otherwise
     */
    bool canPlay(const GameState& gameState, PlayerSide player) const override;
    
    /**
     * @brief Execute the card's effect - place a piece on the board
     * 
     * This method will:
     * - Deduct steam cost from player
     * - Create the specified piece type
     * - Place it on a valid position (determined by game rules)
     * 
     * @param gameState The game state to modify
     * @param player The player playing the card
     * @return true if the card was played successfully, false otherwise
     */
    bool play(GameState& gameState, PlayerSide player) const override;
    
    /**
     * @brief Check if a specific position is valid for piece placement
     * 
     * @param gameState The current game state
     * @param player The player attempting placement
     * @param position The position to check
     * @return true if the position is valid for placement
     */
    bool isValidPlacement(const GameState& gameState, PlayerSide player, const Position& position) const;
    
    /**
     * @brief Get all valid placement positions for this card
     * 
     * @param gameState The current game state
     * @param player The player attempting to play the card
     * @return Vector of valid positions where the piece can be placed
     */
    std::vector<Position> getValidPlacements(const GameState& gameState, PlayerSide player) const;
    
    /**
     * @brief Play the card at a specific position
     * 
     * @param gameState The game state to modify
     * @param player The player playing the card
     * @param position The position to place the piece
     * @return true if the card was played successfully, false otherwise
     */
    bool playAtPosition(GameState& gameState, PlayerSide player, const Position& position) const;
    
    /**
     * @brief Get a detailed description of what this card does
     * 
     * @return Extended description including piece stats and placement rules
     */
    std::string getDetailedDescription() const override;
    
    /**
     * @brief Create a copy of this card
     * 
     * @return A unique pointer to a copy of this card
     */
    std::unique_ptr<Card> clone() const override;

private:
    PieceType pieceType;
    
    /**
     * @brief Get the default placement position for a player
     * 
     * @param player The player side
     * @return The default starting row for piece placement
     */
    int getDefaultPlacementRow(PlayerSide player) const;
};

} // namespace BayouBonanza 