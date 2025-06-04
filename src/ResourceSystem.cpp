#include "ResourceSystem.h"
#include "GameBoard.h"
#include "InfluenceSystem.h"
#include "Square.h"
#include <algorithm>
#include <stdexcept>

namespace BayouBonanza {

ResourceSystem::ResourceSystem(int startingSteam) 
    : player1Steam(startingSteam)
    , player2Steam(startingSteam)
    , lastPlayer1Generation(0)
    , lastPlayer2Generation(0) {
    
    if (startingSteam < 0) {
        throw std::invalid_argument("Starting steam cannot be negative");
    }
}

int ResourceSystem::getSteam(PlayerSide player) const {
    switch (player) {
        case PlayerSide::PLAYER_ONE:
            return player1Steam;
        case PlayerSide::PLAYER_TWO:
            return player2Steam;
        case PlayerSide::NEUTRAL:
            return 0; // Neutral player has no steam
        default:
            throw std::invalid_argument("Invalid player side");
    }
}

void ResourceSystem::setSteam(PlayerSide player, int amount) {
    if (amount < 0) {
        throw std::invalid_argument("Steam amount cannot be negative");
    }
    
    switch (player) {
        case PlayerSide::PLAYER_ONE:
            player1Steam = amount;
            break;
        case PlayerSide::PLAYER_TWO:
            player2Steam = amount;
            break;
        case PlayerSide::NEUTRAL:
            // Neutral player cannot have steam set
            break;
        default:
            throw std::invalid_argument("Invalid player side");
    }
}

void ResourceSystem::addSteam(PlayerSide player, int amount) {
    if (amount < 0) {
        throw std::invalid_argument("Cannot add negative steam amount");
    }
    
    switch (player) {
        case PlayerSide::PLAYER_ONE:
            player1Steam += amount;
            break;
        case PlayerSide::PLAYER_TWO:
            player2Steam += amount;
            break;
        case PlayerSide::NEUTRAL:
            // Neutral player cannot gain steam
            break;
        default:
            throw std::invalid_argument("Invalid player side");
    }
}

bool ResourceSystem::spendSteam(PlayerSide player, int amount) {
    if (amount < 0) {
        throw std::invalid_argument("Cannot spend negative steam amount");
    }
    
    if (amount == 0) {
        return true; // Spending 0 steam always succeeds
    }
    
    switch (player) {
        case PlayerSide::PLAYER_ONE:
            if (player1Steam >= amount) {
                player1Steam -= amount;
                return true;
            }
            return false;
            
        case PlayerSide::PLAYER_TWO:
            if (player2Steam >= amount) {
                player2Steam -= amount;
                return true;
            }
            return false;
            
        case PlayerSide::NEUTRAL:
            return false; // Neutral player cannot spend steam
            
        default:
            throw std::invalid_argument("Invalid player side");
    }
}

std::pair<int, int> ResourceSystem::calculateSteamGeneration(const GameBoard& board) {
    int player1Generation = 0;
    int player2Generation = 0;
    
    // Iterate through all squares on the board
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            const Square& square = board.getSquare(x, y);
            PlayerSide controller = InfluenceSystem::getControllingPlayer(square);
            
            // Count controlled squares for each player
            switch (controller) {
                case PlayerSide::PLAYER_ONE:
                    player1Generation++;
                    break;
                case PlayerSide::PLAYER_TWO:
                    player2Generation++;
                    break;
                case PlayerSide::NEUTRAL:
                    // Neutral squares don't generate steam for anyone
                    break;
                default:
                    // Should not happen, but handle gracefully
                    break;
            }
        }
    }
    
    // Store generation values for debugging/UI purposes
    lastPlayer1Generation = player1Generation;
    lastPlayer2Generation = player2Generation;
    
    return std::make_pair(player1Generation, player2Generation);
}

void ResourceSystem::processTurnStart(PlayerSide activePlayer, const GameBoard& board) {
    // Calculate current steam generation
    auto generation = calculateSteamGeneration(board);
    
    // Add generated steam to the active player
    switch (activePlayer) {
        case PlayerSide::PLAYER_ONE:
            addSteam(PlayerSide::PLAYER_ONE, generation.first);
            break;
        case PlayerSide::PLAYER_TWO:
            addSteam(PlayerSide::PLAYER_TWO, generation.second);
            break;
        case PlayerSide::NEUTRAL:
            // Neutral player doesn't get turns - this is an error
            throw std::invalid_argument("Neutral player cannot have a turn");
        default:
            throw std::invalid_argument("Invalid active player");
    }
}

std::pair<int, int> ResourceSystem::getLastGenerationValues() const {
    return std::make_pair(lastPlayer1Generation, lastPlayer2Generation);
}

void ResourceSystem::reset(int startingSteam) {
    if (startingSteam < 0) {
        throw std::invalid_argument("Starting steam cannot be negative");
    }
    
    player1Steam = startingSteam;
    player2Steam = startingSteam;
    lastPlayer1Generation = 0;
    lastPlayer2Generation = 0;
}

} // namespace BayouBonanza 