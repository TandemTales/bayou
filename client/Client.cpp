#include "Client.h"
#include "NetworkProtocol.h" 
#include "SFML/Window/Event.hpp" 
#include "PieceFactory.h" 
#include "GameState.h"    
#include "GameBoard.h"    
#include "Square.h"       
#include "Piece.h"        // For Position
#include <iostream>
#include <string>
#include <memory> 
#include <cmath> // For floor

// Using directives for convenience
using BayouBonanza::PlayerSide;
using BayouBonanza::Network::MessageType;
using BayouBonanza::Network::PieceType;
using BayouBonanza::Network::NetworkMove; 
using BayouBonanza::Network::operator>>; 
using BayouBonanza::Network::operator<<; // For sending NetworkMove
using BayouBonanza::Position;


std::string Client::pieceTypeToString(BayouBonanza::Network::PieceType type) {
    switch (type) {
        case PieceType::KING: return "King";
        case PieceType::QUEEN: return "Queen";
        case PieceType::ROOK: return "Rook";
        case PieceType::BISHOP: return "Bishop";
        case PieceType::KNIGHT: return "Knight";
        case PieceType::PAWN: return "Pawn";
        default: return ""; 
    }
}

BayouBonanza::Network::PieceType Client::getNetworkPieceTypeFromString(const std::string& typeName) {
    if (typeName == "King") return PieceType::KING;
    if (typeName == "Queen") return PieceType::QUEEN;
    if (typeName == "Rook") return PieceType::ROOK;
    if (typeName == "Bishop") return PieceType::BISHOP;
    if (typeName == "Knight") return PieceType::KNIGHT;
    if (typeName == "Pawn") return PieceType::PAWN;
    return PieceType::NONE;
}

Client::Client(const std::string& serverIp, unsigned short port)
    : connected(false), myPlayerSide(PlayerSide::NEUTRAL), isMyTurn(false), gameActive(true), selectedSquare(std::nullopt) { 
    std::cout << "Initializing client..." << std::endl;
    gameState = std::make_unique<BayouBonanza::GameState>(); 
    setupWindow();
    if (!font.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        std::cerr << "Error loading font assets/fonts/Roboto-Regular.ttf" << std::endl;
        gameActive = false; 
        return;
    }
    connectToServer(serverIp, port);
}

void Client::setupWindow() {
    window.create(sf::VideoMode(800, 600), "Bayou Bonanza Client"); 
    window.setFramerateLimit(60); 
    std::cout << "Window created." << std::endl;
}

void Client::connectToServer(const std::string& serverIp, unsigned short port) {
    std::cout << "Connecting to server " << serverIp << ":" << port << "..." << std::endl;
    sf::Socket::Status status = socket.connect(serverIp, port, sf::seconds(5));
    if (status != sf::Socket::Done) {
        std::cerr << "Error: Could not connect to the server. Status: " << status << std::endl;
        connected = false;
        gameActive = false; 
        return;
    }
    connected = true;
    socket.setBlocking(false); 
    std::cout << "Successfully connected to the server." << std::endl;
}

void Client::loadPieceTextures() {
    std::cout << "Texture loading skipped (using text symbols for pieces)." << std::endl;
}

BayouBonanza::Position Client::screenToBoardCoords(int mouseX, int mouseY) const {
    if (!gameState) return {-1, -1}; // Invalid position if no game state

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    
    float boardAreaSize = std::min(windowWidth, windowHeight) * 0.8f; 
    float squareSize = boardAreaSize / BayouBonanza::GameBoard::BOARD_SIZE;
    float boardStartX = (windowWidth - boardAreaSize) / 2.0f;
    float boardStartY = (windowHeight - boardAreaSize) / 2.0f;

    if (mouseX < boardStartX || mouseX > boardStartX + boardAreaSize ||
        mouseY < boardStartY || mouseY > boardStartY + boardAreaSize) {
        return {-1, -1}; // Click is outside the board
    }

    int col = static_cast<int>(std::floor((mouseX - boardStartX) / squareSize));
    int row = static_cast<int>(std::floor((mouseY - boardStartY) / squareSize));
    
    // Ensure col and row are within board bounds (0-7)
    col = std::max(0, std::min(col, BayouBonanza::GameBoard::BOARD_SIZE - 1));
    row = std::max(0, std::min(row, BayouBonanza::GameBoard::BOARD_SIZE - 1));

    return {col, row};
}


void Client::run() {
    if (!connected || !gameActive) { 
        std::cout << "Client not connected or failed initialization. Exiting run loop." << std::endl;
        return;
    }
    std::cout << "Starting client run loop..." << std::endl;
    while (window.isOpen() && gameActive) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                gameActive = false; 
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    handlePlayerInput(event.mouseButton);
                }
            }
        }

        sf::Packet packet;
        sf::Socket::Status status = socket.receive(packet);
        switch (status) {
            case sf::Socket::Done:
                processServerPacket(packet);
                break;
            case sf::Socket::NotReady:
                break;
            case sf::Socket::Disconnected:
            case sf::Socket::Error: 
                std::cerr << (status == sf::Socket::Disconnected ? "Disconnected from server." : "Network error.") << std::endl;
                gameActive = false; 
                break;
            default: break; 
        }
        render();
    }
    std::cout << "Client run loop ended." << std::endl;
}

void Client::handlePlayerInput(const sf::Event::MouseButtonEvent& event) {
    if (!gameState || !gameActive) return;

    Position clickedPos = screenToBoardCoords(event.x, event.y);
    if (clickedPos.x == -1 || clickedPos.y == -1) { // Click was outside the board
        selectedSquare = std::nullopt; // Deselect if clicked outside
        std::cout << "Clicked outside board." << std::endl;
        return;
    }

    std::cout << "Clicked board square: (" << clickedPos.x << ", " << clickedPos.y << ")" << std::endl;

    if (!isMyTurn) {
        std::cout << "Not your turn. Ignoring click." << std::endl;
        selectedSquare = std::nullopt; // Clear selection if it's not their turn
        return;
    }

    // It is my turn
    if (!selectedSquare.has_value()) {
        // No piece currently selected, try to select one
        const auto& square = gameState->getBoard().getSquare(clickedPos.x, clickedPos.y);
        if (!square.isEmpty() && square.getPiece()->getSide() == myPlayerSide) {
            selectedSquare = clickedPos;
            std::cout << "Selected piece at (" << selectedSquare->x << ", " << selectedSquare->y << ")" << std::endl;
            // Optional: Calculate and store possible moves here
        } else {
            std::cout << "Clicked empty square or opponent's piece. No piece selected." << std::endl;
            selectedSquare = std::nullopt;
        }
    } else {
        // A piece is already selected (selectedSquare.has_value() is true)
        // This click is the target square for the move
        Position fromPos = selectedSquare.value();
        Position toPos = clickedPos;

        std::cout << "Attempting to move from (" << fromPos.x << ", " << fromPos.y
                  << ") to (" << toPos.x << ", " << toPos.y << ")" << std::endl;

        NetworkMove netMove(fromPos, toPos);
        sf::Packet actionPacket;
        actionPacket << static_cast<sf::Uint8>(MessageType::PlayerAction) << netMove;
        
        if (socket.send(actionPacket) == sf::Socket::Done) {
            std::cout << "PlayerAction packet sent." << std::endl;
            // Optimistically, we could assume the turn is over, but server confirmation is better.
            // Server will send GameStateUpdate or ActionInvalid.
            // isMyTurn = false; // Don't do this; wait for server
        } else {
            std::cerr << "Error sending PlayerAction packet." << std::endl;
        }
        selectedSquare = std::nullopt; // Reset selection after attempting a move
    }
}


void Client::processServerPacket(sf::Packet& packet) {
    sf::Uint8 messageTypeInt;
    if (!(packet >> messageTypeInt)) {
        std::cerr << "Error: Could not deserialize message type." << std::endl;
        return;
    }
    MessageType type = static_cast<MessageType>(messageTypeInt);

    switch (type) {
        case MessageType::AssignPlayerIdentity: {
            if (!(packet >> myPlayerSide)) { 
                std::cerr << "Error deserializing AssignPlayerIdentity." << std::endl;
                break;
            }
            std::cout << "Received AssignPlayerIdentity. I am Player " 
                      << (myPlayerSide == PlayerSide::PLAYER_ONE ? "One" : "Two") << std::endl;
            break;
        }
        case MessageType::GameStart: // Fallthrough to GameStateUpdate as logic is similar
        case MessageType::GameStateUpdate: {
            bool isGameStart = (type == MessageType::GameStart);
            std::cout << "Received " << (isGameStart ? "GameStart" : "GameStateUpdate") << " message." << std::endl;
            if (!gameState) {
                gameState = std::make_unique<BayouBonanza::GameState>();
            }
            
            PlayerSide activePlayer;
            sf::Int32 turnNumber = gameState->getTurnNumber(); 

            if (isGameStart) { // GameStart: activePlayer, board, steam
                if (!(packet >> activePlayer)) { std::cerr << "GS: Error activePlayer" << std::endl; break; }
            } else { // GameStateUpdate: activePlayer, turnNumber, board, steam
                if (!(packet >> activePlayer >> turnNumber)) { std::cerr << "GSU: Error activePlayer/turn" << std::endl; break;}
            }
            
            gameState->setActivePlayer(activePlayer);
            if (!isGameStart) { // Only GameStateUpdate sends turn number explicitly this way
                 // This loop is a bit awkward. If GameState had setTurnNumber(turnNumber), it would be better.
                 // Or ensure GameState's default turn number is 0 or 1 and increment aligns.
                 while(gameState->getTurnNumber() < turnNumber) { gameState->incrementTurnNumber(); }
                 if(gameState->getTurnNumber() > turnNumber && turnNumber == 1) { /* might be reset */ } // More complex reset logic might be needed
            }


            isMyTurn = (myPlayerSide == activePlayer);
            if (isMyTurn) {
                std::cout << "It is now my turn." << std::endl;
            } else {
                std::cout << "It is now opponent's turn." << std::endl;
                selectedSquare = std::nullopt; // Clear selection if it's not our turn anymore
            }


            auto& board = gameState->getBoard(); 
            for (int y = 0; y < BayouBonanza::GameBoard::BOARD_SIZE; ++y) {
                for (int x = 0; x < BayouBonanza::GameBoard::BOARD_SIZE; ++x) {
                    sf::Uint8 pieceTypeInt;
                    if (!(packet >> pieceTypeInt)) { std::cerr << "GS/U: Error pieceType" << std::endl; return; }
                    PieceType netPieceType = static_cast<PieceType>(pieceTypeInt);
                    
                    if (netPieceType == PieceType::NONE) {
                        board.getSquare(x, y).setPiece(nullptr);
                    } else {
                        PlayerSide pieceOwner;
                        sf::Int8 health;
                        if (!(packet >> pieceOwner >> health)) { std::cerr << "GS/U: Error pieceOwner/health" << std::endl; return; }
                        
                        std::string typeNameStr = pieceTypeToString(netPieceType);
                        if (!typeNameStr.empty()) {
                            std::shared_ptr<BayouBonanza::Piece> piece = BayouBonanza::PieceFactory::createPiece(typeNameStr, pieceOwner);
                            if (piece) {
                                piece->setHealth(health);
                                piece->setPosition({x,y}); 
                                board.getSquare(x,y).setPiece(piece);
                            } else {
                                std::cerr << "Failed to create piece of type " << typeNameStr << std::endl;
                                board.getSquare(x,y).setPiece(nullptr);
                            }
                        } else {
                             std::cerr << "Unknown piece type received: " << pieceTypeInt << std::endl;
                             board.getSquare(x,y).setPiece(nullptr);
                        }
                    }
                }
            }

            sf::Int32 steamP1, steamP2;
            if (!(packet >> steamP1 >> steamP2)) { std::cerr << "GS/U: Error steam" << std::endl; break; }
            gameState->setSteam(PlayerSide::PLAYER_ONE, steamP1);
            gameState->setSteam(PlayerSide::PLAYER_TWO, steamP2);
            
            std::cout << (isGameStart ? "GameStart" : "GameStateUpdate") << " processed. My turn: " << (isMyTurn ? "Yes" : "No") 
                      << ". Turn: " << gameState->getTurnNumber() << std::endl;
            break;
        }
        case MessageType::TurnChange: { 
            PlayerSide newActivePlayer;
            sf::Int32 currentTurnNumber; 
            if (!(packet >> newActivePlayer >> currentTurnNumber)) {
                std::cerr << "Error deserializing TurnChange." << std::endl;
                break;
            }
            if (gameState) {
                gameState->setActivePlayer(newActivePlayer);
                while(gameState->getTurnNumber() < currentTurnNumber) { gameState->incrementTurnNumber(); }
            }
            isMyTurn = (myPlayerSide == newActivePlayer);
             if (isMyTurn) {
                std::cout << "It is now my turn (via TurnChange)." << std::endl;
            } else {
                std::cout << "It is now opponent's turn (via TurnChange)." << std::endl;
                selectedSquare = std::nullopt; 
            }
            std::cout << "Received TurnChange. Active player: " 
                      << (newActivePlayer == PlayerSide::PLAYER_ONE ? "One" : "Two")
                      << ". My turn: " << (isMyTurn ? "Yes" : "No") << ". Turn: " << currentTurnNumber << std::endl;
            break;
        }
        case MessageType::ActionInvalid: {
            NetworkMove moveAttempt; 
            std::string reason;
            if (!(packet >> moveAttempt >> reason)) {
                std::cerr << "Error deserializing ActionInvalid." << std::endl;
                break;
            }
            std::cout << "Received ActionInvalid for move from (" << moveAttempt.from.x << "," << moveAttempt.from.y
                      << ") to (" << moveAttempt.to.x << "," << moveAttempt.to.y << "). Reason: " << reason << std::endl;
            // Clear selection if my move was invalid.
            selectedSquare = std::nullopt; 
            // TODO: Display this to the user in the UI
            break;
        }
        case MessageType::GameOver: {
            PlayerSide winner;
            sf::Uint8 gameResultInt;
            if (!(packet >> winner >> gameResultInt)) {
                std::cerr << "Error deserializing GameOver." << std::endl;
                break;
            }
            BayouBonanza::GameResult gameResult = static_cast<BayouBonanza::GameResult>(gameResultInt);
            std::cout << "Received GameOver. Winner: " << (winner == PlayerSide::PLAYER_ONE ? "P1" : (winner == PlayerSide::PLAYER_TWO ? "P2" : "Neutral"))
                      << ". Result: " << static_cast<int>(gameResult) << std::endl;
            gameActive = false; 
            if(gameState) gameState->setGameResult(gameResult); 
            break;
        }
        case MessageType::ServerFull: {
            std::cout << "Received ServerFull. Disconnecting." << std::endl;
            gameActive = false;
            connected = false;
            socket.disconnect();
            break;
        }
        case MessageType::HeartbeatRequest: {
            sf::Packet hbResponse;
            hbResponse << static_cast<sf::Uint8>(MessageType::HeartbeatResponse);
            if(socket.send(hbResponse) != sf::Socket::Done){
                std::cerr << "Error sending HeartbeatResponse." << std::endl;
            }
            break;
        }
        default:
            std::cerr << "Received unknown message type: " << messageTypeInt << std::endl;
            break;
    }
}

void Client::drawBoard() {
    if (!gameState) return;
    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    float boardAreaSize = std::min(windowWidth, windowHeight) * 0.8f; 
    float squareSize = boardAreaSize / BayouBonanza::GameBoard::BOARD_SIZE;
    float boardStartX = (windowWidth - boardAreaSize) / 2.0f;
    float boardStartY = (windowHeight - boardAreaSize) / 2.0f;
    sf::Color lightSquareColor(170, 210, 130); 
    sf::Color darkSquareColor(100, 150, 80);  

    for (int y = 0; y < BayouBonanza::GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < BayouBonanza::GameBoard::BOARD_SIZE; ++x) {
            sf::RectangleShape squareShape(sf::Vector2f(squareSize, squareSize));
            squareShape.setPosition(boardStartX + x * squareSize, boardStartY + y * squareSize);
            squareShape.setFillColor(((x + y) % 2 == 0) ? lightSquareColor : darkSquareColor);
            window.draw(squareShape);
        }
    }
}

void Client::drawHighlights() {
    if (!gameState || !selectedSquare.has_value()) return;

    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    float boardAreaSize = std::min(windowWidth, windowHeight) * 0.8f;
    float squareSize = boardAreaSize / BayouBonanza::GameBoard::BOARD_SIZE;
    float boardStartX = (windowWidth - boardAreaSize) / 2.0f;
    float boardStartY = (windowHeight - boardAreaSize) / 2.0f;

    // Highlight the selected square
    sf::RectangleShape highlightShape(sf::Vector2f(squareSize, squareSize));
    highlightShape.setPosition(boardStartX + selectedSquare->x * squareSize, boardStartY + selectedSquare->y * squareSize);
    highlightShape.setFillColor(sf::Color(255, 255, 0, 100)); // Yellow, semi-transparent
    window.draw(highlightShape);

    // TODO: Highlight possible moves if possibleMovesForSelectedPiece is implemented
}


void Client::drawPieces() {
    if (!gameState) return;
    float windowWidth = static_cast<float>(window.getSize().x);
    float windowHeight = static_cast<float>(window.getSize().y);
    float boardAreaSize = std::min(windowWidth, windowHeight) * 0.8f;
    float squareSize = boardAreaSize / BayouBonanza::GameBoard::BOARD_SIZE;
    float boardStartX = (windowWidth - boardAreaSize) / 2.0f;
    float boardStartY = (windowHeight - boardAreaSize) / 2.0f;

    const BayouBonanza::GameBoard& board = gameState->getBoard();
    for (int y = 0; y < BayouBonanza::GameBoard::BOARD_SIZE; ++y) {
        for (int x = 0; x < BayouBonanza::GameBoard::BOARD_SIZE; ++x) {
            const BayouBonanza::Square& square = board.getSquare(x, y);
            if (!square.isEmpty()) {
                std::shared_ptr<BayouBonanza::Piece> piece = square.getPiece();
                std::string symbol = piece->getSymbol(); 
                sf::Text pieceText;
                pieceText.setFont(font);
                pieceText.setString(symbol);
                pieceText.setCharacterSize(static_cast<unsigned int>(squareSize * 0.7f)); 
                if (piece->getSide() == PlayerSide::PLAYER_ONE) {
                    pieceText.setFillColor(sf::Color::White); 
                } else {
                    pieceText.setFillColor(sf::Color(60,60,60));  
                }
                sf::FloatRect textBounds = pieceText.getLocalBounds();
                pieceText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                                    textBounds.top + textBounds.height / 2.0f);
                pieceText.setPosition(boardStartX + x * squareSize + squareSize / 2.0f,
                                      boardStartY + y * squareSize + squareSize / 2.0f);
                window.draw(pieceText);
                
                sf::Text healthText;
                healthText.setFont(font);
                healthText.setString(std::to_string(piece->getHealth()));
                healthText.setCharacterSize(static_cast<unsigned int>(squareSize * 0.2f));
                healthText.setFillColor(sf::Color::Yellow); 
                sf::FloatRect healthBounds = healthText.getLocalBounds();
                healthText.setOrigin(healthBounds.left + healthBounds.width / 2.0f, healthBounds.top + healthBounds.height / 2.0f);
                healthText.setPosition(boardStartX + x * squareSize + squareSize / 2.0f,
                                       boardStartY + y * squareSize + squareSize * 0.8f); 
                window.draw(healthText);
            }
        }
    }
}

void Client::drawUI() {
    if (!gameState && connected) {
         sf::Text loadingText("Connected, waiting for game state...", font, 20);
         loadingText.setFillColor(sf::Color::White);
         loadingText.setPosition(10, window.getSize().y - 30.f);
         window.draw(loadingText);
         return;
    }
    if (!gameState) return;

    sf::Text uiText;
    uiText.setFont(font);
    uiText.setCharacterSize(16);
    uiText.setFillColor(sf::Color::White);

    std::string sideStr = "Unknown";
    if (myPlayerSide == PlayerSide::PLAYER_ONE) sideStr = "One (White)";
    else if (myPlayerSide == PlayerSide::PLAYER_TWO) sideStr = "Two (Black)";
    uiText.setString("You are Player: " + sideStr);
    uiText.setPosition(10, 10);
    window.draw(uiText);

    std::string activePlayerStr = (gameState->getActivePlayer() == PlayerSide::PLAYER_ONE ? "One (White)" : "Two (Black)");
    uiText.setString("Current Turn: " + activePlayerStr);
    uiText.setPosition(10, 30);
    window.draw(uiText);
    
    uiText.setString(isMyTurn ? "It's YOUR turn!" : "Waiting for opponent...");
    uiText.setPosition(10, 50);
    uiText.setFillColor(isMyTurn ? sf::Color::Green : sf::Color::Yellow);
    window.draw(uiText);
    uiText.setFillColor(sf::Color::White);

    uiText.setString("P1 Steam: " + std::to_string(gameState->getSteam(PlayerSide::PLAYER_ONE)));
    uiText.setPosition(10, 70);
    window.draw(uiText);
    uiText.setString("P2 Steam: " + std::to_string(gameState->getSteam(PlayerSide::PLAYER_TWO)));
    uiText.setPosition(10, 90);
    window.draw(uiText);
    
    uiText.setString("Turn: " + std::to_string(gameState->getTurnNumber()));
    uiText.setPosition(10, 110);
    window.draw(uiText);

    if (!gameActive) { 
        std::string endMessage = "Game Over!";
        if (!connected && gameState && gameState->getGameResult() == BayouBonanza::GameResult::IN_PROGRESS) { 
            endMessage = "Disconnected from server.";
        } else if (gameState && gameState->getGameResult() != BayouBonanza::GameResult::IN_PROGRESS) {
             if (gameState->getGameResult() == BayouBonanza::GameResult::PLAYER_ONE_WIN) endMessage = "Player One Wins!";
             else if (gameState->getGameResult() == BayouBonanza::GameResult::PLAYER_TWO_WIN) endMessage = "Player Two Wins!";
             else if (gameState->getGameResult() == BayouBonanza::GameResult::DRAW) endMessage = "It's a Draw!";
        }

        sf::Text gameOverText(endMessage, font, 40);
        gameOverText.setFillColor(sf::Color::Red);
        sf::FloatRect textRect = gameOverText.getLocalBounds();
        gameOverText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        gameOverText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
        window.draw(gameOverText);
    }
}

void Client::render() {
    window.clear(sf::Color(30, 30, 30)); 

    if (!connected && !gameActive) { 
        sf::Text errorText;
        errorText.setFont(font);
        errorText.setString("Failed to connect to server.");
        errorText.setCharacterSize(24);
        errorText.setFillColor(sf::Color::Red);
        sf::FloatRect textRect = errorText.getLocalBounds();
        errorText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        errorText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
        window.draw(errorText);
    } else {
        drawBoard();
        drawHighlights(); // Draw selection highlight
        drawPieces(); 
        drawUI();     
    }

    window.display();
}

Client::~Client() {
    std::cout << "Client shutting down." << std::endl;
    if (socket.getLocalPort() != 0) { 
        socket.disconnect();
    }
}
```
