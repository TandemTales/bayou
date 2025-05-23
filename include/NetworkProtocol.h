#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include <SFML/Network.hpp>
#include "PlayerSide.h" // From the game's include directory
#include "Piece.h"      // For Position, and as a reference for PieceType
// GameBoard.h and GameState.h are not directly included here to keep this header light,
// but their structures inform the GameStateUpdate message.

namespace BayouBonanza {
namespace Network {

// Enum for identifying different types of pieces for network transmission
enum class PieceType : sf::Uint8 {
    NONE = 0, // Represents an empty square
    KING,
    QUEEN,
    ROOK,
    BISHOP,
    KNIGHT,
    PAWN
    // Add other piece types as they are created
};

// Enum for message types exchanged between client and server
enum class MessageType : sf::Uint8 {
    // Client to Server
    ClientReady,        // Client -> Server: Client is connected and ready to receive game info / start.
    PlayerAction,       // Client -> Server: Client wants to perform a game action (e.g., move a piece).

    // Server to Client
    AssignPlayerIdentity, // Server -> Client: Assigns PlayerSide (ONE or TWO) to the client.
    GameStart,          // Server -> Client(s): Signals the game is starting. Contains initial game state.
    GameStateUpdate,    // Server -> Client(s): Sends the current state of the game (full or delta).
    TurnChange,         // Server -> Client(s): Indicates whose turn it is.
    ActionValid,        // Server -> Client: Confirms a PlayerAction was valid and processed. (Optional, could be part of GameStateUpdate)
    ActionInvalid,      // Server -> Client: Informs client that their PlayerAction was invalid.
    GameOver,           // Server -> Client(s): Announces the game has ended and who won.
    ServerFull,         // Server -> Client: If a client tries to connect but server is full.
    HeartbeatRequest,   // Server -> Client: Check if client is still alive.
    HeartbeatResponse   // Client -> Server: Response to HeartbeatRequest.
};

// --- Data Outline for Messages (Comments) ---

// MessageType::ClientReady
// Data:
// - (Potentially) Client version, player name/preference (if any)
// For now: Empty, just signals readiness.

// MessageType::PlayerAction
// Data:
// - Move data:
//   - Position from (e.g., from.x, from.y)
//   - Position to (e.g., to.x, to.y)
//   - (Optional) Promotion piece type (if applicable, like in chess Pawns reaching the end)
//     - For now, we'll assume no promotions for simplicity. PieceType promotionPiece;

// MessageType::AssignPlayerIdentity
// Data:
// - PlayerSide assignedSide; (PLAYER_ONE or PLAYER_TWO)

// MessageType::GameStart
// Data:
// - Initial GameBoard state (see GameStateUpdate for board serialization details)
// - PlayerSide startingPlayer;
// - Initial steam amounts for both players (int steamP1, int steamP2)

// MessageType::GameStateUpdate
// Data:
// - Full GameBoard:
//   - For each of the 64 squares (e.g., row by row):
//     - PieceType pieceType; (e.g., PieceType::KING, PieceType::PAWN, PieceType::NONE if empty)
//     - PlayerSide pieceOwner; (e.g., PlayerSide::PLAYER_ONE, relevant if pieceType != NONE)
//     - sf::Int8 health; (relevant if pieceType != NONE and health can vary)
// - PlayerSide activePlayer;
// - int turnNumber;
// - int steamPlayer1;
// - int steamPlayer2;
// - (Optional) GamePhase phase;
// - (Optional) GameResult result; (if game is over, this might be sent with GameOver instead)

// MessageType::TurnChange
// Data:
// - PlayerSide newActivePlayer;
// - int currentTurnNumber;

// MessageType::ActionValid
// Data: (Optional, if not just relying on GameStateUpdate)
// - The Move that was validated (from.x, from.y, to.x, to.y)
// - (Potentially any consequences, like updated steam, but GameStateUpdate might cover this)

// MessageType::ActionInvalid
// Data:
// - The Move that was invalid (from.x, from.y, to.x, to.y)
// - sf::Uint8 reasonCode; or std::string reasonMessage;

// MessageType::GameOver
// Data:
// - PlayerSide winner; (PLAYER_ONE, PLAYER_TWO, or NEUTRAL for draw)
// - (Optional) GameResult result; (if not sent in GameStateUpdate)
// - (Optional) Final board state or scores

// MessageType::ServerFull
// Data: Empty, type itself is the message.

// MessageType::HeartbeatRequest
// Data: Empty

// MessageType::HeartbeatResponse
// Data: Empty


// --- Serialization for PlayerSide ---
inline sf::Packet& operator <<(sf::Packet& packet, const PlayerSide& side) {
    return packet << static_cast<sf::Uint8>(side);
}

inline sf::Packet& operator >>(sf::Packet& packet, PlayerSide& side) {
    sf::Uint8 temp;
    packet >> temp;
    side = static_cast<PlayerSide>(temp);
    return packet;
}

// --- Serialization for Position (from Piece.h) ---
// Position is simple enough (two ints) that we can serialize it directly
// where needed (e.g., within Move serialization) or create operators if used frequently standalone.
inline sf::Packet& operator <<(sf::Packet& packet, const Position& pos) {
    return packet << static_cast<sf::Int8>(pos.x) << static_cast<sf::Int8>(pos.y);
}

inline sf::Packet& operator >>(sf::Packet& packet, Position& pos) {
    sf::Int8 x, y;
    packet >> x >> y;
    pos.x = x;
    pos.y = y;
    return packet;
}


// --- Serialization for Move (from Move.h) ---
// A Move consists of a start position and an end position.
// The actual piece is usually inferred by the server based on the 'from' position.
// For now, we don't serialize the shared_ptr<Piece> part of the Move class.
struct NetworkMove {
    Position from;
    Position to;
    // PieceType promotionType; // Example if promotion was a feature

    NetworkMove(const Position& fromPos = {}, const Position& toPos = {})
        : from(fromPos), to(toPos) {}
};

inline sf::Packet& operator <<(sf::Packet& packet, const NetworkMove& move) {
    return packet << move.from << move.to;
    // if promotion: packet << static_cast<sf::Uint8>(move.promotionType);
}

inline sf::Packet& operator >>(sf::Packet& packet, NetworkMove& move) {
    packet >> move.from >> move.to;
    // if promotion: {
    //    sf::Uint8 temp;
    //    packet >> temp;
    //    move.promotionType = static_cast<PieceType>(temp);
    // }
    return packet;
}

// --- Plan for Piece and GameBoard Serialization (Comments) ---

// Serializing a Piece:
// To send a piece, we'd typically send:
// 1. PieceType itsType; (e.g. PieceType::PAWN)
// 2. PlayerSide itsOwner; (e.g. PlayerSide::PLAYER_ONE)
// 3. sf::Int8 itsHealth; (current health)
// (Position is implicit by its location on the board during GameBoard serialization)
//
// Example:
// sf::Packet& operator <<(sf::Packet& packet, const SerializablePiece& piece) {
//     packet << static_cast<sf::Uint8>(piece.type);
//     packet << piece.owner; // Uses PlayerSide's operator
//     packet << piece.health;
//     return packet;
// }
// sf::Packet& operator >>(sf::Packet& packet, SerializablePiece& piece) {
//     sf::Uint8 type_temp;
//     packet >> type_temp >> piece.owner >> piece.health;
//     piece.type = static_cast<PieceType>(type_temp);
//     return packet;
// }


// Serializing GameBoard (typically as part of GameStateUpdate):
// Iterate through each square of the board (e.g., 8x8 = 64 squares).
// For each square:
//   If a piece exists:
//     Send PieceType (e.g., KING, PAWN).
//     Send PlayerSide of the piece.
//     Send sf::Int8 current health of the piece.
//   If the square is empty:
//     Send PieceType::NONE.
//     (No need to send PlayerSide or health for an empty square).
//
// Example structure within a GameStateUpdate packet:
// packet << MessageType::GameStateUpdate;
// for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
//     for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
//         const Square& square = board.getSquare(x, y);
//         if (square.isEmpty()) {
//             packet << static_cast<sf::Uint8>(PieceType::NONE);
//         } else {
//             std::shared_ptr<Piece> piece = square.getPiece();
//             packet << static_cast<sf::Uint8>(getNetworkPieceType(piece->getTypeName())); // Requires a helper to map string to PieceType
//             packet << piece->getSide(); // Uses PlayerSide's operator
//             packet << static_cast<sf::Int8>(piece->getHealth());
//         }
//     }
// }
// packet << activePlayer;
// packet << turnNumber;
// packet << steamPlayer1 << steamPlayer2;

} // namespace Network
} // namespace BayouBonanza

#endif // NETWORK_PROTOCOL_H
