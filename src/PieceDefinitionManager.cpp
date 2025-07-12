#include "PieceDefinitionManager.h"
#include <fstream> // For file reading
#include <iostream> // For error messages

// Conditional include for nlohmann/json
// If worker is using nlohmann/json:
#include "nlohmann/json.hpp" // Assuming it's now in vendor/nlohmann/json.hpp
// End if

namespace BayouBonanza {

PieceDefinitionManager::PieceDefinitionManager() : loadedSuccessfully(false) {}

bool PieceDefinitionManager::loadDefinitions(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open piece definition file: " << filePath << std::endl;
        loadedSuccessfully = false;
        return false;
    }

    // === IF USING nlohmann/json ===
    nlohmann::json jsonData;
    try {
        file >> jsonData; // Parse the JSON file
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "Error: Could not parse piece definition JSON: " << e.what() << std::endl;
        loadedSuccessfully = false;
        return false;
    }

    if (!jsonData.is_array()) {
        std::cerr << "Error: Piece definition JSON must be an array of piece stats." << std::endl;
        loadedSuccessfully = false;
        return false;
    }

    pieceStatsMap.clear(); // Clear previous definitions

    for (const auto& pieceJson : jsonData) {
        if (!pieceJson.contains("cardType") || pieceJson["cardType"] != "PIECE_CARD") {
            continue;
        }
        PieceStats stats;
        try {
            stats.typeName = pieceJson.at("typeName").get<std::string>();
            stats.symbol = pieceJson.at("symbol").get<std::string>();
            stats.spritePath = pieceJson.value("sprite", std::string());
    
            stats.cardArtPath = pieceJson.value("cardArt", std::string());
            stats.attack = pieceJson.at("attack").get<int>();
            stats.health = pieceJson.at("health").get<int>();

            stats.isRanged = pieceJson.value("isRanged", false);
            stats.isVictoryPiece = pieceJson.value("victoryPiece", false);

            // Parse movementRules
            if (pieceJson.contains("movementRules") && pieceJson.at("movementRules").is_array()) {
                for (const auto& ruleJson : pieceJson.at("movementRules")) {
                    PieceMovementRule rule;
                    rule.isPawnForward = ruleJson.value("isPawnForward", false);
                    rule.isPawnCapture = ruleJson.value("isPawnCapture", false);
                    rule.canJump = ruleJson.at("canJump").get<bool>();
                    rule.maxRange = ruleJson.at("maxRange").get<int>();
                    if (ruleJson.contains("relativeMoves") && ruleJson.at("relativeMoves").is_array()) {
                        for (const auto& movePosJson : ruleJson.at("relativeMoves")) {
                            rule.relativeMoves.push_back({movePosJson.at("x").get<int>(), movePosJson.at("y").get<int>()});
                        }
                    }
                    stats.movementRules.push_back(rule);
                }
            }

            // Parse influenceRules (similar to movementRules)
            if (pieceJson.contains("influenceRules") && pieceJson.at("influenceRules").is_array()) {
                for (const auto& ruleJson : pieceJson.at("influenceRules")) {
                    PieceMovementRule rule;
                rule.isPawnForward = ruleJson.value("isPawnForward", false); // Use .value() for optional bool
                rule.isPawnCapture = ruleJson.value("isPawnCapture", false); // Use .value() for optional bool
                    rule.canJump = ruleJson.at("canJump").get<bool>();
                    rule.maxRange = ruleJson.at("maxRange").get<int>();
                     if (ruleJson.contains("relativeMoves") && ruleJson.at("relativeMoves").is_array()) {
                        for (const auto& movePosJson : ruleJson.at("relativeMoves")) {
                            rule.relativeMoves.push_back({movePosJson.at("x").get<int>(), movePosJson.at("y").get<int>()});
                        }
                    }
                    stats.influenceRules.push_back(rule);
                }
            }
            pieceStatsMap[stats.typeName] = stats;
        } catch (nlohmann::json::exception& e) {
            std::string currentTypeName = "UNKNOWN";
            if(pieceJson.contains("typeName") && pieceJson.at("typeName").is_string()){
                currentTypeName = pieceJson.at("typeName").get<std::string>();
            }
            std::cerr << "Error: Missing or invalid field in piece definition for '" << currentTypeName << "': " << e.what() << std::endl;
            // Optionally skip this piece and continue, or fail all loading
        }
    }
    // === END IF USING nlohmann/json ===

    if (pieceStatsMap.empty() && jsonData.is_array() && !jsonData.empty()) {
        // This means parsing might have failed for all entries or manual parsing was incomplete
        std::cerr << "Warning: Piece definitions loaded, but map is empty. Check for parsing errors for all entries." << std::endl;
        loadedSuccessfully = false;
        // return false; // Decide if this should be a hard fail
    } else if (pieceStatsMap.empty() && (!jsonData.is_array() || jsonData.empty())) {
        // This implies jsonData was not an array or was empty to begin with.
        // The initial error messages for non-array or non-open file would have caught this.
        // If we reach here, it means the file was okay but contained no valid data or was empty.
        if (jsonData.is_array() && jsonData.empty()){
             std::cout << "Note: Piece definition file was empty." << std::endl;
        }
        // Keep loadedSuccessfully as false if the map is empty.
        loadedSuccessfully = false;
    }
    else {
        loadedSuccessfully = true;
    }
    
    return loadedSuccessfully;
}

const PieceStats* PieceDefinitionManager::getPieceStats(const std::string& typeName) const {
    if (!loadedSuccessfully) {
        // It might be too noisy to print this every time if loading intentionally failed or file was empty.
        // Consider if this warning is always appropriate.
        // std::cerr << "Warning: Attempting to get piece stats, but definitions were not loaded successfully." << std::endl;
        return nullptr;
    }
    auto it = pieceStatsMap.find(typeName);
    if (it != pieceStatsMap.end()) {
        return &it->second;
    }
    // It might be too noisy to print an error every time a piece is not found,
    // as this could be a normal game logic check.
    // std::cerr << "Error: Piece stats not found for type: " << typeName << std::endl;
    return nullptr;
}

std::vector<std::string> PieceDefinitionManager::getAllPieceTypeNames() const {
    if (!loadedSuccessfully) {
         // std::cerr << "Warning: Attempting to get piece type names, but definitions were not loaded successfully." << std::endl;
        return {};
    }
    std::vector<std::string> names;
    names.reserve(pieceStatsMap.size()); // Reserve space to avoid reallocations
    for (const auto& pair : pieceStatsMap) {
        names.push_back(pair.first);
    }
    return names;
}

} // namespace BayouBonanza
