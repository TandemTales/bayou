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
}

sf::Vector2i InputManager::gamePosToBoard(const sf::Vector2f& gamePos) const {
    auto boardParams = graphicsManager.getBoardRenderParams();
    
    int boardX = static_cast<int>((gamePos.x - boardParams.boardStartX) / boardParams.squareSize);
    int boardY = static_cast<int>((gamePos.y - boardParams.boardStartY) / boardParams.squareSize);
    
    if (boardX >= 0 && boardX < GameBoard::BOARD_SIZE && 
        boardY >= 0 && boardY < GameBoard::BOARD_SIZE) {
        return sf::Vector2i(boardX, boardY);
    }
    
    return sf::Vector2i(-1, -1); // Invalid position
}

void InputManager::handleMouseButtonPressed(const sf::Event& event) {
    if (event.mouseButton.button != sf::Mouse::Left) {
        return;
    }
    
    // Convert screen coordinates to game coordinates
    sf::Vector2i screenMousePos(event.mouseButton.x, event.mouseButton.y);
    sf::Vector2f gameMousePos = graphicsManager.screenToGame(screenMousePos);
    sf::Vector2i boardCoords = gamePosToBoard(gameMousePos);
    
    if (boardCoords.x >= 0 && boardCoords.y >= 0) {
        const Square& square = gameState.getBoard().getSquare(boardCoords.x, boardCoords.y);
        if (!square.isEmpty() && square.getPiece()->getSide() == gameState.getActivePlayer()) {
            selectPiece(boardCoords.x, boardCoords.y, gameMousePos);
        }
    }
}

void InputManager::handleMouseMoved(const sf::Event& event) {
    if (pieceSelected) {
        // Convert current screen mouse position to game coordinates
        sf::Vector2i screenMousePos = sf::Mouse::getPosition(window);
        currentMousePosition = graphicsManager.screenToGame(screenMousePos);
    }
}

void InputManager::handleMouseButtonReleased(const sf::Event& event) {
    if (event.mouseButton.button != sf::Mouse::Left || !pieceSelected) {
        return;
    }
    
    // Convert screen coordinates to game coordinates
    sf::Vector2i screenMousePos(event.mouseButton.x, event.mouseButton.y);
    sf::Vector2f gameMousePos = graphicsManager.screenToGame(screenMousePos);
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
    auto boardParams = graphicsManager.getBoardRenderParams();
    float pieceGameX = boardParams.boardStartX + boardX * boardParams.squareSize;
    float pieceGameY = boardParams.boardStartY + boardY * boardParams.squareSize;
    mouseOffset = sf::Vector2f(gameMousePos.x - pieceGameX, gameMousePos.y - pieceGameY);
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

} // namespace BayouBonanza 