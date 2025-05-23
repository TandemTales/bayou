# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Bayou Bonanza is a C++20 chess-variant game that combines traditional chess mechanics with collectible card game elements. Players move pieces on an 8x8 board and can play cards to add new pieces. The goal is to eliminate the opponent's king while protecting your own.

## Build System & Commands

**Build:**
```bash
mkdir -p build && cd build
cmake ..
make
```

**Run:**
```bash
./build/BayouBonanza
```

**Run Tests:**
```bash
cd build
ctest
# Or run directly:
./BayouBonanzaTests
```

**Build Tests Only:**
```bash
cd build
make BayouBonanzaTests
```

## Architecture

**Core Components:**
- `GameState` - Central state manager (board, active player, turn tracking, steam resources)
- `GameBoard` - 8x8 grid of `Square` objects with piece management
- `Piece` hierarchy - Abstract base class with specific implementations (King, Queen, etc.)
- `TurnManager` - Handles turn logic and move processing
- `CombatSystem` - Manages piece combat with health/attack mechanics
- `GameRules` - Validates moves and game rules

**Key Design Patterns:**
- Abstract factory pattern for piece creation (`PieceFactory`)
- Strategy pattern for piece movement validation (each piece type implements `isValidMove`)
- State pattern for game phases (SETUP, MAIN_GAME, GAME_OVER)

**Coordinate System:**
- Board uses (x, y) coordinates where (0,0) is top-left
- All positions validated through `GameBoard::isValidPosition()`

**Combat System:**
- Pieces have attack/health attributes beyond traditional chess
- Damage calculation handled by `CombatCalculator`
- Pieces removed when health <= 0

**Steam Resource System:**
- Players generate "steam" based on controlled squares
- Steam used to play cards (future feature)
- Tracked per player in `GameState`

## Testing

Uses Catch2 v3.4.0 for unit testing. Tests are organized by component (PieceTests.cpp, CombatSystemTests.cpp, etc.). All major game logic should have corresponding unit tests.

## Dependencies

- **SFML 2.5** - Graphics, windowing, input handling (auto-downloaded via CMake if not found)
- **Catch2 v3.4.0** - Testing framework (auto-downloaded)
- **C++20** standard required

## Development Notes

- All game logic in `BayouBonanza` namespace
- Header files in `include/`, implementations in `src/`
- Prefer smart pointers (`std::shared_ptr`) for piece management
- Assets (fonts, etc.) copied to build directory automatically
- Cross-platform build support with Windows DLL handling