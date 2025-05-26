#pragma once

#include <memory>
#include "Piece.h"
#include "PlayerSide.h"

namespace BayouBonanza {

/**
 * @brief Factory class for creating different piece types
 * 
 * This class centralizes the creation of pieces and provides
 * methods to create each specific piece type.
 */
class PieceFactory {
public:
    /**
     * @brief Create a King piece
     * 
     * @param side The player side that owns the piece
     * @return Shared pointer to the created King
     */
    static std::shared_ptr<Piece> createKing(PlayerSide side);
    
    /**
     * @brief Create a Queen piece
     * 
     * @param side The player side that owns the piece
     * @return Shared pointer to the created Queen
     */
    static std::shared_ptr<Piece> createQueen(PlayerSide side);
    
    /**
     * @brief Create a Rook piece
     * 
     * @param side The player side that owns the piece
     * @return Shared pointer to the created Rook
     */
    static std::shared_ptr<Piece> createRook(PlayerSide side);
    
    /**
     * @brief Create a Bishop piece
     * 
     * @param side The player side that owns the piece
     * @return Shared pointer to the created Bishop
     */
    static std::shared_ptr<Piece> createBishop(PlayerSide side);
    
    /**
     * @brief Create a Knight piece
     * 
     * @param side The player side that owns the piece
     * @return Shared pointer to the created Knight
     */
    static std::shared_ptr<Piece> createKnight(PlayerSide side);
    
    /**
     * @brief Create a Pawn piece
     * 
     * @param side The player side that owns the piece
     * @return Shared pointer to the created Pawn
     */
    static std::shared_ptr<Piece> createPawn(PlayerSide side);
    
    /**
     * @brief Create a piece by type name
     * 
     * @param typeName The name of the piece type (e.g., "King", "Queen")
     * @param side The player side that owns the piece
     * @return Shared pointer to the created piece, or nullptr if type is invalid
     */
    static std::shared_ptr<Piece> createPieceByType(const std::string& typeName, PlayerSide side);

    /**
     * @brief Create a piece by PieceType enum
     * 
     * @param type The enum type of the piece
     * @param side The player side that owns the piece
     * @return Shared pointer to the created piece, or nullptr if type is invalid
     */
    static std::shared_ptr<Piece> createPieceByPieceType(PieceType type, PlayerSide side);
};

} // namespace BayouBonanza
