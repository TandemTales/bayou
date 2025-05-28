#pragma once

#include <string>
#include <vector>
#include <map>
#include "PieceData.h" // Contains PieceStats and PieceMovementRule
// Forward declare nlohmann::json if used, or include directly
// #include <nlohmann/json.hpp> // Or forward declare if possible and include in .cpp

// If not using nlohmann::json, no specific includes here beyond standard ones.

namespace BayouBonanza {

class PieceDefinitionManager {
public:
    PieceDefinitionManager();

    // Loads definitions from a JSON file
    bool loadDefinitions(const std::string& filePath);

    // Retrieves stats for a piece type
    const PieceStats* getPieceStats(const std::string& typeName) const;

    // Get all loaded type names (optional, but useful for UI/debugging)
    std::vector<std::string> getAllPieceTypeNames() const;

private:
    std::map<std::string, PieceStats> pieceStatsMap;
    bool loadedSuccessfully;

    // If using nlohmann::json, a helper might be useful
    // void parsePieceStats(const nlohmann::json& j, PieceStats& stats);
    // void parseMovementRule(const nlohmann::json& jRule, PieceMovementRule& moveRule);
};

} // namespace BayouBonanza
