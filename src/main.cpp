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
    // Create the main window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Bayou Bonanza");
    
    // Set the framerate limit
    window.setFramerateLimit(60);

    // Initialize global PieceFactory for deserialization
    if (!globalPieceDefManager.loadDefinitions("assets/data/pieces.json")) {
        std::cerr << "FATAL: Could not load piece definitions from assets/data/pieces.json" << std::endl;
        return -1;
    }
    globalPieceFactory = std::make_unique<PieceFactory>(globalPieceDefManager);
    
    // Set the global PieceFactory for Square deserialization
    Square::setGlobalPieceFactory(globalPieceFactory.get());

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
        uiMessage = "Connected! Waiting for assignment...";
    }
    socket.setBlocking(false); // Use non-blocking mode

    // Load font (use globalFont)
    if (!globalFont.loadFromFile("assets/fonts/Roboto-Regular.ttf")) {
        std::cerr << "Error loading font from assets/fonts/Roboto-Regular.ttf\n";
        return -1; 
    }
    uiMessageText.setFont(globalFont);
    uiMessageText.setCharacterSize(24);
    uiMessageText.setFillColor(sf::Color::White);
    uiMessageText.setPosition(10.f, 10.f);
    
    // Create game components
    GameState gameState; // Client's local copy of the game state, updated by server
    // GameRules gameRules; // Removed
    // GameInitializer initializer; // Removed
    // TurnManager turnManager(gameState, gameRules); // Removed
    // GameOverDetector gameOverDetector; // Removed
    
    // Initialize a new game - This will now be handled by server sending initial state
    // initializer.initializeNewGame(gameState); // Removed
    
    // Print initial board state (client will wait for server's first state update)
    std::cout << "BayouBonanza - Client" << std::endl; // Corrected project name
    std::cout << "----------------------------------------" << std::endl;
    // printBoardState(gameState); // Removed - client waits for server's state

    // Placeholder for player's assigned side - to be received from server
    // PlayerSide myPlayerSide = PlayerSide::PLAYER_ONE; // Default or unassigned

    // Create input manager
    InputManager inputManager(window, socket, gameState, gameHasStarted, myPlayerSide);
    
    // Main game loop
    while (window.isOpen())
    {
        // Process events
        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window if requested
            if (event.type == sf::Event::Closed) {
                window.close();
                continue;
            }

            // Let the input manager handle input events
            if (!inputManager.handleEvent(event)) {
                // Event was not handled by input manager
                // Handle other events here if needed
            }
        }

        // --- Network Receive ---
        sf::Packet receivedPacket;
        sf::Socket::Status status = socket.receive(receivedPacket);
        if (status == sf::Socket::Done) {
            MessageType messageType;
            if (!(receivedPacket >> messageType)) { // Deserialize the type first
                if (receivedPacket.getDataSize() > 0) { // Check if there was data to even attempt deserializing type
                     std::cerr << "Error deserializing message type." << std::endl;
                }
                // If getDataSize is 0, it might just be a keep-alive or empty packet, ignore.
            } else {
                std::cout << "Received message type: " << static_cast<int>(messageType) << std::endl;
                switch (messageType) {
                    case MessageType::PlayerAssignment:
                        sf::Uint8 side_uint8;
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
                        uiMessage = "Game is starting!";
                        std::cout << uiMessage << std::endl;
                        if (receivedPacket >> gameState) { // Deserialize the initial GameState
                            gameHasStarted = true;
                            printBoardState(gameState);
                        } else { std::cerr << "Error deserializing GameStart state." << std::endl; }
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

        // --- Game Board Rendering ---
        const GameBoard& board = gameState.getBoard();
        float windowWidth = static_cast<float>(window.getSize().x);
        float windowHeight = static_cast<float>(window.getSize().y);
        
        float boardSize = std::min(windowWidth, windowHeight) * 0.8f; // Use 80% of the smaller dimension
        float squareSize = boardSize / GameBoard::BOARD_SIZE;
        float boardStartX = (windowWidth - boardSize) / 2.0f;
        float boardStartY = (windowHeight - boardSize) / 2.0f;

        sf::Color lightSquareColor(170, 210, 130);
        sf::Color darkSquareColor(100, 150, 80);

        for (int y = 0; y < GameBoard::BOARD_SIZE; ++y) {
            for (int x = 0; x < GameBoard::BOARD_SIZE; ++x) {
                sf::RectangleShape squareShape(sf::Vector2f(squareSize, squareSize));
                squareShape.setPosition(boardStartX + x * squareSize, boardStartY + y * squareSize);

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
                    pieceText.setCharacterSize(static_cast<unsigned int>(squareSize * 0.6f)); // Adjust size as needed

                    if (piece->getSide() == PlayerSide::PLAYER_ONE) {
                        pieceText.setFillColor(sf::Color::Blue);
                    } else {
                        pieceText.setFillColor(sf::Color::Red);
                    }

                    sf::FloatRect textBounds = pieceText.getLocalBounds();
                    pieceText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                                        textBounds.top + textBounds.height / 2.0f);
                    pieceText.setPosition(boardStartX + x * squareSize + squareSize / 2.0f,
                                          boardStartY + y * squareSize + squareSize / 2.0f);

                    window.draw(pieceText);
                }
            }
        }

        // Draw the selected piece at the mouse cursor position if it's being dragged
        if (inputManager.isPieceSelected() && inputManager.getSelectedPiece()) {
            sf::RectangleShape selectedPieceShape(sf::Vector2f(squareSize, squareSize));
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
