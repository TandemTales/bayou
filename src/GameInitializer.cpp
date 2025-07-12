#include "GameInitializer.h"
#include "GameBoard.h"
#include "Square.h"
#include "InfluenceSystem.h"
#include "PieceCard.h"
#include <vector>
#include <iostream> // For std::cerr

namespace BayouBonanza {

// Constructor
GameInitializer::GameInitializer() {
    ownedPieceDefManager = std::make_unique<PieceDefinitionManager>();
    if (!ownedPieceDefManager->loadDefinitions("assets/data/pieces.json")) {
        // Handle error: Log and maybe throw or exit
        std::cerr << "FATAL: Could not load piece definitions from assets/data/pieces.json" << std::endl;
        // Consider throwing an exception or setting an error state that can be checked.
        // For now, proceeding will likely lead to issues if pieceFactory is used without definitions.
    }
    ownedPieceFactory = std::make_unique<BayouBonanza::PieceFactory>(*ownedPieceDefManager);
    
    // Set references to the owned instances
    pieceDefManager = ownedPieceDefManager.get();
    pieceFactory = ownedPieceFactory.get();
}

// Constructor with external references
GameInitializer::GameInitializer(const PieceDefinitionManager& pieceDefManager, PieceFactory& pieceFactory)
    : ownedPieceDefManager(nullptr), ownedPieceFactory(nullptr),
      pieceDefManager(&pieceDefManager), pieceFactory(&pieceFactory) {
}

void GameInitializer::initializeNewGame(GameState& gameState) {
    // Reset the game state to default values
    resetGameState(gameState);

    // Set up the board with initial pieces
    setupBoard(gameState);

    // Initialize the card system for both players
    gameState.initializeCardSystem();

    // Customize starting hands with victory piece cards
    setupStartingHands(gameState);

    // Calculate initial control values
    calculateInitialControl(gameState);
}

void GameInitializer::initializeNewGame(GameState& gameState, const Deck& deck1, const Deck& deck2) {
    resetGameState(gameState);
    setupBoard(gameState);
    gameState.initializeCardSystem(deck1, deck2);
    setupStartingHands(gameState);
    calculateInitialControl(gameState);
}

void GameInitializer::setupBoard(GameState& gameState) {
    // Clear the board only
    gameState.getBoard().resetBoard();
}

Piece* GameInitializer::createAndPlacePiece(GameState& gameState, const std::string& pieceType, PlayerSide side, int x, int y) {
    // Create the piece using the factory
    std::unique_ptr<Piece> piece = pieceFactory->createPiece(pieceType, side);
    
    if (!piece) {
        std::cerr << "Failed to create piece: " << pieceType << std::endl;
        return nullptr; // Invalid piece type or factory error
    }
    
    // Set the piece's position
    Position pos(x, y);
    piece->setPosition(pos);
    
    // Get raw pointer before moving ownership to the square
    Piece* rawPiecePtr = piece.get(); 
    
    // Place the piece on the board
    Square& square = gameState.getBoard().getSquare(x, y);
    square.setPiece(std::move(piece)); // Square now owns the piece
    
    return rawPiecePtr;
}

void GameInitializer::resetGameState(GameState& gameState) {
    // Set Player 1 as the starting player
    if (gameState.getActivePlayer() != PlayerSide::PLAYER_ONE) {
        gameState.switchActivePlayer();
    }
    
    // Begin in SETUP phase for initial victory piece placement
    gameState.setGamePhase(GamePhase::SETUP);
    
    // Set game result to in progress
    gameState.setGameResult(GameResult::IN_PROGRESS);
    
    // Reset turn number to 1
    gameState.setTurnNumber(1);
    
    // Reset steam for both players
    gameState.setSteam(PlayerSide::PLAYER_ONE, 0);
    gameState.setSteam(PlayerSide::PLAYER_TWO, 0);
}

void GameInitializer::calculateInitialControl(GameState& gameState) {
    GameBoard& board = gameState.getBoard();

    // Give initial control of the 4 center squares of the left and right columns
    std::vector<Position> p1Squares = { {0,2}, {0,3}, {0,4}, {0,5} };
    std::vector<Position> p2Squares = { {7,2}, {7,3}, {7,4}, {7,5} };
    for (const auto& pos : p1Squares) {
        board.getSquare(pos.x, pos.y).setControlledBy(PlayerSide::PLAYER_ONE);
    }
    for (const auto& pos : p2Squares) {
        board.getSquare(pos.x, pos.y).setControlledBy(PlayerSide::PLAYER_TWO);
    }

    // Use the new InfluenceSystem to calculate board influence and control
    InfluenceSystem::calculateBoardInfluence(board);
}

void GameInitializer::setupStartingHands(GameState& gameState) {
    for (PlayerSide side : {PlayerSide::PLAYER_ONE, PlayerSide::PLAYER_TWO}) {
        Hand& hand = gameState.getHand(side);
        Deck& deck = gameState.getDeck(side);

        hand.clear();

        for (size_t i = 0; i < deck.size();) {
            Card* card = deck.getCard(i);
            bool take = false;
            if (card && card->getCardType() == CardType::PIECE_CARD) {
                auto* pc = dynamic_cast<PieceCard*>(card);
                if (pc && pieceFactory->isVictoryPiece(pc->getPieceType())) {
                    take = true;
                }
            }

            if (take) {
                hand.addCard(deck.removeCardAt(i));
                continue;
            }
            ++i;
        }
    }
}

} // namespace BayouBonanza
