#pragma once

namespace BayouBonanza {

/**
 * @brief Enum representing player sides and neutral control
 */
enum class PlayerSide {
    PLAYER_ONE,  // First player
    PLAYER_TWO,  // Second player
    NEUTRAL      // Used for squares with equal control or no control
};

} // namespace BayouBonanza
