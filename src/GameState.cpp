#include "GameState.h"
#include "GameBoard.h" // For GameBoard serialization
#include "PlayerSide.h" // For PlayerSide serialization
#include "CardFactory.h" // For card system initialization
#include "CardPlayValidator.h" // For comprehensive card validation
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
    
    // Initialize card system
    initializeCardSystem();
    
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
    
    // Process card-related turn start events
    processCardTurnStart();
}

// Card System Integration Methods

Deck& GameState::getDeck(PlayerSide side) {
    return (side == PlayerSide::PLAYER_ONE) ? deckPlayer1 : deckPlayer2;
}

const Deck& GameState::getDeck(PlayerSide side) const {
    return (side == PlayerSide::PLAYER_ONE) ? deckPlayer1 : deckPlayer2;
}

Hand& GameState::getHand(PlayerSide side) {
    return (side == PlayerSide::PLAYER_ONE) ? handPlayer1 : handPlayer2;
}

const Hand& GameState::getHand(PlayerSide side) const {
    return (side == PlayerSide::PLAYER_ONE) ? handPlayer1 : handPlayer2;
}

bool GameState::drawCard(PlayerSide side) {
    Deck& deck = getDeck(side);
    Hand& hand = getHand(side);
    
    // Check if deck has cards and hand has space
    if (deck.empty() || hand.isFull()) {
        return false;
    }
    
    // Draw a card from the deck
    auto card = deck.drawCard();
    if (!card) {
        return false;
    }
    
    // Add the card to the hand
    return hand.addCard(std::move(card));
}

bool GameState::playCard(PlayerSide side, size_t handIndex, const Position& targetPosition) {
    // Use the comprehensive CardPlayValidator for validation and execution
    PlayResult result = CardPlayValidator::executeCardPlay(*this, side, handIndex, targetPosition);
    
    // The CardPlayValidator handles all validation, steam deduction, card removal,
    // execution, and rollback automatically, so we just need to return the result
    return result.success;
}

PlayResult GameState::playCardWithResult(PlayerSide side, size_t handIndex, const Position& targetPosition) {
    // Use the comprehensive CardPlayValidator for validation and execution
    return CardPlayValidator::executeCardPlay(*this, side, handIndex, targetPosition);
}

ValidationResult GameState::validateCardPlay(PlayerSide side, size_t handIndex, const Position& targetPosition) const {
    // Use the CardPlayValidator for validation without execution
    if (targetPosition.x != -1 && targetPosition.y != -1) {
        return CardPlayValidator::validateTargetedCardPlay(*this, side, handIndex, targetPosition);
    } else {
        return CardPlayValidator::validateCardPlay(*this, side, handIndex);
    }
}

void GameState::initializeCardSystem() {
    // Initialize the CardFactory if not already done
    CardFactory::initialize();
    
    // Create starter decks for both players
    auto player1Cards = CardFactory::createStarterDeck();
    auto player2Cards = CardFactory::createStarterDeck();
    
    // Initialize decks
    deckPlayer1 = Deck(std::move(player1Cards));
    deckPlayer2 = Deck(std::move(player2Cards));
    
    // Shuffle the decks
    deckPlayer1.shuffle();
    deckPlayer2.shuffle();
    
    // Clear hands
    handPlayer1.clear();
    handPlayer2.clear();
    
    // Draw initial hands (4 cards each)
    for (int i = 0; i < Hand::MAX_HAND_SIZE; i++) {
        drawCard(PlayerSide::PLAYER_ONE);
        drawCard(PlayerSide::PLAYER_TWO);
    }
}

void GameState::processCardTurnStart() {
    // Draw a card for the active player if their hand isn't full
    if (!getHand(activePlayer).isFull()) {
        drawCard(activePlayer);
    }
    
    // TODO: Process any ongoing card effects or status effects
    // This would be implemented when we have a status effect system
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
    
    // Serialize card system - hands only (decks are not synchronized)
    const Hand& hand1 = gs.getHand(PlayerSide::PLAYER_ONE);
    const Hand& hand2 = gs.getHand(PlayerSide::PLAYER_TWO);
    
    // Serialize Player 1 hand
    packet << static_cast<sf::Uint32>(hand1.size());
    for (size_t i = 0; i < hand1.size(); ++i) {
        const Card* card = hand1.getCard(i);
        if (card) {
            packet << card->getId();
        } else {
            packet << static_cast<int>(-1); // Invalid card ID
        }
    }
    
    // Serialize Player 2 hand
    packet << static_cast<sf::Uint32>(hand2.size());
    for (size_t i = 0; i < hand2.size(); ++i) {
        const Card* card = hand2.getCard(i);
        if (card) {
            packet << card->getId();
        } else {
            packet << static_cast<int>(-1); // Invalid card ID
        }
    }
    
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
    
    // Deserialize card system - hands only
    Hand& hand1 = gs.getHand(PlayerSide::PLAYER_ONE);
    Hand& hand2 = gs.getHand(PlayerSide::PLAYER_TWO);
    
    // Clear existing hands
    hand1.clear();
    hand2.clear();
    
    // Deserialize Player 1 hand
    sf::Uint32 hand1Size;
    packet >> hand1Size;
    for (sf::Uint32 i = 0; i < hand1Size; ++i) {
        int cardId;
        packet >> cardId;
        if (cardId != -1) {
            auto card = CardFactory::createCard(cardId);
            if (card) {
                hand1.addCard(std::move(card));
            }
        }
    }
    
    // Deserialize Player 2 hand
    sf::Uint32 hand2Size;
    packet >> hand2Size;
    for (sf::Uint32 i = 0; i < hand2Size; ++i) {
        int cardId;
        packet >> cardId;
        if (cardId != -1) {
            auto card = CardFactory::createCard(cardId);
            if (card) {
                hand2.addCard(std::move(card));
            }
        }
    }
    
    return packet;
}

} // namespace BayouBonanza
