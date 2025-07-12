#include "InputManager.h"
#include "GraphicsManager.h"
#include "GameBoard.h"
#include <iostream>

namespace BayouBonanza {

InputManager::InputManager(sf::RenderWindow& window, 
                          sf::TcpSocket& socket,
                          GameState& gameState,
                          bool& gameHasStarted,
                          PlayerSide& myPlayerSide,
                          GraphicsManager& graphicsManager)
    : window(window)
    , socket(socket)
    , gameState(gameState)
    , gameHasStarted(gameHasStarted)
    , myPlayerSide(myPlayerSide)
    , graphicsManager(graphicsManager)
    , selectedPiece(nullptr)
    , originalSquareCoords(-1, -1)
    , mouseOffset(0.f, 0.f)
    , pieceSelected(false)
    , currentMousePosition(0.f, 0.f)
    , selectedCardIndex(-1)
    , cardSelected(false)
    , waitingForCardTarget(false)
    , cardDragging(false)
{
}

bool InputManager::handleEvent(const sf::Event& event) {
    switch (event.type) {
        case sf::Event::MouseButtonPressed:
            handleMouseButtonPressed(event);
            return true;
        
        case sf::Event::MouseMoved:
            handleMouseMoved(event);
            return true;
        
        case sf::Event::MouseButtonReleased:
            handleMouseButtonReleased(event);
            return true;
        
        case sf::Event::KeyPressed:
            handleKeyPressed(event);
            return false; // Don't claim to handle keyboard events
        
        default:
            return false; // Event not handled
    }
}

Piece* InputManager::getSelectedPiece() const {
    return selectedPiece;
}

bool InputManager::isPieceSelected() const {
    return pieceSelected;
}

sf::Vector2i InputManager::getOriginalSquareCoords() const {
    return originalSquareCoords;
}

sf::Vector2f InputManager::getCurrentMousePosition() const {
    return currentMousePosition;
}

sf::Vector2f InputManager::getMouseOffset() const {
    return mouseOffset;
}

void InputManager::resetInputState() {
    selectedPiece = nullptr;
    pieceSelected = false;
    originalSquareCoords = sf::Vector2i(-1, -1);
    mouseOffset = sf::Vector2f(0.f, 0.f);
    currentMousePosition = sf::Vector2f(0.f, 0.f);
    selectedCardIndex = -1;
    cardSelected = false;
    waitingForCardTarget = false;
    cardDragging = false;
}

sf::Vector2i InputManager::gamePosToBoard(const sf::Vector2f& gamePos) const {
    return graphicsManager.gameToBoard(gamePos);
}

void InputManager::handleMouseButtonPressed(const sf::Event& event) {
    if (event.mouseButton.button != sf::Mouse::Left) {
        return;
    }
    
    // Convert screen coordinates to game coordinates
    sf::Vector2i screenMousePos(event.mouseButton.x, event.mouseButton.y);
    sf::Vector2f gameMousePos = graphicsManager.screenToGame(screenMousePos);
    
    // Check if clicking on a card
    int cardIndex = getCardIndexAtPosition(gameMousePos);
    if (cardIndex >= 0) {
        startCardDrag(cardIndex, gameMousePos);
        return;
    }
    
    // Check if clicking on the board for piece selection
    sf::Vector2i boardCoords = gamePosToBoard(gameMousePos);
    if (boardCoords.x >= 0 && boardCoords.y >= 0) {
        const Square& square = gameState.getBoard().getSquare(boardCoords.x, boardCoords.y);
        if (!square.isEmpty() && square.getPiece()->getSide() == gameState.getActivePlayer()) {
            if (!square.getPiece()->isStunned()) {
                selectPiece(boardCoords.x, boardCoords.y, gameMousePos);
            }
        }
    }
}

void InputManager::handleMouseMoved(const sf::Event& event) {
    if (pieceSelected || cardDragging) {
        // Convert current screen mouse position to game coordinates
        sf::Vector2i screenMousePos = sf::Mouse::getPosition(window);
        currentMousePosition = graphicsManager.screenToGame(screenMousePos);
    }
}

void InputManager::handleMouseButtonReleased(const sf::Event& event) {
    if (event.mouseButton.button != sf::Mouse::Left) {
        return;
    }

    // Convert screen coordinates to game coordinates
    sf::Vector2i screenMousePos(event.mouseButton.x, event.mouseButton.y);
    sf::Vector2f gameMousePos = graphicsManager.screenToGame(screenMousePos);

    if (cardDragging) {
        sf::Vector2i targetCoords = gamePosToBoard(gameMousePos);
        if (targetCoords.x >= 0 && targetCoords.y >= 0) {
            attemptCardPlay(targetCoords.x, targetCoords.y);
        } else {
            std::cout << "Invalid card target: drop on the board." << std::endl;
            resetCardSelection();
        }
        cardDragging = false;
        return;
    }

    if (!pieceSelected) {
        return;
    }

    sf::Vector2i targetCoords = gamePosToBoard(gameMousePos);

    if (targetCoords.x >= 0 && targetCoords.y >= 0) {
        attemptMove(targetCoords.x, targetCoords.y);
    } else {
        std::cout << "Invalid move: Target square is off-board." << std::endl;
    }

    // Deselect piece regardless of move outcome
    resetInputState();
}

void InputManager::selectPiece(int boardX, int boardY, const sf::Vector2f& gameMousePos) {
    const Square& square = gameState.getBoard().getSquare(boardX, boardY);
    selectedPiece = square.getPiece();
    originalSquareCoords = sf::Vector2i(boardX, boardY);
    pieceSelected = true;
    
    // Calculate offset in game coordinates
    sf::Vector2f piecePos = graphicsManager.boardToGame(boardX, boardY);
    mouseOffset = sf::Vector2f(gameMousePos.x - piecePos.x, gameMousePos.y - piecePos.y);
    currentMousePosition = gameMousePos;
}

void InputManager::attemptMove(int targetX, int targetY) {
    // Create Position object for the target
    Position targetPosition(targetX, targetY);
    
    // Check if the move is valid according to piece logic
    if (selectedPiece && selectedPiece->getSide() == gameState.getActivePlayer() &&
        selectedPiece->isValidMove(gameState.getBoard(), targetPosition)) {
        
        std::cout << "Move validated. Processing action: "
                  << originalSquareCoords.x << "," << originalSquareCoords.y << " -> "
                  << targetX << "," << targetY << std::endl;
        
        Position startPosition(originalSquareCoords.x, originalSquareCoords.y);
        // Create a temporary shared_ptr wrapper for the Move constructor
        // Note: Using no-op deleter since the piece is owned by Square
        std::shared_ptr<Piece> piecePtr(selectedPiece, [](Piece*){});
        Move gameMove(piecePtr, startPosition, targetPosition);
        
        if (gameHasStarted && myPlayerSide == gameState.getActivePlayer()) {
            sendMoveToServer(gameMove);
        } else {
            std::cout << "Not your turn or game not started. Move not sent." << std::endl;
        }
    } else {
        // Invalid move (client-side validation before sending)
        std::cout << "Invalid move attempt: "
                  << originalSquareCoords.x << "," << originalSquareCoords.y << " -> "
                  << targetX << "," << targetY << std::endl;
        // Piece returns to original square visually (no GameState change was made)
    }
}

void InputManager::sendMoveToServer(const Move& move) {
    sf::Packet movePacket;
    movePacket << MessageType::MoveToServer << move;
    
    if (socket.send(movePacket) == sf::Socket::Done) {
        std::cout << "Move sent to server: " 
                  << move.getFrom().x << "," << move.getFrom().y << " -> " 
                  << move.getTo().x << "," << move.getTo().y << std::endl;
    } else {
        std::cerr << "Failed to send move to server." << std::endl;
    }
}

int InputManager::getSelectedCardIndex() const {
    return selectedCardIndex;
}

bool InputManager::isCardSelected() const {
    return cardSelected;
}

bool InputManager::isWaitingForCardTarget() const {
    return cardDragging;
}

int InputManager::getCardIndexAtPosition(const sf::Vector2f& gamePos) const {
    const Hand& hand = gameState.getHand(myPlayerSide);
    if (hand.size() == 0) return -1;
    
    auto boardParams = graphicsManager.getBoardRenderParams();
    
    // Card dimensions and positioning (must match renderPlayerHand)
    float cardWidth = 120.0f;
    float cardHeight = 120.0f; // Updated to match renderPlayerHand
    float cardSpacing = 10.0f;
    float totalHandWidth = hand.size() * cardWidth + (hand.size() - 1) * cardSpacing;
    
    // Position cards below the board, centered
    float handStartX = (GraphicsManager::BASE_WIDTH - totalHandWidth) / 2.0f;
    float handY = boardParams.boardStartY + boardParams.boardSize + 10.0f; // Reduced spacing to match renderPlayerHand
    
    // Check if click is within the hand area
    if (gamePos.y < handY || gamePos.y > handY + cardHeight) {
        return -1; // Not in hand area
    }
    
    // Check which card was clicked
    for (size_t i = 0; i < hand.size(); ++i) {
        float cardX = handStartX + i * (cardWidth + cardSpacing);
        if (gamePos.x >= cardX && gamePos.x <= cardX + cardWidth) {
            return static_cast<int>(i);
        }
    }
    
    return -1; // Not on any card
}

void InputManager::startCardDrag(int cardIndex, const sf::Vector2f& gameMousePos) {
    const Hand& hand = gameState.getHand(myPlayerSide);
    if (cardIndex < 0 || static_cast<size_t>(cardIndex) >= hand.size()) {
        return; // Invalid card index
    }
    
    const Card* card = hand.getCard(cardIndex);
    if (!card) {
        return; // No card at index
    }
    
    // Check if player can afford the card
    if (gameState.getSteam(myPlayerSide) < card->getSteamCost()) {
        std::cout << "Cannot select card: insufficient steam ("
                  << gameState.getSteam(myPlayerSide) << "/" << card->getSteamCost() << ")" << std::endl;
        return;
    }

    auto boardParams = graphicsManager.getBoardRenderParams();
    float cardWidth = 120.0f;
    float cardHeight = 120.0f;
    float cardSpacing = 10.0f;
    float totalHandWidth = hand.size() * cardWidth + (hand.size() - 1) * cardSpacing;
    float handStartX = (GraphicsManager::BASE_WIDTH - totalHandWidth) / 2.0f;
    float handY = boardParams.boardStartY + boardParams.boardSize + 10.0f;
    float cardX = handStartX + cardIndex * (cardWidth + cardSpacing);
    float cardY = handY;

    selectedCardIndex = cardIndex;
    cardSelected = true;
    cardDragging = true;
    waitingForCardTarget = false;

    // For drag offset
    mouseOffset = sf::Vector2f(gameMousePos.x - cardX, gameMousePos.y - cardY);
    currentMousePosition = gameMousePos;

    // Clear any piece selection
    selectedPiece = nullptr;
    pieceSelected = false;

    std::cout << "Card drag started: " << card->getName() << " (index " << cardIndex << ")" << std::endl;
}

void InputManager::attemptCardPlay(int targetX, int targetY) {
    if (!cardSelected || selectedCardIndex < 0) {
        return; // No card selected
    }
    
    Position targetPosition(targetX, targetY);
    
    std::cout << "Attempting to play card " << selectedCardIndex 
              << " at position (" << targetX << ", " << targetY << ")" << std::endl;
    
    if (gameHasStarted && myPlayerSide == gameState.getActivePlayer()) {
        sendCardPlayToServer(selectedCardIndex, targetPosition);
    } else {
        std::cout << "Not your turn or game not started. Card play not sent." << std::endl;
    }
    
    // Reset card selection state
    selectedCardIndex = -1;
    cardSelected = false;
    cardDragging = false;
    waitingForCardTarget = false;
}

void InputManager::sendCardPlayToServer(int cardIndex, const Position& targetPosition) {
    CardPlayData cardPlayData(cardIndex, targetPosition.x, targetPosition.y);
    sf::Packet cardPlayPacket;
    cardPlayPacket << MessageType::CardPlayToServer << cardPlayData;
    
    if (socket.send(cardPlayPacket) == sf::Socket::Done) {
        std::cout << "Card play sent to server: card " << cardIndex 
                  << " at (" << targetPosition.x << ", " << targetPosition.y << ")" << std::endl;
    } else {
        std::cerr << "Failed to send card play to server." << std::endl;
    }
}

void InputManager::resetCardSelection() {
    selectedCardIndex = -1;
    cardSelected = false;
    waitingForCardTarget = false;
    cardDragging = false;
    std::cout << "Card selection reset." << std::endl;
}

void InputManager::handleKeyPressed(const sf::Event& event) {
    // Space key no longer needed - actions auto-end turns
    // Keep method for future key handling if needed
    (void)event; // Suppress unused parameter warning
}

// sendPhaseAdvanceToServer method removed - no longer needed since actions auto-end turns

} // namespace BayouBonanza 