# Graphics System - Resolution and Aspect Ratio Management

## Overview

Bayou Bonanza now includes a comprehensive graphics management system that handles resolution scaling and aspect ratio management. The system ensures the game looks consistent across different screen sizes and resolutions while maintaining the intended visual proportions.

## Key Features

### 1. **16:9 Aspect Ratio Standard**
- **Base Resolution**: 1280×720 (16:9)
- **Why 16:9?**: Most common aspect ratio for modern displays, streaming platforms, and gaming
- **Compatibility**: Works well with YouTube, Twitch, and other streaming services
- **Future-proof**: Scales well to 1080p, 1440p, 4K, and other common resolutions

### 2. **Automatic Scaling**
- Game content scales automatically to fit any window size
- Maintains aspect ratio with letterboxing when necessary
- All game logic remains independent of actual screen resolution
- Smooth scaling preserves visual quality

### 3. **Coordinate System**
- **Game Coordinates**: All game logic uses the base 1280×720 coordinate system
- **Screen Coordinates**: Actual pixel positions on the user's display
- **Automatic Conversion**: GraphicsManager handles all coordinate transformations
- **Input Handling**: Mouse clicks are automatically converted to game coordinates

## Architecture

### GraphicsManager Class

The `GraphicsManager` class is the core component that handles:

```cpp
class GraphicsManager {
public:
    // Base resolution constants
    static constexpr float BASE_WIDTH = 1280.0f;
    static constexpr float BASE_HEIGHT = 720.0f;
    static constexpr float BASE_ASPECT_RATIO = BASE_WIDTH / BASE_HEIGHT;
    
    // Key methods
    void updateView();                                    // Call when window resizes
    sf::Vector2f screenToGame(const sf::Vector2i& screenPos);  // Convert coordinates
    BoardRenderParams getBoardRenderParams();             // Get board layout info
    void applyView();                                     // Apply scaling to window
};
```

### Integration Points

1. **Main Game Loop**: 
   - `graphicsManager.applyView()` before rendering
   - `graphicsManager.updateView()` on window resize

2. **Input System**: 
   - All mouse coordinates converted through GraphicsManager
   - InputManager uses game coordinates internally

3. **Rendering**: 
   - Board layout calculated using `getBoardRenderParams()`
   - UI elements positioned using base resolution coordinates

## Usage Examples

### Window Resize Handling
```cpp
if (event.type == sf::Event::Resized) {
    graphicsManager.updateView();
}
```

### Coordinate Conversion
```cpp
// Convert screen mouse position to game coordinates
sf::Vector2i screenPos = sf::Mouse::getPosition(window);
sf::Vector2f gamePos = graphicsManager.screenToGame(screenPos);
```

### Board Rendering
```cpp
auto boardParams = graphicsManager.getBoardRenderParams();
// Use boardParams.boardSize, boardParams.squareSize, etc.
```

## Benefits

### For Players
- **Consistent Experience**: Game looks the same regardless of screen size
- **No Distortion**: Maintains proper proportions on any display
- **Flexible Window Sizes**: Can resize window or play fullscreen
- **High DPI Support**: Scales cleanly on high-resolution displays

### For Developers
- **Simple Coordinates**: Always work with 1280×720 coordinate system
- **Resolution Independence**: No need to handle different screen sizes in game logic
- **Easy UI Layout**: Position elements using consistent base coordinates
- **Automatic Scaling**: Graphics system handles all the complexity

## Technical Details

### Scaling Algorithm
1. Calculate window aspect ratio
2. Compare with base aspect ratio (16:9)
3. Scale to fit while maintaining proportions
4. Add letterboxing (black bars) if needed
5. Set up SFML viewport for proper rendering

### Letterboxing
- **Wider Windows**: Black bars on left and right
- **Taller Windows**: Black bars on top and bottom
- **Perfect 16:9**: No letterboxing needed

### Performance
- Minimal overhead: scaling handled by GPU
- No impact on game logic performance
- Efficient coordinate conversion
- Smooth rendering at any resolution

## Future Enhancements

### Planned Features
- **Multiple Aspect Ratio Support**: 4:3, 21:9 ultrawide options
- **UI Scaling Options**: Allow players to adjust UI size
- **Resolution Presets**: Quick selection of common resolutions
- **Fullscreen Modes**: Exclusive fullscreen and borderless windowed

### Configuration Options
- **Graphics Settings Menu**: In-game resolution and scaling options
- **Config File Support**: Save preferred resolution settings
- **Command Line Args**: Override resolution for testing

## Migration Notes

### From Old System
The previous system calculated board layout manually in main.cpp:
```cpp
// Old approach (removed)
float windowWidth = static_cast<float>(window.getSize().x);
float windowHeight = static_cast<float>(window.getSize().y);
float boardSize = std::min(windowWidth, windowHeight) * 0.8f;
```

### New System
Now uses GraphicsManager for consistent scaling:
```cpp
// New approach
auto boardParams = graphicsManager.getBoardRenderParams();
// boardParams contains all layout information
```

## Testing

### Resolution Testing
Test the game at various resolutions to ensure proper scaling:
- **Common 16:9**: 1920×1080, 1366×768, 2560×1440
- **Older 4:3**: 1024×768, 1280×1024
- **Ultrawide**: 3440×1440, 2560×1080
- **Mobile/Tablet**: Various portrait and landscape ratios

### Window Resize Testing
- Start with default window size
- Resize to different dimensions
- Switch between windowed and fullscreen
- Verify UI elements remain properly positioned

## Troubleshooting

### Common Issues
1. **Blurry Graphics**: Ensure proper scaling factor calculation
2. **UI Misalignment**: Check coordinate conversion in input handling
3. **Performance Issues**: Verify efficient viewport usage
4. **Aspect Ratio Problems**: Confirm letterboxing implementation

### Debug Information
The GraphicsManager outputs scaling information to console:
```
Graphics: Window 1920x1080, Scale: 1.5, Offset: (0, 0)
```

This helps diagnose scaling and positioning issues during development. 