#pragma once
#include <SFML/Config.hpp> // For sf::Uint8
#include <SFML/Network/Packet.hpp> // For sf::Packet

namespace BayouBonanza {

/**
 * @brief Enum representing player sides and neutral control
 */
enum class PlayerSide {
    PLAYER_ONE,  // First player
    PLAYER_TWO,  // Second player
    NEUTRAL      // Used for squares with equal control or no control
};

// SFML Packet operators for PlayerSide
inline sf::Packet& operator<<(sf::Packet& packet, const PlayerSide& side) {
    return packet << static_cast<sf::Uint8>(side);
}

inline sf::Packet& operator>>(sf::Packet& packet, PlayerSide& side) {
    sf::Uint8 side_uint8;
    packet >> side_uint8;
    side = static_cast<PlayerSide>(side_uint8);
    return packet;
}

} // namespace BayouBonanza
