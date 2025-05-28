# Bayou Bonanza Architecture Guidelines

## Overview

This document outlines the architectural principles and decision-making process for organizing code between the three main components of the Bayou Bonanza project:

1. **GameLogic** (Shared Static Library)
2. **BayouBonanzaClient** (Client Executable)
3. **BayouBonanzaServer** (Server Executable)

## Architectural Principles

### 1. Separation of Concerns
- **Core Logic**: Game rules, piece behavior, and state management belong in shared code
- **Presentation**: Rendering, input handling, and UI belong in client code
- **Authority**: Server-specific validation and network management belong in server code

### 2. Dependency Management
- **Minimize Dependencies**: Each component should only depend on what it actually needs
- **Avoid Circular Dependencies**: Clear dependency hierarchy must be maintained
- **Platform Independence**: Server should be able to run headless

### 3. Code Reusability
- **Shared Logic**: Common functionality should be in GameLogic to avoid duplication
- **Client-Specific**: UI and input handling should not pollute shared code
- **Server-Specific**: Authoritative logic should not be accessible to clients

## Decision Matrix

### GameLogic (Shared Library)

**Include if the code:**
- ✅ Implements core game rules (piece movement, combat, win conditions)
- ✅ Manages game state that both client and server need
- ✅ Provides data structures used by both client and server
- ✅ Handles serialization/deserialization for network communication
- ✅ Contains algorithms that should be consistent between client and server
- ✅ Is platform-independent and doesn't require graphics

**Examples:**
- `GameBoard`, `GameState`, `Piece` classes
- `Move`, `Position` data structures
- `GameRules`, `CombatSystem`, `TurnManager`
- Network protocol definitions (`NetworkProtocol.h`)
- Piece movement validation
- Game state serialization

**Dependencies:**
- SFML Network (for serialization support)
- SFML System (for basic types)
- Standard C++ libraries

### BayouBonanzaClient (Client Executable)

**Include if the code:**
- ✅ Handles user input (mouse, keyboard, gamepad)
- ✅ Manages rendering and graphics
- ✅ Implements UI components and layouts
- ✅ Handles client-side networking (connecting to server)
- ✅ Manages client-specific state (selected pieces, UI state)
- ✅ Provides user feedback (animations, sounds, notifications)

**Examples:**
- `main.cpp` - Application entry point and main loop
- `InputManager` - Mouse and keyboard input handling
- Rendering code for board, pieces, and UI
- Client-side move validation (for immediate feedback)
- Connection management to game server
- UI state management

**Dependencies:**
- GameLogic library
- SFML Graphics, Window, Audio
- SFML Network (for server communication)
- Platform-specific libraries as needed

### BayouBonanzaServer (Server Executable)

**Include if the code:**
- ✅ Manages authoritative game state
- ✅ Handles client connections and networking
- ✅ Validates moves and enforces rules
- ✅ Manages game sessions and matchmaking
- ✅ Implements server-side security and anti-cheat
- ✅ Handles persistence and logging

**Examples:**
- `server_main.cpp` - Server entry point and main loop
- Client connection management
- Authoritative move validation
- Game session management
- Player authentication and authorization
- Game state persistence
- Server administration tools

**Dependencies:**
- GameLogic library
- SFML Network (for client communication)
- Database libraries (if using persistence)
- Logging libraries
- Security libraries

## Dependency Graph

```
┌─────────────────────┐    ┌─────────────────────┐
│  BayouBonanzaClient │    │  BayouBonanzaServer │
│                     │    │                     │
│  • main.cpp         │    │  • server_main.cpp │
│  • InputManager     │    │  • Connection Mgmt  │
│  • Rendering        │    │  • Auth & Security  │
│  • UI Components    │    │  • Persistence      │
└──────────┬──────────┘    └──────────┬──────────┘
           │                          │
           │         ┌────────────────┴──────────────┐
           │         │                               │
           └─────────▼───────────────────────────────▼─┐
                     │         GameLogic               │
                     │                                 │
                     │  • GameBoard, GameState         │
                     │  • Piece classes & logic        │
                     │  • Move validation              │
                     │  • Combat system                │
                     │  • Network protocol             │
                     │  • Serialization                │
                     └─────────────────────────────────┘
                                       │
                                       ▼
                              ┌─────────────────┐
                              │  SFML Network   │
                              │  SFML System    │
                              └─────────────────┘
```

## Real-World Example: InputManager Decision

### Initial Implementation (Incorrect)
```
GameLogic/
├── src/InputManager.cpp  ❌ Wrong location
└── include/InputManager.h
```

**Problems:**
- GameLogic now depends on SFML Graphics
- Server unnecessarily links graphics libraries
- Violates separation of concerns
- Makes testing more complex

### Corrected Implementation
```
BayouBonanzaClient/
├── src/main.cpp
├── src/InputManager.cpp  ✅ Correct location
└── include/InputManager.h
```

**Benefits:**
- Clean dependency separation
- Server remains headless
- Client-specific concerns isolated
- Easier testing and maintenance

## Testing Architecture

### GameLogic Tests (`BayouBonanzaTests`)
**Test:**
- Core game mechanics
- Piece movement rules
- Combat calculations
- Game state management
- Network serialization

**Dependencies:**
- GameLogic library
- Catch2 testing framework
- SFML Network/System (inherited from GameLogic)

### Client Tests (`BayouBonanzaClientTests`)
**Test:**
- Input handling logic
- UI component behavior
- Client-side validation
- Rendering calculations
- User interaction flows

**Dependencies:**
- GameLogic library
- Client-specific code (InputManager, etc.)
- Catch2 testing framework
- Full SFML suite (Graphics, Window, etc.)

### Server Tests (Future)
**Test:**
- Server networking logic
- Authoritative validation
- Security measures
- Performance under load
- Database operations

## Common Pitfalls and Solutions

### Pitfall 1: Graphics Dependencies in Shared Code
**Problem:** Adding SFML Graphics dependencies to GameLogic
**Solution:** Keep graphics code in client, use abstract interfaces if needed

### Pitfall 2: Client-Specific Logic in GameLogic
**Problem:** Adding UI state or input handling to shared library
**Solution:** Create client-specific managers that use GameLogic APIs

### Pitfall 3: Duplicated Validation Logic
**Problem:** Different validation logic in client and server
**Solution:** Shared validation in GameLogic, called by both client and server

### Pitfall 4: Tight Coupling Between Components
**Problem:** Direct dependencies between client and server code
**Solution:** Communicate only through GameLogic interfaces and network protocol

## Best Practices

### 1. Interface Design
- Design clean, minimal interfaces between components
- Use dependency injection where possible
- Prefer composition over inheritance

### 2. Data Flow
- Client → GameLogic → Server for move validation
- Server → GameLogic → Client for state updates
- Never direct client ↔ server communication outside of network protocol

### 3. Error Handling
- GameLogic should provide clear error states
- Client should handle UI-specific error presentation
- Server should handle security and persistence errors

### 4. Performance Considerations
- GameLogic should be optimized for both client and server use
- Client can cache state for immediate feedback
- Server should prioritize correctness over speed

### 5. Future Extensibility
- Design interfaces to support multiple client types
- Keep network protocol versioned and extensible
- Plan for horizontal server scaling

## Migration Guidelines

When moving code between components:

1. **Analyze Dependencies**: What libraries does the code require?
2. **Check Usage**: Is it used by client, server, or both?
3. **Consider Coupling**: Does it create inappropriate dependencies?
4. **Update Tests**: Move tests to appropriate test suite
5. **Update Build**: Modify CMakeLists.txt accordingly
6. **Verify Build**: Ensure all components still compile
7. **Run Tests**: Confirm functionality is preserved

## Conclusion

This architecture provides:
- **Clear Separation**: Each component has well-defined responsibilities
- **Maintainability**: Easy to understand and modify individual components
- **Testability**: Components can be tested in isolation
- **Scalability**: Server can be scaled independently of client
- **Flexibility**: Different client implementations possible
- **Performance**: Minimal dependencies reduce build times and binary size

Following these guidelines ensures a robust, maintainable, and scalable codebase that can evolve with the project's needs while maintaining clean architectural boundaries. 