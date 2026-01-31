#ifndef BAYOU_BONANZA_GAME_DEBUG_H
#define BAYOU_BONANZA_GAME_DEBUG_H

#include <iostream>

#include "GameState.h"
#include "PlayerSide.h"

namespace BayouBonanza {

void printBoardState(const GameState& gameState,
                     PlayerSide localPlayer = PlayerSide::NEUTRAL);

} // namespace BayouBonanza

#endif // BAYOU_BONANZA_GAME_DEBUG_H
