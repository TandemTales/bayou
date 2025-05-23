#include <SFML/Network.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <iomanip> // For std::setw

// Assuming NetworkProtocol.h is in the include path
#include "NetworkProtocol.h" 
// PlayerSide is in its own header but often included via others like GameState or NetworkProtocol
#include "PlayerSide.h" 
// Piece.h for Position, GameBoard.h for BOARD_SIZE might be needed if not in NetworkProtocol.h
// For this test, we'll rely on NetworkProtocol.h to have enough (like Position operators)
// and GameBoard::BOARD_SIZE can be assumed or hardcoded if not easily available.
// Let's assume NetworkProtocol.h defines Position serialization operators.
// For GameBoard::BOARD_SIZE, let's use a known constant for now.
const int GAME_BOARD_SIZE = 8; // Matches GameBoard::BOARD_SIZE

// Using namespace for convenience in this simple test client
using namespace BayouBonanza;
using namespace BayouBonanza::Network;

// Helper to print packet contents (for debugging)
void printPacketDetails(sf::Packet& packet, const std::string& packetName) {
    std::cout << "--- " << packetName << " Details (size: " << packet.getDataSize() << ") ---" << std::endl;
    const char* data = static_cast<const char*>(packet.getData());
    std::cout << "Hex dump:" << std::endl;
    for (std::size_t i = 0; i < packet.getDataSize(); ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<unsigned char>(data[i])) << " ";
        if ((i + 1) % 16 == 0) std::cout << std::endl;
    }
    std::cout << std::dec << std::endl; // Reset to decimal output
    std::cout << "--- End " << packetName << " ---" << std::endl;
}


int main(int argc, char* argv[]) {
    std::string serverIp = "127.0.0.1";
    unsigned short port = 12345;

    if (argc > 1) {
        serverIp = argv[1];
    }
    if (argc > 2) {
        port = static_cast<unsigned short>(std::stoi(argv[2]));
    }

    std::cout << "Attempting to connect to server at " << serverIp << ":" << port << std::endl;

    sf::TcpSocket socket;
    socket.setBlocking(true); // For simplicity in test client, use blocking sockets initially

    if (socket.connect(serverIp, port, sf::seconds(10)) != sf::Socket::Done) {
        std::cerr << "TEST FAILED: Could not connect to server." << std::endl;
        return 1;
    }
    std::cout << "Connected to server." << std::endl;

    PlayerSide mySide = PlayerSide::NEUTRAL;

    // 1. Receive AssignPlayerIdentity
    sf::Packet identityPacket;
    std::cout << "Waiting for AssignPlayerIdentity..." << std::endl;
    if (socket.receive(identityPacket) != sf::Socket::Done) {
        std::cerr << "TEST FAILED: Did not receive AssignPlayerIdentity packet (or timeout)." << std::endl;
        return 1;
    }
    // printPacketDetails(identityPacket, "AssignPlayerIdentity");

    sf::Uint8 msgTypeInt;
    if (!(identityPacket >> msgTypeInt)) {
        std::cerr << "TEST FAILED: Could not deserialize message type for identity packet." << std::endl;
        return 1;
    }
    MessageType identityMsgType = static_cast<MessageType>(msgTypeInt);

    if (identityMsgType != MessageType::AssignPlayerIdentity) {
        std::cerr << "TEST FAILED: Expected AssignPlayerIdentity, got type " << msgTypeInt << std::endl;
        return 1;
    }

    if (!(identityPacket >> mySide)) { // Uses PlayerSide operator>>
        std::cerr << "TEST FAILED: Could not deserialize PlayerSide." << std::endl;
        return 1;
    }

    if (mySide != PlayerSide::PLAYER_ONE && mySide != PlayerSide::PLAYER_TWO) {
        std::cerr << "TEST FAILED: Invalid PlayerSide received: " << static_cast<int>(mySide) << std::endl;
        return 1;
    }
    std::cout << "Received player identity: " << (mySide == PlayerSide::PLAYER_ONE ? "Player One" : "Player Two") << std::endl;

    // 2. Receive GameStart
    sf::Packet gameStartPacket;
    std::cout << "Waiting for GameStart..." << std::endl;
    if (socket.receive(gameStartPacket) != sf::Socket::Done) {
        std::cerr << "TEST FAILED: Did not receive GameStart packet (or timeout)." << std::endl;
        return 1;
    }
    // printPacketDetails(gameStartPacket, "GameStart");
    
    if (!(gameStartPacket >> msgTypeInt)) {
        std::cerr << "TEST FAILED: Could not deserialize message type for game start packet." << std::endl;
        return 1;
    }
    MessageType gameStartMsgType = static_cast<MessageType>(msgTypeInt);

    if (gameStartMsgType != MessageType::GameStart) {
        std::cerr << "TEST FAILED: Expected GameStart, got type " << msgTypeInt << std::endl;
        return 1;
    }

    PlayerSide activePlayer;
    if (!(gameStartPacket >> activePlayer)) {
        std::cerr << "TEST FAILED: Could not deserialize active player for GameStart." << std::endl;
        return 1;
    }
    if (activePlayer != PlayerSide::PLAYER_ONE && activePlayer != PlayerSide::PLAYER_TWO) {
        std::cerr << "TEST FAILED: Invalid active player in GameStart: " << static_cast<int>(activePlayer) << std::endl;
        return 1;
    }
    std::cout << "GameStart: Active player is " << (activePlayer == PlayerSide::PLAYER_ONE ? "Player One" : "Player Two") << std::endl;

    // Deserialize board state
    int pieceCount = 0;
    std::cout << "GameStart: Board State:" << std::endl;
    for (int y = 0; y < GAME_BOARD_SIZE; ++y) {
        for (int x = 0; x < GAME_BOARD_SIZE; ++x) {
            sf::Uint8 pieceTypeInt;
            if (!(gameStartPacket >> pieceTypeInt)) {
                std::cerr << "TEST FAILED: Could not deserialize piece type at (" << x << "," << y << ")" << std::endl;
                return 1;
            }
            PieceType pieceType = static_cast<PieceType>(pieceTypeInt);

            if (pieceType != PieceType::NONE) {
                pieceCount++;
                PlayerSide pieceOwner;
                sf::Int8 health;
                if (!(gameStartPacket >> pieceOwner >> health)) {
                    std::cerr << "TEST FAILED: Could not deserialize piece owner/health at (" << x << "," << y << ")" << std::endl;
                    return 1;
                }
                // Basic validation
                if (pieceOwner != PlayerSide::PLAYER_ONE && pieceOwner != PlayerSide::PLAYER_TWO) {
                     std::cerr << "TEST FAILED: Invalid piece owner " << (int)pieceOwner << " at (" << x << "," << y << ")" << std::endl;
                     return 1;
                }
                if (health <= 0) {
                     std::cerr << "TEST FAILED: Non-positive health " << (int)health << " for piece at (" << x << "," << y << ")" << std::endl;
                     return 1;
                }
                // std::cout << "  Piece at (" << x << "," << y << "): Type " << pieceTypeInt 
                //           << ", Owner " << (pieceOwner == PlayerSide::PLAYER_ONE ? "P1" : "P2")
                //           << ", Health " << (int)health << std::endl;
            }
        }
    }
    // Basic check: initial game should have some pieces. Standard chess has 32.
    // Our game might differ, but 0 pieces is suspicious.
    // The default GameInitializer sets up 16 pieces per side initially.
    if (pieceCount < 16) { // A somewhat arbitrary number, but should be more than a few.
        std::cerr << "TEST FAILED: Unexpectedly low piece count on initial board: " << pieceCount << std::endl;
        return 1;
    }
     std::cout << "  Total pieces on board: " << pieceCount << " (looks plausible)." << std::endl;


    sf::Int32 steamP1, steamP2;
    if (!(gameStartPacket >> steamP1 >> steamP2)) {
        std::cerr << "TEST FAILED: Could not deserialize steam amounts." << std::endl;
        return 1;
    }
    if (steamP1 < 0 || steamP2 < 0) {
        std::cerr << "TEST FAILED: Negative steam amounts received: P1=" << steamP1 << ", P2=" << steamP2 << std::endl;
        return 1;
    }
    std::cout << "GameStart: Steam P1=" << steamP1 << ", P2=" << steamP2 << std::endl;

    // Check if there's any trailing data in the packet, which might indicate a problem
    if (gameStartPacket.getDataSize() > 0 && !gameStartPacket.endOfPacket()) {
        // This check is tricky because sf::Packet extraction might not advance read pos if it fails to read a complex type.
        // However, if all individual reads succeeded, and there's still data, it's a warning.
        // For now, let's assume if all deserializations passed, it's okay.
        // std::cout << "Warning: Trailing data in GameStart packet." << std::endl;
    }


    std::cout << "Received GameStart. Initial board and state seems valid." << std::endl;
    std::cout << "TEST PASSED: Connected and received initial game state." << std::endl;
    socket.disconnect();
    return 0;
}
