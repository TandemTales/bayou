#include "GameState.h"
#include "GameBoard.h" // For GameBoard serialization
#include "PlayerSide.h" // For PlayerSide serialization
#include <SFML/Network/Packet.hpp> // For sf::Packet

// GameState.h should have already included these.
// GameBoard.h includes Square.h, etc.
// PlayerSide.h includes its own packet operators.

namespace BayouBonanza {

GameState::GameState() :
    activePlayer(PlayerSide::PLAYER_ONE),
    phase(GamePhase::SETUP),
    result(GameResult::IN_PROGRESS),
    turnNumber(1),
    resourceSystem(0), // Initialize ResourceSystem with 0 starting steam
    steamPlayer1(0),
    steamPlayer2(0) {
}

GameBoard& GameState::getBoard() {
    return board;
}

const GameBoard& GameState::getBoard() const {
    return board;
}

PlayerSide GameState::getActivePlayer() const {
    return activePlayer;
}

void GameState::setActivePlayer(PlayerSide player) {
    activePlayer = player;
}

void GameState::switchActivePlayer() {
    activePlayer = (activePlayer == PlayerSide::PLAYER_ONE) ? 
                   PlayerSide::PLAYER_TWO : PlayerSide::PLAYER_ONE;
}

GamePhase GameState::getGamePhase() const {
    return phase;
}

void GameState::setGamePhase(GamePhase phase) {
    this->phase = phase;
}

GameResult GameState::getGameResult() const {
    return result;
}

void GameState::setGameResult(GameResult result) {
    this->result = result;
    
    // If we're setting a result other than IN_PROGRESS, also update the game phase
    if (result != GameResult::IN_PROGRESS) {
        setGamePhase(GamePhase::GAME_OVER);
    }
}

void GameState::initializeNewGame() {
    // Reset the board
    board.resetBoard();
    
    // Set initial game state
    activePlayer = PlayerSide::PLAYER_ONE;
    phase = GamePhase::MAIN_GAME;
    result = GameResult::IN_PROGRESS;
    turnNumber = 1;
    
    // Reset ResourceSystem
    resourceSystem.reset(0);
    
    // Reset legacy steam tracking for backward compatibility
    steamPlayer1 = 0;
    steamPlayer2 = 0;
    
    // TODO: Place initial pieces on the board (King for each player)
    // This will be implemented when we have the Piece class
}

int GameState::getTurnNumber() const {
    return turnNumber;
}

void GameState::setTurnNumber(int turn) {
    turnNumber = turn;
}

void GameState::incrementTurnNumber() {
    turnNumber++;
}

int GameState::getSteam(PlayerSide side) const {
    // Delegate to ResourceSystem for primary steam tracking
    return resourceSystem.getSteam(side);
}

void GameState::setSteam(PlayerSide side, int amount) {
    // Delegate to ResourceSystem
    resourceSystem.setSteam(side, amount);
    
    // Update legacy tracking for backward compatibility
    if (side == PlayerSide::PLAYER_ONE) {
        steamPlayer1 = amount;
    } else if (side == PlayerSide::PLAYER_TWO) {
        steamPlayer2 = amount;
    }
}

void GameState::addSteam(PlayerSide side, int amount) {
    // Delegate to ResourceSystem
    resourceSystem.addSteam(side, amount);
    
    // Update legacy tracking for backward compatibility
    if (side == PlayerSide::PLAYER_ONE) {
        steamPlayer1 = resourceSystem.getSteam(PlayerSide::PLAYER_ONE);
    } else if (side == PlayerSide::PLAYER_TWO) {
        steamPlayer2 = resourceSystem.getSteam(PlayerSide::PLAYER_TWO);
    }
}

bool GameState::spendSteam(PlayerSide side, int amount) {
    // Delegate to ResourceSystem
    bool success = resourceSystem.spendSteam(side, amount);
    
    // Update legacy tracking for backward compatibility
    if (success) {
        if (side == PlayerSide::PLAYER_ONE) {
            steamPlayer1 = resourceSystem.getSteam(PlayerSide::PLAYER_ONE);
        } else if (side == PlayerSide::PLAYER_TWO) {
            steamPlayer2 = resourceSystem.getSteam(PlayerSide::PLAYER_TWO);
        }
    }
    
    return success;
}

ResourceSystem& GameState::getResourceSystem() {
    return resourceSystem;
}

const ResourceSystem& GameState::getResourceSystem() const {
    return resourceSystem;
}

void GameState::processTurnStart() {
    // Use ResourceSystem to process turn start and calculate steam generation
    resourceSystem.processTurnStart(activePlayer, board);
    
    // Update legacy tracking for backward compatibility
    steamPlayer1 = resourceSystem.getSteam(PlayerSide::PLAYER_ONE);
    steamPlayer2 = resourceSystem.getSteam(PlayerSide::PLAYER_TWO);
}

// SFML Packet operators for GamePhase enum
sf::Packet& operator<<(sf::Packet& packet, const GamePhase& phase) {
    return packet << static_cast<int>(phase);
}

sf::Packet& operator>>(sf::Packet& packet, GamePhase& phase) {
    int value;
    packet >> value;
    phase = static_cast<GamePhase>(value);
    return packet;
}

// SFML Packet operators for GameResult enum
sf::Packet& operator<<(sf::Packet& packet, const GameResult& result) {
    return packet << static_cast<int>(result);
}

sf::Packet& operator>>(sf::Packet& packet, GameResult& result) {
    int value;
    packet >> value;
    result = static_cast<GameResult>(value);
    return packet;
}

// SFML Packet operators for GameState
sf::Packet& operator<<(sf::Packet& packet, const GameState& gs) {
    packet << gs.getBoard();
    packet << gs.getActivePlayer();
    packet << gs.getGamePhase();
    packet << gs.getGameResult();
    packet << gs.getTurnNumber();
    packet << gs.getSteam(PlayerSide::PLAYER_ONE);
    packet << gs.getSteam(PlayerSide::PLAYER_TWO);
    return packet;
}

sf::Packet& operator>>(sf::Packet& packet, GameState& gs) {
    PlayerSide activePlayer;
    GamePhase phase;
    GameResult result;
    int turnNumber;
    int steamPlayer1;
    int steamPlayer2;
    
    // Deserialize directly into the GameState's board
    packet >> gs.getBoard();
    packet >> activePlayer;
    packet >> phase;
    packet >> result;
    packet >> turnNumber;
    packet >> steamPlayer1;
    packet >> steamPlayer2;
    
    // Set the deserialized values
    gs.setActivePlayer(activePlayer);
    gs.setGamePhase(phase);
    gs.setGameResult(result);
    gs.setTurnNumber(turnNumber);
    gs.setSteam(PlayerSide::PLAYER_ONE, steamPlayer1);
    gs.setSteam(PlayerSide::PLAYER_TWO, steamPlayer2);
    
    return packet;
}

} // namespace BayouBonanza
