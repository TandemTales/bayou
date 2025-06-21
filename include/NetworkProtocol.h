#pragma once
#include <SFML/Config.hpp> // For sf::Uint8
#include <SFML/Network/Packet.hpp> // For sf::Packet, needed for the operators

enum class MessageType : sf::Uint8 {
    ConnectionRequest,      // Client to Server: Initial connection (optional, server might just accept)
    ConnectionAccepted,     // Server to Client: Sent when server accepts the connection
    PlayerAssignment,       // Server to Client: Assigns PlayerSide (PLAYER_ONE or PLAYER_TWO)
    WaitingForOpponent,     // Server to Client: Sent to the first client while waiting for the second
    GameStart,              // Server to Client: Indicates the game is starting and sends initial GameState
    MoveToServer,           // Client to Server: Player sends a move
    CardPlayToServer,       // Client to Server: Player plays a card
    EndTurn,                // Client to Server: Player ends their turn/advances phase
    MoveRejected,           // Server to Client: Move was invalid (optional, or just send new state)
    CardPlayRejected,       // Server to Client: Card play was invalid (optional)
    GameStateUpdate,        // Server to Client: Sends the full updated GameState
    GameOver,               // Server to Client: Announces game over and result (optional for now)
    Error,                  // Server to Client or Client to Server: Generic error message
    Ping,                   // Client to Server (optional, for keep-alive)
    Pong,                   // Server to Client (optional, for keep-alive)
    UserLogin,              // Client to Server: Sends username for login/registration
    CardCollectionData,     // Server to Client: Sends player's full card collection
    DeckData,               // Server to Client: Sends the player's deck
    SaveDeck                // Client to Server: Save deck changes
};

/**
 * @brief Structure for card play data sent over network
 */
struct CardPlayData {
    int cardIndex;          // Index of the card in the player's hand
    int targetX;            // Target X coordinate on the board
    int targetY;            // Target Y coordinate on the board
    
    CardPlayData() : cardIndex(-1), targetX(-1), targetY(-1) {}
    CardPlayData(int index, int x, int y) : cardIndex(index), targetX(x), targetY(y) {}
};

// Packet operators for CardPlayData
inline sf::Packet& operator<<(sf::Packet& packet, const CardPlayData& data) {
    return packet << data.cardIndex << data.targetX << data.targetY;
}

inline sf::Packet& operator>>(sf::Packet& packet, CardPlayData& data) {
    return packet >> data.cardIndex >> data.targetX >> data.targetY;
}

// Operator to stream MessageType into sf::Packet
inline sf::Packet& operator<<(sf::Packet& packet, MessageType type) {
    return packet << static_cast<sf::Uint8>(type);
}

// Operator to stream MessageType from sf::Packet
inline sf::Packet& operator>>(sf::Packet& packet, MessageType& type) {
    sf::Uint8 val;
    packet >> val;
    type = static_cast<MessageType>(val);
    return packet;
}
