#include "InputManager.h"
#include "GameBoard.h"
#include <iostream>

namespace BayouBonanza {

InputManager::InputManager(sf::RenderWindow& window, 
                          sf::TcpSocket& socket,
                          GameState& gameState,
                          bool& gameHasStarted,
                          PlayerSide& myPlayerSide)
    : window(window)
    , socket(socket)
    , gameState(gameState)
    , gameHasStarted(gameHasStarted)
    , myPlayerSide(myPlayerSide)
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

std::shared_ptr<Piece> InputManager::getSelectedPiece() const {
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

void InputManager::calculateBoardLayout(float& boardSize, float& squareSize, 
                                       float& boardStartX, float& boardStartY) const {
    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    
    boardSize = std::min(windowWidth, windowHeight) * 0.8f;
    squareSize = boardSize / GameBoard::BOARD_SIZE;
    boardStartX = (windowWidth - boardSize) / 2.0f;
    boardStartY = (windowHeight - boardSize) / 2.0f;
}

sf::Vector2i InputManager::screenToBoard(const sf::Vector2f& screenPos, 
                                        float boardStartX, float boardStartY, 
                                        float squareSize) const {
    int boardX = static_cast<int>((screenPos.x - boardStartX) / squareSize);
    int boardY = static_cast<int>((screenPos.y - boardStartY) / squareSize);
    
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
    
    float boardSize, squareSize, boardStartX, boardStartY;
    calculateBoardLayout(boardSize, squareSize, boardStartX, boardStartY);
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    sf::Vector2i boardCoords = screenToBoard(mousePosF, boardStartX, boardStartY, squareSize);
    
    if (boardCoords.x >= 0 && boardCoords.y >= 0) {
        const Square& square = gameState.getBoard().getSquare(boardCoords.x, boardCoords.y);
        if (!square.isEmpty() && square.getPiece()->getSide() == gameState.getActivePlayer()) {
            selectPiece(boardCoords.x, boardCoords.y, mousePos, boardStartX, boardStartY, squareSize);
        }
    }
}

void InputManager::handleMouseMoved(const sf::Event& event) {
    if (pieceSelected) {
        currentMousePosition = sf::Vector2f(
            static_cast<float>(sf::Mouse::getPosition(window).x), 
            static_cast<float>(sf::Mouse::getPosition(window).y)
        );
    }
}

void InputManager::handleMouseButtonReleased(const sf::Event& event) {
    if (event.mouseButton.button != sf::Mouse::Left || !pieceSelected) {
        return;
    }
    
    float boardSize, squareSize, boardStartX, boardStartY;
    calculateBoardLayout(boardSize, squareSize, boardStartX, boardStartY);
    
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
    sf::Vector2i targetCoords = screenToBoard(mousePosF, boardStartX, boardStartY, squareSize);
    
    if (targetCoords.x >= 0 && targetCoords.y >= 0) {
        attemptMove(targetCoords.x, targetCoords.y);
    } else {
        std::cout << "Invalid move: Target square is off-board." << std::endl;
    }
    
    // Deselect piece regardless of move outcome
    resetInputState();
}

void InputManager::selectPiece(int boardX, int boardY, const sf::Vector2i& mousePos,
                              float boardStartX, float boardStartY, float squareSize) {
    const Square& square = gameState.getBoard().getSquare(boardX, boardY);
    selectedPiece = square.getPiece();
    originalSquareCoords = sf::Vector2i(boardX, boardY);
    pieceSelected = true;
    
    // Calculate offset: mouse_pos - piece_top_left_pos
    float pieceScreenX = boardStartX + boardX * squareSize;
    float pieceScreenY = boardStartY + boardY * squareSize;
    mouseOffset = sf::Vector2f(mousePos.x - pieceScreenX, mousePos.y - pieceScreenY);
    currentMousePosition = sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
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
        Move gameMove(selectedPiece, startPosition, targetPosition);
        
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
        std::cerr << "Error sending move to server." << std::endl;
        // Potentially handle disconnection or error
    }
}

} // namespace BayouBonanza 