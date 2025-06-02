#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Network.hpp> // Added for networking
#include <iostream>
#include <string>
#include <memory>
#include <cctype> // Required for std::tolower

#include "GameBoard.h"
#include "GameState.h"
#include "Square.h"       // For Square::setGlobalPieceFactory
#include "Move.h"      // For Move and its sf::Packet operators
#include "NetworkProtocol.h" // For MessageType enum and operators
#include "PlayerSide.h"  // For PlayerSide enum
#include "InputManager.h" // New input manager
#include "GraphicsManager.h" // New graphics manager
#include "GameInitializer.h"
#include "PieceFactory.h"
#include "PieceDefinitionManager.h"
#include "TurnManager.h"
// #include "King.h" // Removed - using data-driven approach with PieceFactory


// The actual sf::Packet operators are now defined with their respective classes.

using namespace BayouBonanza;

// Client-specific global variables
PlayerSide myPlayerSide = PlayerSide::NEUTRAL; // Default, will be assigned by server
bool gameHasStarted = false;
std::string uiMessage = "Connecting..."; // For displaying messages like "Waiting for opponent"
sf::Text uiMessageText;
sf::Font globalFont; // Loaded once

// Global PieceFactory for deserialization
PieceDefinitionManager globalPieceDefManager;
std::unique_ptr<PieceFactory> globalPieceFactory;

// UI Elements for Usernames/Ratings
sf::Text localPlayerUsernameText;
sf::Text localPlayerRatingText;
sf::Text remotePlayerUsernameText;
sf::Text remotePlayerRatingText;

// Function to recreate pieces after deserialization without resetting game state
void recreatePiecesAfterDeserialization(GameState& gameState) {
    // The issue is that Square deserialization loses pieces due to PieceFactory access
    // Instead of resetting the entire game state, we'll recreate pieces using a standard layout
    // This is a temporary workaround until we fix the Square deserialization properly
    
    GameBoard& board = gameState.getBoard();
    
    // Clear the board first
    board.resetBoard();
    
    // Recreate the standard starting position using PieceFactory
    if (globalPieceFactory) {
        // Player 1 pieces (bottom of board)
        // Back row
        board.getSquare(0, 7).setPiece(globalPieceFactory->createPiece("Rook", PlayerSide::PLAYER_ONE));
        board.getSquare(1, 7).setPiece(globalPieceFactory->createPiece("Knight", PlayerSide::PLAYER_ONE));
        board.getSquare(2, 7).setPiece(globalPieceFactory->createPiece("Bishop", PlayerSide::PLAYER_ONE));
        board.getSquare(3, 7).setPiece(globalPieceFactory->createPiece("Queen", PlayerSide::PLAYER_ONE));
        board.getSquare(4, 7).setPiece(globalPieceFactory->createPiece("King", PlayerSide::PLAYER_ONE));
        board.getSquare(5, 7).setPiece(globalPieceFactory->createPiece("Bishop", PlayerSide::PLAYER_ONE));
        board.getSquare(6, 7).setPiece(globalPieceFactory->createPiece("Knight", PlayerSide::PLAYER_ONE));
        board.getSquare(7, 7).setPiece(globalPieceFactory->createPiece("Rook", PlayerSide::PLAYER_ONE));
        
        // Pawn row
        for (int x = 0; x < 8; x++) {
            board.getSquare(x, 6).setPiece(globalPieceFactory->createPiece("Pawn", PlayerSide::PLAYER_ONE));
        }
        
        // Player 2 pieces (top of board)
        // Back row
        board.getSquare(0, 0).setPiece(globalPieceFactory->createPiece("Rook", PlayerSide::PLAYER_TWO));
        board.getSquare(1, 0).setPiece(globalPieceFactory->createPiece("Knight", PlayerSide::PLAYER_TWO));
        board.getSquare(2, 0).setPiece(globalPieceFactory->createPiece("Bishop", PlayerSide::PLAYER_TWO));
        board.getSquare(3, 0).setPiece(globalPieceFactory->createPiece("Queen", PlayerSide::PLAYER_TWO));
        board.getSquare(4, 0).setPiece(globalPieceFactory->createPiece("King", PlayerSide::PLAYER_TWO));
        board.getSquare(5, 0).setPiece(globalPieceFactory->createPiece("Bishop", PlayerSide::PLAYER_TWO));
        board.getSquare(6, 0).setPiece(globalPieceFactory->createPiece("Knight", PlayerSide::PLAYER_TWO));
        board.getSquare(7, 0).setPiece(globalPieceFactory->createPiece("Rook", PlayerSide::PLAYER_TWO));
        
        // Pawn row
        for (int x = 0; x < 8; x++) {
            board.getSquare(x, 1).setPiece(globalPieceFactory->createPiece("Pawn", PlayerSide::PLAYER_TWO));
        }
        
        // Set piece positions
        for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
                const Square& square = board.getSquare(x, y);
                if (!square.isEmpty()) {
                    Piece* piece = square.getPiece();
                    piece->setPosition(Position(x, y));
                }
            }
        }
    }
}

// Function to print board state to console for debugging
void printBoardState(const GameState& gameState) {
    const GameBoard& board = gameState.getBoard();
    
    std::cout << "\n  Board State (Turn " << gameState.getTurnNumber() << "):\n";
    std::cout << "  Active Player: " << (gameState.getActivePlayer() == PlayerSide::PLAYER_ONE ? "Player 1" : "Player 2") << "\n\n";
    
    std::cout << "    0 1 2 3 4 5 6 7\n";
    std::cout << "  ----------------\n";
    
    for (int y = 0; y < GameBoard::BOARD_SIZE; y++) {
        std::cout << y << " | ";
        
        for (int x = 0; x < GameBoard::BOARD_SIZE; x++) {
            const Square& square = board.getSquare(x, y);
            
            char symbol = '.';
            if (!square.isEmpty()) {
                Piece* piece = square.getPiece();
                std::string symbol_str = piece->getSymbol();
                symbol = symbol_str.empty() ? '.' : symbol_str[0];
                
                if (piece->getSide() == PlayerSide::PLAYER_TWO) {
                    symbol = std::tolower(symbol);
                }
            }
            
            // Print piece symbol and control indicator
            std::cout << symbol << ' ';
        }
        
        std::cout << "|\n";
    }
    
    std::cout << "  ----------------\n";
    std::cout << "  Player 1 Steam: " << gameState.getSteam(PlayerSide::PLAYER_ONE) << "\n";
    std::cout << "  Player 2 Steam: " << gameState.getSteam(PlayerSide::PLAYER_TWO) << "\n";
}

int main()
{
    // Create the main window with a default size - GraphicsManager will handle scaling
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Bayou Bonanza");
    window.setFramerateLimit(60);

    // Initialize graphics manager for resolution scaling
    GraphicsManager graphicsManager(window);

    // Initialize global PieceFactory for deserialization
    if (!globalPieceDefManager.loadDefinitions("assets/data/pieces.json")) {
        std::cerr << "FATAL: Could not load piece definitions from assets/data/pieces.json" << std::endl;
        return -1;
    }
    globalPieceFactory = std::make_unique<PieceFactory>(globalPieceDefManager);
    
    // Set the global PieceFactory for Square deserialization
    Square::setGlobalPieceFactory(globalPieceFactory.get());

    // Get username input
    std::string username;
    std::cout << "Enter your username: ";
    std::cin >> username;

    // Basic validation: ensure username is not empty
    while (username.empty()) {
        std::cout << "Username cannot be empty. Please enter your username: ";
        std::cin >> username;
    }

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
    socket.setBlocking(false); // Use non-blocking mode for game loop

    // Load font (use globalFont)
    if (!globalFont.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        std::cerr << "Error loading font from assets/fonts/Roboto-Regular.ttf\n";
        return -1;
    }

    // Initialize UI text elements
    uiMessageText.setFont(globalFont);
    uiMessageText.setCharacterSize(24);
    uiMessageText.setFillColor(sf::Color::White);
    uiMessageText.setPosition(10.f, 10.f);
    uiMessageText.setString(uiMessage);

    localPlayerUsernameText.setFont(globalFont);
    localPlayerUsernameText.setCharacterSize(18);
    localPlayerUsernameText.setFillColor(sf::Color::Cyan);

    localPlayerRatingText.setFont(globalFont);
    localPlayerRatingText.setCharacterSize(16);
    localPlayerRatingText.setFillColor(sf::Color::White);

    remotePlayerUsernameText.setFont(globalFont);
    remotePlayerUsernameText.setCharacterSize(18);
    remotePlayerUsernameText.setFillColor(sf::Color::Yellow);

    remotePlayerRatingText.setFont(globalFont);
    remotePlayerRatingText.setCharacterSize(16);
    remotePlayerRatingText.setFillColor(sf::Color::White);

    // Initialize game state
    GameState gameState;

    // Initialize input manager with graphics manager
    InputManager inputManager(window, socket, gameState, gameHasStarted, myPlayerSide, graphicsManager);

    // Main game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::Resized) {
                // Update graphics manager when window is resized
                graphicsManager.updateView();
            } else if (gameHasStarted && myPlayerSide == gameState.getActivePlayer()) {
                // Only handle input if it's the player's turn
                inputManager.handleEvent(event);
            }
        }

        // --- Network Receive ---
        sf::Packet receivedPacket;
        sf::Socket::Status status = socket.receive(receivedPacket);
        if (status == sf::Socket::Done) {
            MessageType messageType;
            if (receivedPacket >> messageType) {
                switch (messageType) {
                    case MessageType::PlayerAssignment:
                        uint8_t side_uint8;
                        if (receivedPacket >> side_uint8) {
                            myPlayerSide = static_cast<PlayerSide>(side_uint8);
                            uiMessage = "Assigned player side: Player ";
                            uiMessage += (myPlayerSide == PlayerSide::PLAYER_ONE ? "One" : "Two");
                            std::cout << uiMessage << std::endl;
                        } else { std::cerr << "Error deserializing PlayerAssignment data." << std::endl; }
                        break;
                    case MessageType::WaitingForOpponent:
                        uiMessage = "Waiting for opponent to connect...";
                        std::cout << uiMessage << std::endl;
                        break;
                    case MessageType::GameStart:
                        {
                            std::string p1_username, p2_username;
                            int p1_rating, p2_rating;

                            if (receivedPacket >> p1_username >> p1_rating >> p2_username >> p2_rating >> gameState) {
                                gameHasStarted = true;
                                printBoardState(gameState); // Keep this for debugging

                                // Determine local and remote player data
                                if (myPlayerSide == PlayerSide::PLAYER_ONE) {
                                    localPlayerUsernameText.setString("You: " + p1_username);
                                    localPlayerRatingText.setString("Rating: " + std::to_string(p1_rating));
                                    remotePlayerUsernameText.setString("Opponent: " + p2_username);
                                    remotePlayerRatingText.setString("Rating: " + std::to_string(p2_rating));
                                } else if (myPlayerSide == PlayerSide::PLAYER_TWO) {
                                    localPlayerUsernameText.setString("You: " + p2_username);
                                    localPlayerRatingText.setString("Rating: " + std::to_string(p2_rating));
                                    remotePlayerUsernameText.setString("Opponent: " + p1_username);
                                    remotePlayerRatingText.setString("Rating: " + std::to_string(p1_rating));
                                }
                                uiMessage = "Game started!"; 
                                std::cout << uiMessage << " P1: " << p1_username << " (" << p1_rating << "), P2: " << p2_username << " (" << p2_rating << ")" << std::endl;

                                // Set positions for username/rating texts (using base resolution coordinates)
                                localPlayerUsernameText.setPosition(10.f, uiMessageText.getPosition().y + 30.f);
                                localPlayerRatingText.setPosition(10.f, localPlayerUsernameText.getPosition().y + 20.f);
                                
                                // Position remote player text on the right side
                                float remoteTextWidthEstimate = 200.f; 
                                remotePlayerUsernameText.setPosition(GraphicsManager::BASE_WIDTH - remoteTextWidthEstimate - 10.f, uiMessageText.getPosition().y + 30.f);
                                remotePlayerRatingText.setPosition(GraphicsManager::BASE_WIDTH - remoteTextWidthEstimate - 10.f, remotePlayerUsernameText.getPosition().y + 20.f);

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
                                printBoardState(gameState);
                            } else { std::cerr << "Error deserializing GameStateUpdate." << std::endl; }
                        }
                        break;
                    case MessageType::MoveRejected: // Optional
                        uiMessage = "Move rejected by server.";
                        std::cout << uiMessage << std::endl;
                        
                        // Reset input manager state
                        inputManager.resetInputState();
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
            std::cerr << uiMessage << std::endl;
            window.close(); // Or handle reconnection
        } else if (status == sf::Socket::Error) {
            std::cerr << "Network error receiving data." << std::endl;
            // Potentially handle disconnection or error
        }
        // --- End Network Receive ---
        
        // Apply the game view for proper scaling
        graphicsManager.applyView();
        
        // Clear screen with a dark green background (bayou-like)
        window.clear(sf::Color(10, 50, 20));
        
        // Draw game elements here (based on the local gameState, now updated by server)
        
        // Update UI message
        if (gameHasStarted) {
            uiMessage = (myPlayerSide == gameState.getActivePlayer() ? "Your turn (Player " : "Opponent's turn (Player ");
            uiMessage += (gameState.getActivePlayer() == PlayerSide::PLAYER_ONE ? "One)" : "Two)");
        }
        uiMessageText.setString(uiMessage);
        window.draw(uiMessageText);

        // Draw username/rating info if game has started
        if (gameHasStarted) {
            window.draw(localPlayerUsernameText);
            window.draw(localPlayerRatingText);
            window.draw(remotePlayerUsernameText);
            window.draw(remotePlayerRatingText);
        }

        // --- Game Board Rendering ---
        const GameBoard& board = gameState.getBoard();
        auto boardParams = graphicsManager.getBoardRenderParams();

        sf::Color lightSquareColor(170, 210, 130);
        sf::Color darkSquareColor(100, 150, 80);

        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                sf::RectangleShape squareShape(sf::Vector2f(boardParams.squareSize, boardParams.squareSize));
                squareShape.setPosition(boardParams.boardStartX + x * boardParams.squareSize, 
                                       boardParams.boardStartY + y * boardParams.squareSize);

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
                    std::string symbol = piece->getSymbol();

                    sf::Text pieceText;
                    pieceText.setFont(globalFont); // Use globalFont
                    pieceText.setString(symbol);
                    pieceText.setCharacterSize(static_cast<unsigned int>(boardParams.squareSize * 0.6f)); // Adjust size as needed

                    if (piece->getSide() == PlayerSide::PLAYER_ONE) {
                        pieceText.setFillColor(sf::Color::Blue);
                    } else {
                        pieceText.setFillColor(sf::Color::Red);
                    }

                    sf::FloatRect textBounds = pieceText.getLocalBounds();
                    pieceText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                                        textBounds.top + textBounds.height / 2.0f);
                    pieceText.setPosition(boardParams.boardStartX + x * boardParams.squareSize + boardParams.squareSize / 2.0f,
                                          boardParams.boardStartY + y * boardParams.squareSize + boardParams.squareSize / 2.0f);

                    window.draw(pieceText);
                }
            }
        }

        // Draw the selected piece at the mouse cursor position if it's being dragged
        if (inputManager.isPieceSelected() && inputManager.getSelectedPiece()) {
            sf::RectangleShape selectedPieceShape(sf::Vector2f(boardParams.squareSize, boardParams.squareSize));
            // Adjust position using mouseOffset so the piece is grabbed correctly
            sf::Vector2f mouseOffset = inputManager.getMouseOffset();
            sf::Vector2f currentMousePosition = inputManager.getCurrentMousePosition();
            selectedPieceShape.setPosition(currentMousePosition.x - mouseOffset.x, 
                                         currentMousePosition.y - mouseOffset.y);
            
            if (inputManager.getSelectedPiece()->getSide() == PlayerSide::PLAYER_ONE) {
                selectedPieceShape.setFillColor(sf::Color(0, 0, 255, 180)); // Blue with some transparency
            } else {
                selectedPieceShape.setFillColor(sf::Color(255, 0, 0, 180)); // Red with some transparency
            }
            window.draw(selectedPieceShape);
        }
        // --- End Piece Rendering ---
        
        // Update the window
        window.display();
    }
    
    return 0;
}
