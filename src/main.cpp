#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "CardCollection.h"
#include "CardFactory.h"
#include "DeckEditorScreen.h"
#include "GameBoard.h"
#include "GameDebug.h"
#include "GameOverDetector.h"
#include "GameRendering.h"
#include "GameState.h"
#include "GameUi.h"
#include "GraphicsManager.h"
#include "HandRenderer.h"
#include "InfluenceSystem.h"
#include "InputManager.h"
#include "Menu.h"
#include "NetworkProtocol.h"
#include "PieceData.h"
#include "PieceDefinitionManager.h"
#include "PieceFactory.h"
#include "Square.h"
// The actual sf::Packet operators are now defined with their respective classes.

using namespace BayouBonanza;











int main()
{
    // Create the main window with a default size - GraphicsManager will handle scaling
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Bayou Bonanza");
    window.setFramerateLimit(60);

    // Initialize graphics manager for resolution scaling
    GraphicsManager graphicsManager(window);
    sf::Font globalFont;
    PieceDefinitionManager globalPieceDefManager;
    std::unique_ptr<PieceFactory> globalPieceFactory;
    std::map<std::string, sf::Texture> pieceTextures;
    CardCollection myCollection;
    Deck myDeck;
    PlayerSide myPlayerSide = PlayerSide::NEUTRAL;
    bool gameHasStarted = false;
    std::string uiMessage = "Connecting...";
    bool gameStartReceived = false;
    sf::Packet gameStartPacketData;
    GameState gameState;
    GameOverDetector gameOverDetector;
    int myCurrentRating = 0;
    std::string myUsername;
    std::string opponentUsername;
    int opponentRating = 0;
    bool returnToMenuRequested = false;
    GameUiState gameUi;


    // Load font early for login screen and UI
    if (!globalFont.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        std::cerr << "Error loading font from assets/fonts/Roboto-Regular.ttf" << std::endl;
        return -1;
    }

    // Initialize global PieceFactory for deserialization
    if (!globalPieceDefManager.loadDefinitions("assets/data/cards.json")) {
        std::cerr << "FATAL: Could not load piece definitions from assets/data/cards.json" << std::endl;
        return -1;
    }
    globalPieceFactory = std::make_unique<PieceFactory>(globalPieceDefManager);

    // Initialize CardFactory for card operations
    CardFactory::initialize();
    gameUi = createGameUiState(globalFont);
    updateUiMessage(gameUi, uiMessage);

    // Load piece textures
    for (const auto& typeName : globalPieceDefManager.getAllPieceTypeNames()) {
        const PieceStats* stats = globalPieceDefManager.getPieceStats(typeName);
        if (!stats) continue;
        if (!stats->spritePath.empty()) {
            sf::Texture tex;
            if (tex.loadFromFile(std::string("assets/") + stats->spritePath)) {
                pieceTextures[typeName] = tex;
            }
        }
    }

    // Set the global PieceFactory for Square deserialization
    Square::setGlobalPieceFactory(globalPieceFactory.get());

    // Display login screen for username input
    std::string username = runLoginScreen(window, graphicsManager, globalFont);
    if (username.empty()) {
        return 0; // Window closed before entering username
    }
    
    // Store username for menu display
    myUsername = username;

    // Network Socket
    sf::TcpSocket socket;
    const unsigned short PORT = 50000;
    const std::string SERVER_IP = "127.0.0.1"; // localhost

    std::cout << "Attempting to connect to server " << SERVER_IP << ":" << PORT << std::endl;
    if (socket.connect(SERVER_IP, PORT, sf::seconds(5)) != sf::Socket::Done) {
        std::cerr << "Error: Could not connect to the server." << std::endl;
        uiMessage = "Failed to connect to server.";
        // No return -1 yet, let the window open to display the message
    } else {
        std::cout << "Connected to server!" << std::endl;
        
        // Send UserLogin message
        sf::Packet loginPacket;
        loginPacket << MessageType::UserLogin << username;
        if (socket.send(loginPacket) != sf::Socket::Done) {
            std::cerr << "Error: Failed to send login packet." << std::endl;
            uiMessage = "Failed to send login info.";
            // Consider closing socket or handling error more robustly
        } else {
            std::cout << "Login packet sent with username: " << username << std::endl;
            uiMessage = "Login sent! Waiting for assignment...";
        }
    }

    updateUiMessage(gameUi, uiMessage);
    // Check immediately for ongoing game messages before entering menu
    socket.setBlocking(false); // Keep non-blocking to avoid freezing
    sf::Clock resumeClock;
    while (resumeClock.getElapsedTime().asSeconds() < 3.f && !gameStartReceived && window.isOpen()) {
        // Process window events to prevent freezing
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return 0;
            } else if (event.type == sf::Event::Resized) {
                graphicsManager.updateView();
            }
        }
        
        // Check for network messages
        sf::Packet receivedPacket;
        sf::Socket::Status status = socket.receive(receivedPacket);
        if (status == sf::Socket::Done) {
            MessageType messageType;
            if (receivedPacket >> messageType) {
                switch (messageType) {
                    case MessageType::PlayerAssignment: {
                        uint8_t side_uint8;
                        if (receivedPacket >> side_uint8) {
                            myPlayerSide = static_cast<PlayerSide>(side_uint8);
                        }
                        break;
                    }
                    case MessageType::CardCollectionData: {
                        std::string data;
                        if (receivedPacket >> data) {
                            myCollection.deserialize(data);
                        }
                        break;
                    }
                    case MessageType::DeckData: {
                        std::string data;
                        if (receivedPacket >> data) {
                            myDeck.deserialize(data);
                        }
                        break;
                    }
                    case MessageType::GameStart:
                        {
                            // Create a new packet with the GameStart message type and data
                            gameStartPacketData.clear();
                            gameStartPacketData << MessageType::GameStart;
                            
                            // Extract and re-add the data to the new packet
                            std::string p1_username, p2_username;
                            int p1_rating, p2_rating;
                            GameState tempGameState;
                            
                            if (receivedPacket >> p1_username >> p1_rating >> p2_username >> p2_rating >> tempGameState) {
                                gameStartPacketData << p1_username << p1_rating << p2_username << p2_rating << tempGameState;
                                gameStartReceived = true;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        } else if (status == sf::Socket::NotReady) {
            // No data available, continue loop
        } else {
            break; // Error or disconnection
        }
        
        // Render a "connecting" screen during the wait
        graphicsManager.applyView();
        window.clear(sf::Color(10, 50, 20));
        
        sf::Text connectingText;
        connectingText.setFont(globalFont);
        connectingText.setString("Connecting...");
        connectingText.setCharacterSize(32);
        connectingText.setFillColor(sf::Color::White);
        
        sf::FloatRect bounds = connectingText.getLocalBounds();
        connectingText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        connectingText.setPosition(GraphicsManager::BASE_WIDTH / 2.f, GraphicsManager::BASE_HEIGHT / 2.f);
        
        window.draw(connectingText);
        window.display();
        
        // Small sleep to prevent busy waiting
        sf::sleep(sf::milliseconds(16)); // ~60 FPS
    }

    // Show main menu only if no game to resume
    MainMenuOption menuChoice = MainMenuOption::NONE;
    if (!gameStartReceived) {
        do {
            menuChoice = runMainMenu(window, graphicsManager, socket, myCollection, myDeck, myPlayerSide, myUsername, myCurrentRating, gameStartPacketData, gameStartReceived, globalFont);
            if (menuChoice == MainMenuOption::DECK_EDITOR) {
                runDeckEditor(window, graphicsManager, socket, myCollection, myDeck, globalFont, globalPieceDefManager);
            } else if (menuChoice == MainMenuOption::PLAY_AI) {
                showPlaceholderScreen(window, graphicsManager, "Play vs AI Coming Soon", globalFont);
            }
        } while (window.isOpen() && menuChoice != MainMenuOption::PLAY_HUMAN);
        if (!window.isOpen()) {
            return 0;
        }
    }


    
    // Register win condition callback
    GameOverDetector::registerWinConditionCallback(
        [&](PlayerSide winner, const std::string& /*description*/) {
            prepareWinScreen(gameUi, winner, myPlayerSide, opponentUsername, myCurrentRating, opponentRating, globalFont);
        });

    // Initialize input manager with graphics manager
    InputManager inputManager(window, socket, gameState, gameHasStarted, myPlayerSide, graphicsManager);

    sf::Clock frameClock;

    // Main game loop
    while (window.isOpen()) {
        float dt = frameClock.restart().asSeconds();
        
        // Process stored GameStart data if received in main menu
        if (gameStartReceived && !gameHasStarted) {
            std::cout << "Processing stored GameStart packet data from main menu" << std::endl;
            
            // Deserialize the stored packet data
            std::string p1_username, p2_username;
            int p1_rating, p2_rating;
            MessageType dummyType;  // Add this to skip the MessageType in the stored packet
            
            if (gameStartPacketData >> dummyType >> p1_username >> p1_rating >> p2_username >> p2_rating >> gameState) {
                gameHasStarted = true;
                printBoardState(gameState, myPlayerSide); // Keep this for debugging

                // Determine local and remote player data
                std::string localName = (myPlayerSide == PlayerSide::PLAYER_ONE) ? p1_username : p2_username;
                int localRatingValue = (myPlayerSide == PlayerSide::PLAYER_ONE) ? p1_rating : p2_rating;
                std::string remoteName = (myPlayerSide == PlayerSide::PLAYER_ONE) ? p2_username : p1_username;
                int remoteRatingValue = (myPlayerSide == PlayerSide::PLAYER_ONE) ? p2_rating : p1_rating;

                myCurrentRating = localRatingValue;
                opponentRating = remoteRatingValue;
                myUsername = localName;
                opponentUsername = remoteName;

                updatePlayerInfo(gameUi, myPlayerSide, localName, localRatingValue, remoteName, remoteRatingValue);
                updateLocalSteam(gameUi, gameState.getSteam(myPlayerSide));

                uiMessage = "Game started!";
                updateUiMessage(gameUi, uiMessage);

                std::cout << "Successfully processed stored GameStart data" << std::endl;
            } else {
                std::cerr << "Error deserializing stored GameStart packet data" << std::endl;
            }
            
            // Clear the flag so we don't process it again
            gameStartReceived = false;
        }
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                // Update graphics manager when window is resized
                graphicsManager.updateView();
            } else if (gameUi.showWinMessage) {
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2f clickPos = graphicsManager.screenToGame({event.mouseButton.x, event.mouseButton.y});
                    if (gameUi.menuButton.getGlobalBounds().contains(clickPos)) {
                        returnToMenuRequested = true;
                        gameUi.showWinMessage = false;
                    }
                } else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                    returnToMenuRequested = true;
                    gameUi.showWinMessage = false;
                }
            } else if (gameHasStarted && myPlayerSide == gameState.getActivePlayer()) {
                // Only handle input if it's the player's turn
                inputManager.handleEvent(event);
            } else if (gameHasStarted) {
                // Debug: Log when input is blocked

            }
        }

        // --- Network Receive ---
        sf::Packet receivedPacket;
        sf::Socket::Status status = socket.receive(receivedPacket);
        if (status == sf::Socket::Done) {
            std::cout << "Received packet from server" << std::endl;
            MessageType messageType;
            if (receivedPacket >> messageType) {
                std::cout << "Message type: " << static_cast<int>(messageType) << std::endl;
                switch (messageType) {
                    case MessageType::PlayerAssignment:
                        uint8_t side_uint8;
                        if (receivedPacket >> side_uint8) {
                            myPlayerSide = static_cast<PlayerSide>(side_uint8);
                            uiMessage = "Assigned player side: Player ";
                            uiMessage += (myPlayerSide == PlayerSide::PLAYER_ONE ? "One" : "Two");
                            std::cout << uiMessage << std::endl;
                            updateUiMessage(gameUi, uiMessage);
                        } else { std::cerr << "Error deserializing PlayerAssignment data." << std::endl; }
                        break;
                    case MessageType::CardCollectionData:
                        {
                            std::string data;
                            if (receivedPacket >> data) {
                                myCollection.deserialize(data);
                                std::cout << "Collection received with " << myCollection.size() << " cards" << std::endl;
                            }
                        }
                        break;
                    case MessageType::DeckData:
                        {
                            std::string data;
                            if (receivedPacket >> data) {
                                myDeck.deserialize(data);
                                std::cout << "Deck received with " << myDeck.size() << " cards" << std::endl;
                            }
                        }
                        break;
                    case MessageType::WaitingForOpponent:
                        uiMessage = "Waiting for opponent...";
                        updateUiMessage(gameUi, uiMessage);
                        std::cout << uiMessage << std::endl;
                        break;
                    case MessageType::GameStart:
                        {
                            std::string p1_username, p2_username;
                            int p1_rating, p2_rating;

                            if (receivedPacket >> p1_username >> p1_rating >> p2_username >> p2_rating >> gameState) {
                                gameHasStarted = true;
                                printBoardState(gameState, myPlayerSide); // Keep this for debugging

                                // Determine local and remote player data
                                std::string localName = (myPlayerSide == PlayerSide::PLAYER_ONE) ? p1_username : p2_username;
                                int localRatingValue = (myPlayerSide == PlayerSide::PLAYER_ONE) ? p1_rating : p2_rating;
                                std::string remoteName = (myPlayerSide == PlayerSide::PLAYER_ONE) ? p2_username : p1_username;
                                int remoteRatingValue = (myPlayerSide == PlayerSide::PLAYER_ONE) ? p2_rating : p1_rating;

                                myCurrentRating = localRatingValue;
                                opponentRating = remoteRatingValue;
                                myUsername = localName;
                                opponentUsername = remoteName;

                                updatePlayerInfo(gameUi, myPlayerSide, localName, localRatingValue, remoteName, remoteRatingValue);
                                updateLocalSteam(gameUi, gameState.getSteam(myPlayerSide));

                                uiMessage = "Game started!";
                                updateUiMessage(gameUi, uiMessage);

                                // If the game already started in a game-over state (unlikely but safe), trigger end screen
                                if (gameState.getGamePhase() == GamePhase::GAME_OVER && !gameUi.showWinMessage) {
                                    PlayerSide winner = gameOverDetector.getWinner(gameState);
                                    std::string desc = gameOverDetector.getWinConditionDescription(gameState);
                                    prepareWinScreen(gameUi, winner, myPlayerSide, opponentUsername, myCurrentRating, opponentRating, globalFont);
                                }

                            } else {
                                std::cerr << "Error deserializing GameStart data (with user info)." << std::endl;
                            }
                        }
                        break;
                    case MessageType::GameStateUpdate:
                        if (gameHasStarted) {
                            if (receivedPacket >> gameState) { // Deserialize the updated GameState
                                // uiMessage = (myPlayerSide == gameState.getActivePlayer() ? "Your turn" : "Opponent's turn");
                                std::cout << "GameState updated. Turn: " << gameState.getTurnNumber() << std::endl;

                                printBoardState(gameState, myPlayerSide);
                                
                                // Update local player steam display
                                updateLocalSteam(gameUi, gameState.getSteam(myPlayerSide));
                                
                                // If server reports game over, trigger the end screen once
                                if (gameState.getGamePhase() == GamePhase::GAME_OVER && !gameUi.showWinMessage) {
                                    PlayerSide winner = gameOverDetector.getWinner(gameState);
                                    std::string desc = gameOverDetector.getWinConditionDescription(gameState);
                                    prepareWinScreen(gameUi, winner, myPlayerSide, opponentUsername, myCurrentRating, opponentRating, globalFont);
                                }
                            } else { std::cerr << "Error deserializing GameStateUpdate." << std::endl; }
                        }
                        break;
                    case MessageType::MoveRejected: // Optional
                        uiMessage = "Move rejected by server.";
                        updateUiMessage(gameUi, uiMessage);
                        std::cout << uiMessage << std::endl;
                        
                        // Reset input manager state
                        inputManager.resetInputState();
                        break;
                    case MessageType::CardPlayRejected: // Optional
                        uiMessage = "Card play rejected by server.";
                        updateUiMessage(gameUi, uiMessage);
                        std::cout << uiMessage << std::endl;
                        
                        // Reset card selection state in input manager
                        inputManager.resetCardSelection();
                        break;
                    case MessageType::Error: // Example, server might send string
                        // std::string errorMessage;
                        // if (receivedPacket >> errorMessage) {
                        //     uiMessage = "Server error: " + errorMessage;
                        //     std::cerr << uiMessage << std::endl;
                        // } else { std::cerr << "Error deserializing Error message data." << std::endl; }
                        break;
                    default:
                        std::cout << "Received unhandled/unknown message type: " << static_cast<int>(messageType) << std::endl;
                }
            }
        } else if (status == sf::Socket::NotReady) {
            // No data received, non-blocking socket, this is normal
        } else if (status == sf::Socket::Disconnected) {
            uiMessage = "Connection to server lost.";
            updateUiMessage(gameUi, uiMessage);
            std::cerr << uiMessage << std::endl;
            window.close(); // Or handle reconnection
        } else if (status == sf::Socket::Error) {
            std::cerr << "Network error receiving data." << std::endl;
            // Potentially handle disconnection or error
        }
        // --- End Network Receive ---
        if (returnToMenuRequested) {
            break;
        }

        // Apply the game view for proper scaling
        graphicsManager.applyView();
        
        // Clear screen with a dark green background (bayou-like)
        window.clear(sf::Color(10, 50, 20));
        
        // Draw game elements here (based on the local gameState, now updated by server)
        
        // Update UI message
        if (gameHasStarted) {
            std::string phaseStr;
            switch (gameState.getGamePhase()) {
                case GamePhase::SETUP: phaseStr = "Setup"; break;
                case GamePhase::DRAW: phaseStr = "Drawing"; break;
                case GamePhase::PLAY: phaseStr = "Action"; break;
                case GamePhase::MOVE: phaseStr = "Action"; break; // Legacy support
                case GamePhase::GAME_OVER: phaseStr = "Game Over"; break;
                default: phaseStr = "Unknown"; break;
            }
            
            // Update turn message (without phase)
            uiMessage = (myPlayerSide == gameState.getActivePlayer() ? "Your turn (Player " : "Opponent's turn (Player ");
            uiMessage += (gameState.getActivePlayer() == PlayerSide::PLAYER_ONE ? "One" : "Two");
            uiMessage += ")";
            
            // Update phase message separately (no instructions needed since actions auto-end turns)
            std::string phaseMessage = phaseStr + " Phase";
            updatePhaseText(gameUi, phaseMessage);
        }
        updateUiMessage(gameUi, uiMessage);
        window.draw(gameUi.uiMessageText);
        
        // Draw phase text if game has started
        if (gameHasStarted) {
            window.draw(gameUi.phaseText);
        }

        // Draw username/rating info if game has started
        if (gameHasStarted) {
            window.draw(gameUi.localPlayerUsernameText);
            window.draw(gameUi.localPlayerRatingText);
            window.draw(gameUi.remotePlayerUsernameText);
            window.draw(gameUi.remotePlayerRatingText);
            window.draw(gameUi.localPlayerSteamText);
        }

        // --- Game Board Rendering ---
        const GameBoard& board = gameState.getBoard();
        auto boardParams = graphicsManager.getBoardRenderParams();

        sf::Color lightSquareColor(170, 210, 130);
        sf::Color darkSquareColor(100, 150, 80);

        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                sf::Vector2f squarePos = graphicsManager.boardToGame(x, y);
                sf::RectangleShape squareShape(sf::Vector2f(boardParams.squareSize, boardParams.squareSize));
                squareShape.setPosition(squarePos);

                // Alternate colors for chessboard pattern
                if ((x + y) % 2 == 0) {
                    squareShape.setFillColor(lightSquareColor);
                } else {
                    squareShape.setFillColor(darkSquareColor);
                }
                window.draw(squareShape);
            }
        }
        // --- End Game Board Rendering ---

        // --- Control Visualization ---
        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                const Square& square = board.getSquare(x, y);
                PlayerSide controller = InfluenceSystem::getControllingPlayer(square);
                
                if (controller != PlayerSide::NEUTRAL) {
                    sf::Vector2f squarePos = graphicsManager.boardToGame(x, y);
                    sf::RectangleShape controlIndicator(sf::Vector2f(boardParams.squareSize, boardParams.squareSize));
                    controlIndicator.setPosition(squarePos);
                    
                    if (controller == PlayerSide::PLAYER_ONE) {
                        // More visible blue tint for Player One control
                        controlIndicator.setFillColor(sf::Color(0, 100, 255, 120)); // Blue with higher alpha
                    } else if (controller == PlayerSide::PLAYER_TWO) {
                        // More visible red tint for Player Two control
                        controlIndicator.setFillColor(sf::Color(255, 50, 0, 120)); // Red with higher alpha
                    }
                    
                    window.draw(controlIndicator);
                }
            }
        }
        // --- End Control Visualization ---

        // --- Move Highlighting ---
        std::vector<Position> highlightSquares;
        const Piece* highlightPiece = nullptr;
        if (inputManager.isPieceSelected() && inputManager.getSelectedPiece()) {
            highlightPiece = inputManager.getSelectedPiece();
        } else {
            sf::Vector2i mouseScreen = sf::Mouse::getPosition(window);
            sf::Vector2f mouseGame = graphicsManager.screenToGame(mouseScreen);
            sf::Vector2i boardCoords = graphicsManager.gameToBoard(mouseGame);
            if (boardCoords.x >= 0 && boardCoords.y >= 0) {
                const Square& hovered = board.getSquare(boardCoords.x, boardCoords.y);
                if (!hovered.isEmpty()) {
                    highlightPiece = hovered.getPiece();
                }
            }
        }

        if (highlightPiece) {
            highlightSquares = highlightPiece->getValidMoves(board);
        }

        for (const Position& pos : highlightSquares) {
            sf::Vector2f squarePos = graphicsManager.boardToGame(pos.x, pos.y);
            sf::RectangleShape highlightRect(sf::Vector2f(boardParams.squareSize, boardParams.squareSize));
            highlightRect.setPosition(squarePos);
            highlightRect.setFillColor(sf::Color(255, 255, 0, 120));
            window.draw(highlightRect);
        }
        // --- End Move Highlighting ---

        // --- Piece Rendering ---
        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                // Skip drawing the selected piece here if it's being dragged
                if (inputManager.isPieceSelected() && 
                    x == inputManager.getOriginalSquareCoords().x && 
                    y == inputManager.getOriginalSquareCoords().y) {
                    continue; 
                }

                const Square& square = board.getSquare(x, y);
                if (!square.isEmpty()) {
                    Piece* piece = square.getPiece();
                    sf::Vector2f piecePos = graphicsManager.boardToGame(x, y);
                    
                    auto texIt = pieceTextures.find(piece->getTypeName());
                    if (texIt != pieceTextures.end()) {
                        sf::Sprite spr(texIt->second);
                        if (piece->isStunned()) {
                            spr.setColor(sf::Color(128, 128, 128));
                        }
                        spr.setPosition(piecePos);
                        
                        // Scale based on actual texture dimensions
                        sf::Vector2u textureSize = texIt->second.getSize();
                        float scaleX = boardParams.squareSize / static_cast<float>(textureSize.x);
                        float scaleY = boardParams.squareSize / static_cast<float>(textureSize.y);
                        if (piece->getSide() == PlayerSide::PLAYER_TWO) {
                            spr.setOrigin(static_cast<float>(textureSize.x), 0.f);
                            spr.setScale(-scaleX, scaleY);
                        } else {
                            spr.setScale(scaleX, scaleY);
                        }
                        window.draw(spr);
                    }

                    // Render health bar
                    renderHealthBar(window, piece,
                                    piecePos.x,
                                    piecePos.y,
                                    boardParams.squareSize);

                    // Render attack value
                    renderAttackValue(window, piece,
                                     piecePos.x,
                                     piecePos.y,
                                     boardParams.squareSize, globalFont);
                }
            }
        }

        // Draw the selected piece at the mouse cursor position if it's being dragged
        if (inputManager.isPieceSelected() && inputManager.getSelectedPiece()) {
            sf::Vector2f mouseOffset = inputManager.getMouseOffset();
            sf::Vector2f currentMousePosition = inputManager.getCurrentMousePosition();
            float draggedPieceX = currentMousePosition.x - mouseOffset.x;
            float draggedPieceY = currentMousePosition.y - mouseOffset.y;

            Piece* draggedPiece = inputManager.getSelectedPiece();
            auto texIt = pieceTextures.find(draggedPiece->getTypeName());
            if (texIt != pieceTextures.end()) {
                sf::Sprite spr(texIt->second);
                if (draggedPiece->isStunned()) {
                    spr.setColor(sf::Color(128, 128, 128));
                }
                spr.setPosition(draggedPieceX, draggedPieceY);

                // Scale based on actual texture dimensions
                sf::Vector2u textureSize = texIt->second.getSize();
                float scaleX = boardParams.squareSize / static_cast<float>(textureSize.x);
                float scaleY = boardParams.squareSize / static_cast<float>(textureSize.y);
                if (draggedPiece->getSide() == PlayerSide::PLAYER_TWO) {
                    spr.setOrigin(static_cast<float>(textureSize.x), 0.f);
                    spr.setScale(-scaleX, scaleY);
                } else {
                    spr.setScale(scaleX, scaleY);
                }
                window.draw(spr);
            }
            
            // Render health bar for the dragged piece
            renderHealthBar(window, draggedPiece, draggedPieceX, draggedPieceY, boardParams.squareSize);

            // Render attack value for the dragged piece
            renderAttackValue(window, draggedPiece, draggedPieceX, draggedPieceY, boardParams.squareSize, globalFont);
        }
        // --- End Piece Rendering ---
        
        // --- Card Hand Rendering ---
        if (gameHasStarted) {
            int selectedCard = inputManager.isCardSelected() ? inputManager.getSelectedCardIndex() : -1;
            renderPlayerHand(window, gameState, myPlayerSide, graphicsManager, globalFont, selectedCard);

            // Draw dragged card on top of everything
            if (inputManager.isCardSelected() && inputManager.isWaitingForCardTarget()) {
                const Hand& hand = gameState.getHand(myPlayerSide);
                const Card* card = hand.getCard(inputManager.getSelectedCardIndex());
                if (card) {
                    sf::Vector2f mouseOffset = inputManager.getMouseOffset();
                    sf::Vector2f currentMousePosition = inputManager.getCurrentMousePosition();
                    float cardX = currentMousePosition.x - mouseOffset.x;
                    float cardY = currentMousePosition.y - mouseOffset.y;

                    float cardWidth = 120.f;
                    float cardHeight = 120.f;

                    sf::RectangleShape cardRect(sf::Vector2f(cardWidth, cardHeight));
                    cardRect.setPosition(cardX, cardY);
                    cardRect.setFillColor(sf::Color(60, 80, 60, 200));
                    cardRect.setOutlineColor(sf::Color::Yellow);
                    cardRect.setOutlineThickness(2.f);
                    window.draw(cardRect);

                    sf::Text nameText;
                    nameText.setFont(globalFont);
                    nameText.setCharacterSize(14);
                    nameText.setFillColor(sf::Color::White);
                    nameText.setString(card->getName());
                    sf::FloatRect nameBounds = nameText.getLocalBounds();
                    nameText.setPosition(cardX + (cardWidth - nameBounds.width) / 2.f, cardY + 10.f);
                    window.draw(nameText);

                    sf::Text costText;
                    costText.setFont(globalFont);
                    costText.setCharacterSize(16);
                    costText.setFillColor(sf::Color::Cyan);
                    costText.setString("Steam: " + std::to_string(card->getSteamCost()));
                    sf::FloatRect costBounds = costText.getLocalBounds();
                    costText.setPosition(cardX + (cardWidth - costBounds.width) / 2.f, cardY + cardHeight - 25.f);
                    window.draw(costText);
                }
            }
        }
        // --- End Card Hand Rendering ---
        
        // --- Win Message Display ---
        drawWinScreen(window, gameUi, globalFont);
        // --- End Win Message Display ---
        
        // Update the window
        window.display();
    }
    
    if (returnToMenuRequested && window.isOpen()) {
        runMainMenu(window, graphicsManager, socket, myCollection, myDeck, myPlayerSide, myUsername, myCurrentRating, gameStartPacketData, gameStartReceived, globalFont);
    }
    return 0;
}

