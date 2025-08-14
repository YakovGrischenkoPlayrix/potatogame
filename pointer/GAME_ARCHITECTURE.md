# Game Architecture Overview

## High-Level System Overview

The potatogame is a C++ SDL2 implementation of a Brotato-style top-down survival game with the following core systems:

### Core Systems
- **Game Loop**: Main game management, rendering, and state coordination
- **Player System**: Character movement, stats, health, experience, and progression
- **Combat System**: Weapons, bullets, melee attacks, and damage calculation
- **Enemy System**: AI-driven enemies with different types and behaviours
- **Shop System**: Wave-based item purchasing with materials currency
- **Resource System**: Experience orbs and materials collection
- **Wave System**: Timed waves with increasing difficulty

## System Interactions

### Main Game Loop Flow
```
Game::run() -> handleEvents() -> update() -> render() -> repeat
```

### System Dependencies
```
Game (Central Hub)
├── Player (character state & weapons)
│   ├── Weapon[] (up to 6 weapons)
│   └── PlayerStats (health, damage, speed, etc.)
├── Enemy[] (AI-driven hostiles)
│   ├── SlimeEnemy (ranged projectile type)
│   └── PebblinEnemy (melee rush type)
├── Bullet[] (projectiles from weapons/enemies)
├── Shop (wave-end purchasing system)
├── ExperienceOrb[] (player progression)
└── Material[] (currency pickups)
```

## Key Files and Responsibilities

### Core Game Management
- **`Game.h/cpp`**: Central game controller, main loop, system coordination
  - Window/renderer management
  - Game state (wave system, timing)
  - Entity collection management (enemies, bullets, orbs)
  - Collision detection and resolution
  - UI rendering (health, experience, wave timer)

### Player System
- **`Player.h/cpp`**: Player character implementation
  - Movement and input handling
  - Health, experience, and leveling system
  - Weapon inventory management (max 6 weapons)
  - Stats system with Brotato-style progression
  - Sprite rendering and collision

### Combat System
- **`Weapon.h/cpp`**: Weapon types and combat mechanics
  - Multiple weapon types (Pistol, SMG, Melee Stick, Shotgun)
  - Tier-based weapon progression (T1-T4)
  - Attack timing, damage calculation, critical hits
  - Melee vs ranged weapon handling
  - Sprite-based weapon rendering with rotation

- **`Bullet.h/cpp`**: Projectile system
  - Different bullet types (player/enemy projectiles)
  - Physics simulation (linear/lobbed trajectories)
  - Range and damage properties
  - Collision detection

### Enemy System
- **`Enemy.h/cpp`**: Base enemy class with animation system
  - State management (Idle, Hit)
  - Sprite-based animation (idle frames, hit reaction)
  - Basic AI (move toward player)
  - Health and damage properties

- **`SlimeEnemy.h/cpp`**: Ranged enemy type
  - Projectile-based attacks
  - Maintains distance from player
  - Lob-style bullet physics

- **`PebblinEnemy.h/cpp`**: Melee enemy type
  - Rush-style AI behaviour
  - Direct contact damage

### Shop System
- **`Shop.h/cpp`**: Wave-end purchasing interface
  - Material-based economy
  - Weapon tier restrictions by wave
  - Reroll and item locking mechanics
  - TTF-based UI with mouse/keyboard input
  - Player stats display

### Resource System
- **`ExperienceOrb.h/cpp`**: Player progression pickups
  - Automatic collection within pickup range
  - Visual effects (bobbing animation)
  - Lifetime management

- **`Material.h/cpp`**: Currency system
  - Brotato-style material drops from enemies
  - Map limit enforcement (50 max on map)
  - Material bag overflow system

### Utility Systems
- **`Vector2.h/cpp`**: 2D math operations
  - Position, direction, and distance calculations
  - Vector arithmetic for physics

## Data Flow Between Systems

### Combat Flow
```
Player Input -> Weapon::update() -> Bullet Creation -> 
Collision Detection -> Enemy Damage -> Death -> 
Experience/Material Drops -> Collection -> Player Progression
```

### Wave Progression Flow
```
Wave Timer -> Enemy Spawning -> Combat -> Wave End -> 
Shop Opening -> Item Purchase -> Next Wave Start
```

### Player Progression Flow
```
Experience Collection -> Level Up -> Stat Increases -> 
Material Collection -> Shop Purchases -> Weapon Acquisition
```

## Key Design Patterns

### Entity Management
- Uses `std::vector<std::unique_ptr<T>>` for entity collections
- Automatic cleanup via RAII and smart pointers
- Remove-if idiom for entity cleanup

### Update/Render Separation
- Clear separation between game logic (`update()`) and rendering (`render()`)
- Delta time-based updates for frame-rate independence
- Consistent update order across all systems

### State Management
- Player stats centralised in `PlayerStats` struct
- Enemy states managed via enum-based state machines
- Shop state controls game pause/resume

### Resource Management
- SDL texture loading with error handling
- Automatic asset cleanup in destructors
- Fallback rendering when assets fail to load

## Performance Considerations

### Rendering
- Sprite-based rendering with SDL2 hardware acceleration
- Efficient circle rendering for fallback graphics
- TTF text rendering with surface-to-texture conversion

### Memory Management
- Smart pointers prevent memory leaks
- Entity removal via iterators to avoid invalidation
- Texture caching in weapon/enemy classes

### Update Optimisation
- Early exits when systems are inactive (shop open)
- Collision detection only for alive entities
- Efficient vector operations for math calculations

## Integration Points for New Features

### Adding New Weapon Types
1. Extend `WeaponType` enum
2. Add initialisation method in `Weapon.cpp`
3. Update texture loading in `loadWeaponTexture()`
4. Add shop integration in `Shop.cpp`

### Adding New Enemy Types
1. Inherit from `Enemy` base class
2. Implement custom `update()` and `render()` methods
3. Add factory function (like `CreateSlimeEnemy()`)
4. Update spawn system in `Game::spawnEnemies()`

### Adding New Systems
1. Add system class to `Game` as member
2. Integrate into main update/render loop
3. Handle system interactions through `Game` class
4. Add necessary UI elements to `renderUI()`

This architecture provides a solid foundation for expanding the game with new features while maintaining clean separation of concerns and efficient performance.
