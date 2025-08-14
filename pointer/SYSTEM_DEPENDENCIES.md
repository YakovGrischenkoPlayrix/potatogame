# System Dependencies and Setup

## Core Dependencies

### SDL2 Ecosystem
The project uses SDL2 and its extensions for multimedia functionality:

```json
// vcpkg.json
{
  "name": "brotato-game",
  "version": "1.0.0",
  "dependencies": [
    "sdl2",           // Core SDL2 library for window/input/rendering
    "sdl2-image",     // Image loading (PNG support)
    "sdl2-ttf"        // TrueType font rendering
  ]
}
```

### CMake Configuration
```cmake
# CMakeLists.txt - Minimum requirements
cmake_minimum_required(VERSION 3.16)
project(BrotatoGame)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_image CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

# Link libraries with proper fallback
target_link_libraries(${PROJECT_NAME} 
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)
```

## Build System Setup

### Windows (vcpkg + CMake)
```powershell
# Install vcpkg dependencies
vcpkg install sdl2 sdl2-image sdl2-ttf

# Configure CMake with vcpkg toolchain
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path-to-vcpkg]/scripts/buildsystems/vcpkg.cmake

# Build project
cmake --build build --config Release
```

### Build Scripts
The project includes several build scripts for convenience:

- **`build-game.bat`**: Standard Windows build
- **`build-game.ps1`**: PowerShell build script
- **`debug-build.bat`**: Debug configuration build
- **`quick-build.bat`**: Fast incremental build
- **`simple-build.bat`**: Minimal build without extras

### Asset Management
```cmake
# Automatic asset copying
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/monsters
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/monsters
    COMMENT "Copying monster assets to build directory"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying UI assets to build directory"
)
```

## Required Assets Structure

### Directory Layout
```
project_root/
├── assets/
│   ├── character/
│   │   └── brick.png          # Player sprite
│   ├── weapons/
│   │   ├── pistol.png         # Weapon sprites
│   │   ├── pistol2.png
│   │   ├── pistol3.png
│   │   ├── smg.png
│   │   ├── smg2.png
│   │   ├── brickonstick.png
│   │   ├── shotgun.png
│   │   └── shotgun2.png
│   ├── ui/
│   │   ├── card_normal.png    # Shop UI elements
│   │   ├── coin.png
│   │   └── lock_unlocked.png
│   └── fonts/
│       └── default.ttf        # Primary font file
└── monsters/
    └── landmonster/
        └── Transparent PNG/
            ├── idle/
            │   ├── frame-1.png
            │   └── frame-2.png
            └── got hit/
                └── frame.png
```

### Font Fallback System
```cpp
// Font loading with fallback priority
const char* fontPaths[] = {
    "assets/fonts/default.ttf",      // Custom font (preferred)
    "C:/Windows/Fonts/arial.ttf",    // Windows fallback
    "C:/Windows/Fonts/calibri.ttf",  // Alternative Windows font
    "C:/Windows/Fonts/consola.ttf"   // Monospace fallback
};

// Graceful degradation to bitmap rendering if no TTF font available
if (!defaultFont) {
    std::cout << "No TTF font available - using bitmap fallback" << std::endl;
    // Continue with bitmap text rendering
}
```

## Platform-Specific Considerations

### Windows Requirements
- **Visual Studio 2019** or later (for C++17 support)
- **vcpkg** package manager for dependency management
- **CMake 3.16+** for build system
- **Windows 10** or later (tested platform)

### Display Configuration
```cpp
// Window setup for full-screen desktop mode
window = SDL_CreateWindow("Brotato MVP", 
                         SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         WINDOW_WIDTH, WINDOW_HEIGHT, 
                         SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);

// Fixed resolution assumptions
static const int WINDOW_WIDTH = 1920;
static const int WINDOW_HEIGHT = 1080;
```

### Performance Requirements
- **60 FPS target**: 16ms frame budget with `SDL_Delay(16)`
- **Hardware acceleration**: Uses `SDL_RENDERER_ACCELERATED`
- **Memory usage**: Designed for hundreds of entities simultaneously

## Development Environment Setup

### Recommended IDE Configuration
```cpp
// Preprocessor definitions for development
#ifdef DEBUG_MODE
    #define DEBUG_LOG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
    #define DEBUG_LOG(msg)
#endif

#ifdef ENABLE_TESTS
    // Include test framework
    void runAllTests();
#endif
```

### Code Style Requirements
- **C++17 standard** minimum
- **RAII patterns** for resource management
- **Smart pointers** for memory management
- **const correctness** for method signatures
- **Include guards** or `#pragma once`

## Runtime Dependencies

### SDL2 Initialisation Order
```cpp
bool Game::init() {
    // 1. Core SDL2
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    
    // 2. SDL_image for PNG support
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) return false;
    
    // 3. SDL_ttf for font rendering
    if (TTF_Init() == -1) return false;
    
    // 4. Create window and renderer
    window = SDL_CreateWindow(...);
    renderer = SDL_CreateRenderer(...);
    
    return true;
}
```

### Resource Loading Patterns
```cpp
// Texture loading with error handling
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cout << "Failed to load " << path << ": " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Always clean up surface
    
    return texture;
}
```

## Memory Management

### Smart Pointer Usage
```cpp
// Entity collections with automatic cleanup
std::vector<std::unique_ptr<Enemy>> enemies;
std::vector<std::unique_ptr<Bullet>> bullets;
std::vector<std::unique_ptr<Weapon>> weapons;

// Player and shop as unique_ptr members
std::unique_ptr<Player> player;
std::unique_ptr<Shop> shop;
```

### SDL Resource Cleanup
```cpp
Game::~Game() {
    cleanup();
}

void Game::cleanup() {
    // Font cleanup
    if (defaultFont) {
        TTF_CloseFont(defaultFont);
        defaultFont = nullptr;
    }
    
    // Renderer cleanup
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    // Window cleanup
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    // SDL subsystem cleanup
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}
```

## Error Handling Strategy

### Graceful Degradation
```cpp
// Asset loading with fallbacks
if (playerTexture) {
    // Render sprite
    SDL_RenderCopy(renderer, playerTexture, nullptr, &destRect);
} else {
    // Fallback to geometric rendering
    renderFallbackSprite(renderer);
}

// Font rendering with fallback
if (defaultFont) {
    renderTTFText(renderer, text, x, y, color, fontSize);
} else {
    // Use bitmap font system
    renderText(text, x, y, scale);
}
```

### Error Reporting
```cpp
// Consistent error reporting format
std::cout << "System failed to initialize! Error: " << GetSystemError() << std::endl;

// Context-specific error messages
std::cout << "Failed to load weapon texture: " << texturePath 
          << " - " << IMG_GetError() << std::endl;
```

## Performance Optimisation Dependencies

### Compiler Optimisations
```cmake
# Release build optimisations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        $<$<CXX_COMPILER_ID:MSVC>:/O2>
        $<$<CXX_COMPILER_ID:GNU>:-O3>
    )
endif()
```

### Runtime Performance Monitoring
```cpp
// Frame time measurement
Uint32 lastTime = SDL_GetTicks();
while (running) {
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;
    
    // Game logic...
    
    SDL_Delay(16); // Target ~60 FPS
}
```

## Troubleshooting Common Issues

### Missing Dependencies
- **vcpkg not found**: Ensure vcpkg is installed and CMAKE_TOOLCHAIN_FILE points to vcpkg.cmake
- **SDL2 link errors**: Verify vcpkg triplet matches your build configuration (x64-windows vs x86-windows)
- **Asset not found**: Check that build scripts properly copy assets to output directory

### Runtime Issues
- **Black screen**: Verify renderer creation and SDL_RenderPresent() calls
- **No audio**: Project doesn't use SDL_mixer, only visual/input systems
- **Font rendering fails**: Check font file paths and TTF_Init() success

### Performance Problems
- **Low FPS**: Monitor entity counts, reduce sprite complexity, or optimise update loops
- **Memory leaks**: Ensure proper cleanup of SDL resources and smart pointer usage
- **High CPU usage**: Profile update loops and collision detection algorithms

This dependency framework provides a solid foundation for development while maintaining compatibility across different Windows environments and build configurations.
