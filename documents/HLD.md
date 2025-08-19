## Brotato MVP — High-Level Design (HLD)

This document explains the overall architecture, key systems, data flow, and extensibility of the Brotato-inspired MVP game implemented in C++17 with SDL2.

### Goals
- Deliver a playable top‑down survival shooter MVP with Brotato‑style loops.
- Keep systems small, readable, and easy to extend (new enemies, weapons, UI, etc.).
- Be buildable on Windows via CMake + vcpkg with SDL2, SDL2_image, SDL2_ttf.

### Non‑Goals
- Full parity with Brotato's meta‑progression, itemization, and complex balancing.
- Highly generalized engine or data‑driven authoring tools.

## Project Structure

```
potatogame/
  assets/                 # runtime assets (copied to build dir)
    enemies/              # enemy sprites including boss variants
    weapons/              # weapon sprites with tier variations
    ui/                   # UI elements, boosters, shop items
    fonts/                # TTF fonts for text rendering
  monsters/               # classic monster sprites (copied to build dir)
  src/                    # C++ sources
    Bullet.*              # Projectile model + rendering
    Enemy.*               # Base enemy with simple AI + animation
    SlimeEnemy.*          # Ranged slime (single shots)
    PebblinEnemy.*        # Tanky pebblin (triple spread)
    BossEnemy.*           # Advanced boss with complex AI
    MiniBossEnemy.*       # Swarm boss with multiple variants
    FractalBoss.*         # Composite fractal structure boss
    FractalNode.*         # Individual fractal components
    ExperienceOrb.*       # XP pickup entity
    Material.*            # Currency pickup entity
    SpeedUpBooster.*      # Temporary speed boost pickup
    Weapon.*              # Ranged & melee weapon behaviors
    Player.*              # Player state, input, weapons, stats
    Game.*                # Game loop, spawning, collisions, UI, shop
    Vector2.*             # 2D vector math
    Shop.*                # Wave‑end shop UI/logic
    main.cpp              # Entrypoint
  CMakeLists.txt          # Build config, assets copy steps
  vcpkg.json              # Dependencies: sdl2, sdl2-image, sdl2-ttf
  README.md               # Setup and run guide
  CONTROLS_AND_GAMEPLAY.md
  QUICK_SETUP.md
  TROUBLESHOOTING.md
```

## Build and Run

- Toolchain: CMake ≥ 3.16, MSVC (VS 2019/2022), vcpkg.
- Dependencies: `sdl2`, `sdl2-image`, `sdl2-ttf` (see `vcpkg.json`).
- Configuration: project uses vcpkg toolchain file. Example:
  - `cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake`
  - In this workshop, the path may be `C:/W/vcpkg/...` as used by scripts.
- Post‑build: `CMakeLists.txt` copies `assets/` and `monsters/` into the executable directory.

## Runtime Overview

- Window: Fullscreen desktop, logical size 1920×1080.
- Game loop: ~60 FPS fixed delay, variable `deltaTime` based on `SDL_GetTicks()`.
- Systems: Player input and movement, enemy spawn/AI, projectiles, pickups (XP/Materials/Boosters), wave timer, end‑of‑wave shop, boss spawning, and UI rendering.

### Main Loop (simplified)

```mermaid
sequenceDiagram
  participant OS
  participant Game
  participant SDL
  participant Player
  participant Systems as Enemies/Bullets/Pickups/Shop
  participant Bosses as BossEnemy/MiniBoss/FractalBoss

  OS->>Game: start()
  Game->>SDL: init video, image, ttf; create window/renderer
  Game->>Player: create + initialize(renderer)
  Game->>Systems: create Shop; load assets; init weapons
  loop while running
    Game->>SDL: poll events
    alt Shop active
      Game->>Systems: Shop handle keyboard/mouse
    else
      Game->>Player: handle keyboard
      Game->>Player: update aim from mouse
    end
    Game->>Game: update(deltaTime)
    Game->>Bosses: update boss AI and spawning
    Game->>Game: render()
    Game->>SDL: present + delay(16ms)
  end
  Game->>SDL: destroy renderer/window; quit subsystems
```

## Core Modules and Responsibilities

### `Game`
- Initializes SDL subsystems and resources.
- Owns `Player`, `Enemy` list, `Bullet` list, pickups (XP/Materials/Boosters), `Shop`, and boss systems.
- Game state: score, wave index, wave timers, spawn telegraphers, material bag, boss spawning logic.
- Systems per frame:
  - Input routing (Shop vs Player), player/weapon updates, enemy updates.
  - Enemy spawn telegraphing and material/XP/booster updates.
  - Boss spawning logic (FractalBoss, MiniBoss, regular BossEnemy).
  - Collision checks (bullet↔enemy, enemy↔player, enemy bullets↔player).
  - Melee sweep damage window (tip‑based contact region).
  - Wave completion → credit bagged materials → open `Shop` → next wave prep.
  - Render world, pickups, UI, and Shop overlay.

Key constants and fields:
- `WINDOW_WIDTH=1920`, `WINDOW_HEIGHT=1080`
- `spawnTelegraphSeconds=2.0`
- Wave: starts at 20s, +5s per wave, capped at 60s.
- Materials on map capped at 50; overflow goes to material bag and credited on wave end.
- Boss spawning: FractalBoss at wave 8+, MiniBoss swarms at wave 5+, regular bosses at wave 3+.

### `Player`
- Stats (`PlayerStats`): maxHealth, moveSpeed, pickupRange, attackSpeed, damage, range, armor, healthRegen, dodgeChance, luck, materials.
- Starts with: Melee Stick (Tier 1) and Pistol (Tier 1).
- Movement via WASD/Arrows; aim points to mouse cursor.
- Leveling: XP orbs increase `experience`; level‑up raises `maxHealth` (+1 per current implementation) and grants test weapons at certain levels.
- Health: damage reduced by armor; dodge chance check; optional regen over time.
- Weapons: up to 6; positioned around player in a circle; all aim at mouse.

### `Weapon`
- Types: `PISTOL`, `SMG`, `MELEE_STICK`, `SHOTGUN`, `SNIPER`, `ORBITING_BRICK` (with tiers I–IV affecting stats).
- Ranged: automatic fire when cooldown elapses; SMG adds random spread; crits supported.
- Shotgun: fires 5 pellets with ±15° spread, each can crit independently.
- Sniper: high damage, slow fire rate, high crit chance (25%) and multiplier (3.0x).
- Melee: time‑boxed extension/retraction arc; damage handled in `Game::checkMeleeAttacks()` around the computed tip position.
- Orbiting: `ORBITING_BRICK` rotates around player dealing continuous damage.
- Assets: loaded per type/tier from `assets/weapons/…`.

### `Bullet`
- Position, direction, speed, radius, max range, damage, ownership (enemy or player).
- Types: `PISTOL`, `SMG`, `ENEMY_LOB`, `SHOTGUN`, `SNIPER`; current enemies use straight shots.
- Lifetime ends on range cap, bounds exit, or collision.

### `Enemy` and Variants
- Base `Enemy`: simple homing toward player; idle/hit animation using sprites in `monsters/…`.
- `SlimeEnemy`: slower, fires single straight shots at intervals, green look from `assets/enemies/slime.png`.
- `PebblinEnemy`: tanky, slow, occasionally fires 3‑shot spread.
- `BossEnemy`: advanced AI with multiple attack patterns, higher health, special abilities.
- `MiniBossEnemy`: swarm boss with 5 variants, leader-follower mechanics, coordinated attacks.
- `FractalBoss`: composite boss with fractal structure, each node can shoot independently, dynamic health system.
- Spawning is telegraphed with a red blinking X for `spawnTelegraphSeconds` before the enemy appears.

### Pickups
- `ExperienceOrb`: green glow with bobbing; expires after ~30s; collection adds XP.
- `Material`: currency token with bobbing; expires after ~60s; collection adds materials and a bit of XP. If too many exist on the map, extra drops are diverted into the "material bag" and paid at wave end.
- `SpeedUpBooster`: temporary speed boost pickup, spawns every 10 seconds, provides speed increase for limited duration.

### `Shop`
- Opens after each wave completes and the material bag is credited.
- Generates up to 4 weapon offers based on wave and random tier unlocks (Tier 2+ unlocks over waves; Tier 4 at wave ≥ 8).
- Prices scale with tier and wave; reroll cost increases within a wave (`R` key or UI button). Items can be locked to persist across rerolls.
- Input: number keys 1–4 or mouse click to buy; `ESC` to close.
- Shows a side panel with live player stats and owned weapons grid.
- New weapons: Shotgun (12-48 materials), Sniper (25-100 materials) with tier-based pricing.

## Data Flow and Interactions

- Input: `SDL_GetKeyboardState` and `SDL_GetMouseState` inside `Game::handleEvents()`.
- Player → Weapons: `Player::updateWeapons` computes weapon positions, calls `Weapon::update`, which may emit bullets.
- Enemies: each updates toward player and may emit bullets (slime/pebblin variants).
- Bosses: advanced AI patterns, coordinated attacks, fractal structure management.
- Collisions: distance checks between circles (player/enemy radii; bullet radii). Player bullets destroy enemies on hit; enemy bullets damage player; enemies contacting player deal contact damage.
- Pickups: proximity to player within `pickupRange` collects Materials/XP/Boosters.

## Rendering & UI

- Renderer: `SDL_Renderer` accelerated; background tan color.
- Sprites: enemies and weapons use textures if present; otherwise fallback primitives.
- UI (Brotato‑style):
  - Top‑left: health bar "X / Y"; LV.
  - Top‑left circle: materials count.
  - Top‑center: wave number; below it a countdown timer.
  - Bottom: XP progress bar using formula based on `(level+3)^2` thresholds.
- Text: prefers TTF via `assets/fonts/default.ttf`, falls back to Windows fonts, else custom bitmap digits/letters.

## Spawning, Waves, and Difficulty

- Spawn cadence: dynamic `spawnRate = max(0.2, 1.0 - 0.1*wave)`; telegraphed before actual spawn.
- Composition: wave 1 spawns base/slimes; wave ≥ 2 mixes slimes and pebblins.
- Boss spawning:
  - Wave 3+: Regular BossEnemy with advanced AI
  - Wave 5+: MiniBoss swarms with 5 variants and leader mechanics
  - Wave 8+: FractalBoss with complex fractal structure
- Materials drop chance: `1.0 - 0.015*(wave-1)` clamped to ≥ 0.5.
- Wave end: credit bagged materials → open Shop → increment wave → increase `waveDuration` by 5s up to 60s.

## Boss Systems

### FractalBoss
- Composite boss with fractal square structure (3 levels deep, 10 total nodes)
- Each node can shoot independently at the player
- Dynamic health system: damage to any node affects overall boss health
- Rotating structure with configurable rotation speed
- Nodes can be destroyed individually, affecting boss capabilities

### MiniBoss Swarm
- 5 variant mini-bosses with coordinated attacks
- Leader-follower mechanics for group behavior
- Different attack patterns and health values per variant
- Swarm spawning with telegraph indicators

### Regular BossEnemy
- Advanced AI with multiple attack patterns
- Higher health and damage than regular enemies
- Special abilities and movement patterns

## Error Handling

- SDL init failures and asset loads log to console and gracefully continue where possible (e.g., fallback rendering when textures are missing).
- Shop and UI guard against null resources; missing textures degrade visuals only.

## Performance Considerations

- Simple O(n) iteration over bullets/enemies/pickups each frame; entity counts in MVP remain small.
- Uses primitive rendering when assets not available; avoids expensive overdraw.
- Fixed `SDL_Delay(16)` to target ~60 FPS; consider adaptive timing for future refinement.
- Boss systems optimized for reasonable performance with complex structures.

## Extensibility Guide

### Add a New Enemy Type
1. Create `NewEnemy.h/.cpp` inheriting from `Enemy`.
2. Implement `update` (movement + firing) and `render` (texture or primitive).
3. Add a factory helper (e.g., `CreateNewEnemy`) and integrate selection into `Game::spawnEnemies()` and telegraph handling.
4. Add assets under `assets/enemies/…` and wire up `CMakeLists.txt` if new directories are needed.

### Add a New Boss Type
1. Create `NewBoss.h/.cpp` inheriting from `Enemy` or `BossEnemy`.
2. Implement complex AI patterns, special abilities, and unique mechanics.
3. Add boss spawning logic in `Game::spawnEnemies()` with appropriate wave conditions.
4. Integrate with the existing boss management system.

### Add a New Weapon
1. Extend `WeaponType` and stats initialization in `Weapon`.
2. Implement special `fire` logic if needed; reuse crit/scaling helpers.
3. Add textures in `assets/weapons/…` and load them in `loadWeaponTexture`.
4. Update `Shop` to surface the new weapon (names, icons, pricing).

### Add a New Pickup
1. Create an entity modeled after `ExperienceOrb`/`Material`/`SpeedUpBooster`.
2. Update `Game` to spawn and update it; add collection logic near `updateMaterialCollection()`.

### Add UI Elements
1. Extend `Game::renderUI()` or add a dedicated UI module.
2. Prefer TTF rendering if available; fallback to bitmap for consistency.

### Balance and Tuning
- Wave lengths, spawn rates, drop chances, material caps, crit parameters, melee windows, boss spawning conditions, and weapon stats are all isolated in one of: `Game`, `Weapon`, or per‑enemy implementations for easy tuning.

## Known Gaps vs Intended Design

- Bullet piercing for Pistol is mentioned but not yet implemented in collisions (currently first enemy hit destroys the bullet).
- `BulletType::ENEMY_LOB` physics is present but enemies currently fire straight shots; hook up lob shots if desired.
- Player level‑up effects are minimal and scripted for testing; a proper choice UI is planned via Shop/upgrades.
- Spawn timing uses a fixed `+0.016f` increment rather than `deltaTime`; consider using `deltaTime` for accuracy.
- FractalBoss node destruction visual effects could be enhanced.
- MiniBoss swarm coordination could be improved with more sophisticated group AI.

## Asset Pipeline

- At build, `assets/` and `monsters/` are copied next to the executable. The game loads by relative paths at runtime.
- Missing assets fall back to primitive rendering to avoid crashes.
- New weapon sprites and boss textures have been added to support expanded weapon and enemy systems.

## Testing and Troubleshooting

- Build issues: verify vcpkg integration; ensure SDL2 packages installed; see `TROUBLESHOOTING.md`.
- Runtime: check console logs for missing textures or TTF failures; expect bitmap fallback.
- Performance: try Release config (`cmake --build . --config Release`).
- Boss systems: monitor performance with complex fractal structures and multiple boss types.

## Appendix A — Key APIs

- `Game::run()`: main loop; calls `handleEvents()`, `update(dt)`, `render()`.
- `Player::updateWeapons(dt, bullets)`: positions weapons and lets them fire.
- `Weapon::update(...)`: fires on cooldown; melee uses `muzzleFlashTimer` as attack window.
- `Game::checkMeleeAttacks()`: resolves melee damage via weapon tip radius.
- `Shop::openShop(wave)`: generates items; `Shop::buyItem(i, player)` equips the weapon and deducts materials.
- `FractalBoss::update(...)`: manages fractal structure, node updates, and coordinated attacks.
- `MiniBossEnemy::update(...)`: handles swarm behavior and leader-follower mechanics.

## Appendix B — Sequence: Wave Completion with Boss Systems

```mermaid
sequenceDiagram
  participant Game
  participant Player
  participant Shop
  participant Bosses

  Game->>Game: waveTimer >= waveDuration
  alt Materials on map exceeded cap earlier
    Game->>Game: credit materialBag to Player
  else
    Note over Game: Uncollected Materials expire → credited to bag on cleanup
  end
  Game->>Shop: openShop(currentWave)
  Shop-->>Player: purchase weapons (deduct materials)
  Shop-->>Game: closeShop()
  Game->>Game: wave++ and waveDuration += 5s (cap 60s)
  Game->>Bosses: check boss spawning conditions
  alt Wave 3+
    Game->>Bosses: spawn BossEnemy
  alt Wave 5+
    Game->>Bosses: spawn MiniBoss swarm
  alt Wave 8+
    Game->>Bosses: spawn FractalBoss
  end
  Game->>Game: reset timers; resume spawning
```

## Appendix C — New Weapon Types

### Shotgun (SHOTGUN)
- **Damage**: 3-6 per pellet (tier-based)
- **Pellets**: 5 per shot
- **Spread**: ±15° (30° total)
- **Fire Rate**: 1.2-1.5 seconds
- **Range**: 300 pixels
- **Special**: Each pellet can crit independently

### Sniper Rifle (SNIPER)
- **Damage**: 25-60 (tier-based)
- **Fire Rate**: 2.0 seconds (fixed)
- **Range**: 600 pixels
- **Crit Chance**: 25%
- **Crit Multiplier**: 3.0x
- **Special**: High precision, long range

### Orbiting Brick (ORBITING_BRICK)
- **Damage**: 8-26 (tier-based)
- **Orbit Radius**: 70-96 pixels
- **Angular Speed**: 2.5-3.4 rad/sec
- **Special**: Continuous damage while orbiting player

---

Version: 2.0  
Last Updated: 2025‑01‑27


