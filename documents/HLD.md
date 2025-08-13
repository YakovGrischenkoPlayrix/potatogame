## Brotato MVP — High-Level Design (HLD)

This document explains the overall architecture, key systems, data flow, and extensibility of the Brotato-inspired MVP game implemented in C++17 with SDL2.

### Goals
- Deliver a playable top‑down survival shooter MVP with Brotato‑style loops.
- Keep systems small, readable, and easy to extend (new enemies, weapons, UI, etc.).
- Be buildable on Windows via CMake + vcpkg with SDL2, SDL2_image, SDL2_ttf.

### Non‑Goals
- Full parity with Brotato’s meta‑progression, itemization, and complex balancing.
- Highly generalized engine or data‑driven authoring tools.

## Project Structure

```
potatogame/
  assets/                 # runtime assets (copied to build dir)
  monsters/               # classic monster sprites (copied to build dir)
  src/                    # C++ sources
    Bullet.*              # Projectile model + rendering
    Enemy.*               # Base enemy with simple AI + animation
    SlimeEnemy.*          # Ranged slime (single shots)
    PebblinEnemy.*        # Tanky pebblin (triple spread)
    ExperienceOrb.*       # XP pickup entity
    Material.*            # Currency pickup entity
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
- Systems: Player input and movement, enemy spawn/AI, projectiles, pickups (XP/Materials), wave timer, end‑of‑wave shop, and UI rendering.

### Main Loop (simplified)

```mermaid
sequenceDiagram
  participant OS
  participant Game
  participant SDL
  participant Player
  participant Systems as Enemies/Bullets/Pickups/Shop

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
    Game->>Game: render()
    Game->>SDL: present + delay(16ms)
  end
  Game->>SDL: destroy renderer/window; quit subsystems
```

## Core Modules and Responsibilities

### `Game`
- Initializes SDL subsystems and resources.
- Owns `Player`, `Enemy` list, `Bullet` list, pickups (XP/Materials), `Shop`.
- Game state: score, wave index, wave timers, spawn telegraphers, material bag.
- Systems per frame:
  - Input routing (Shop vs Player), player/weapon updates, enemy updates.
  - Enemy spawn telegraphing and material/XP updates.
  - Collision checks (bullet↔enemy, enemy↔player, enemy bullets↔player).
  - Melee sweep damage window (tip‑based contact region).
  - Wave completion → credit bagged materials → open `Shop` → next wave prep.
  - Render world, pickups, UI, and Shop overlay.

Key constants and fields:
- `WINDOW_WIDTH=1920`, `WINDOW_HEIGHT=1080`
- `spawnTelegraphSeconds=2.0`
- Wave: starts at 20s, +5s per wave, capped at 60s.
- Materials on map capped at 50; overflow goes to material bag and credited on wave end.

### `Player`
- Stats (`PlayerStats`): maxHealth, moveSpeed, pickupRange, attackSpeed, damage, range, armor, healthRegen, dodgeChance, luck, materials.
- Starts with: Melee Stick (Tier 1) and Pistol (Tier 1).
- Movement via WASD/Arrows; aim points to mouse cursor.
- Leveling: XP orbs increase `experience`; level‑up raises `maxHealth` (+1 per current implementation) and grants test weapons at certain levels.
- Health: damage reduced by armor; dodge chance check; optional regen over time.
- Weapons: up to 6; positioned around player in a circle; all aim at mouse.

### `Weapon`
- Types: `PISTOL`, `SMG`, `MELEE_STICK` (with tiers I–IV affecting stats).
- Ranged: automatic fire when cooldown elapses; SMG adds random spread; crits supported.
- Melee: time‑boxed extension/retraction arc; damage handled in `Game::checkMeleeAttacks()` around the computed tip position.
- Assets: loaded per type/tier from `assets/weapons/…`.

### `Bullet`
- Position, direction, speed, radius, max range, damage, ownership (enemy or player).
- Types: `PISTOL`, `SMG`, `ENEMY_LOB` (parabolic, gravity); current enemies use straight shots.
- Lifetime ends on range cap, bounds exit, or collision.

### `Enemy` and Variants
- Base `Enemy`: simple homing toward player; idle/hit animation using sprites in `monsters/…`.
- `SlimeEnemy`: slower, fires single straight shots at intervals, green look from `assets/enemies/slime.png`.
- `PebblinEnemy`: tanky, slow, occasionally fires 3‑shot spread.
- Spawning is telegraphed with a red blinking X for `spawnTelegraphSeconds` before the enemy appears.

### Pickups
- `ExperienceOrb`: green glow with bobbing; expires after ~30s; collection adds XP.
- `Material`: currency token with bobbing; expires after ~60s; collection adds materials and a bit of XP. If too many exist on the map, extra drops are diverted into the “material bag” and paid at wave end.

### `Shop`
- Opens after each wave completes and the material bag is credited.
- Generates up to 4 weapon offers based on wave and random tier unlocks (Tier 2+ unlocks over waves; Tier 4 at wave ≥ 8).
- Prices scale with tier and wave; reroll cost increases within a wave (`R` key or UI button). Items can be locked to persist across rerolls.
- Input: number keys 1–4 or mouse click to buy; `ESC` to close.
- Shows a side panel with live player stats and owned weapons grid.

## Data Flow and Interactions

- Input: `SDL_GetKeyboardState` and `SDL_GetMouseState` inside `Game::handleEvents()`.
- Player → Weapons: `Player::updateWeapons` computes weapon positions, calls `Weapon::update`, which may emit bullets.
- Enemies: each updates toward player and may emit bullets (slime/pebblin variants).
- Collisions: distance checks between circles (player/enemy radii; bullet radii). Player bullets destroy enemies on hit; enemy bullets damage player; enemies contacting player deal contact damage.
- Pickups: proximity to player within `pickupRange` collects Materials/XP.

## Rendering & UI

- Renderer: `SDL_Renderer` accelerated; background tan color.
- Sprites: enemies and weapons use textures if present; otherwise fallback primitives.
- UI (Brotato‑style):
  - Top‑left: health bar “X / Y”; LV.
  - Top‑left circle: materials count.
  - Top‑center: wave number; below it a countdown timer.
  - Bottom: XP progress bar using formula based on `(level+3)^2` thresholds.
- Text: prefers TTF via `assets/fonts/default.ttf`, falls back to Windows fonts, else custom bitmap digits/letters.

## Spawning, Waves, and Difficulty

- Spawn cadence: dynamic `spawnRate = max(0.2, 1.0 - 0.1*wave)`; telegraphed before actual spawn.
- Composition: wave 1 spawns base/slimes; wave ≥ 2 mixes slimes and pebblins.
- Materials drop chance: `1.0 - 0.015*(wave-1)` clamped to ≥ 0.5.
- Wave end: credit bagged materials → open Shop → increment wave → increase `waveDuration` by 5s up to 60s.

## Error Handling

- SDL init failures and asset loads log to console and gracefully continue where possible (e.g., fallback rendering when textures are missing).
- Shop and UI guard against null resources; missing textures degrade visuals only.

## Performance Considerations

- Simple O(n) iteration over bullets/enemies/pickups each frame; entity counts in MVP remain small.
- Uses primitive rendering when assets not available; avoids expensive overdraw.
- Fixed `SDL_Delay(16)` to target ~60 FPS; consider adaptive timing for future refinement.

## Extensibility Guide

### Add a New Enemy Type
1. Create `NewEnemy.h/.cpp` inheriting from `Enemy`.
2. Implement `update` (movement + firing) and `render` (texture or primitive).
3. Add a factory helper (e.g., `CreateNewEnemy`) and integrate selection into `Game::spawnEnemies()` and telegraph handling.
4. Add assets under `assets/enemies/…` and wire up `CMakeLists.txt` if new directories are needed.

### Add a New Weapon
1. Extend `WeaponType` and stats initialization in `Weapon`.
2. Implement special `fire` logic if needed; reuse crit/scaling helpers.
3. Add textures in `assets/weapons/…` and load them in `loadWeaponTexture`.
4. Update `Shop` to surface the new weapon (names, icons, pricing).

### Add a New Pickup
1. Create an entity modeled after `ExperienceOrb`/`Material`.
2. Update `Game` to spawn and update it; add collection logic near `updateMaterialCollection()`.

### Add UI Elements
1. Extend `Game::renderUI()` or add a dedicated UI module.
2. Prefer TTF rendering if available; fallback to bitmap for consistency.

### Balance and Tuning
- Wave lengths, spawn rates, drop chances, material caps, crit parameters, and melee windows are all isolated in one of: `Game`, `Weapon`, or per‑enemy implementations for easy tuning.

## Known Gaps vs Intended Design

- Bullet piercing for Pistol is mentioned but not yet implemented in collisions (currently first enemy hit destroys the bullet).
- `BulletType::ENEMY_LOB` physics is present but enemies currently fire straight shots; hook up lob shots if desired.
- Player level‑up effects are minimal and scripted for testing; a proper choice UI is planned via Shop/upgrades.
- Spawn timing uses a fixed `+0.016f` increment rather than `deltaTime`; consider using `deltaTime` for accuracy.

## Asset Pipeline

- At build, `assets/` and `monsters/` are copied next to the executable. The game loads by relative paths at runtime.
- Missing assets fall back to primitive rendering to avoid crashes.

## Testing and Troubleshooting

- Build issues: verify vcpkg integration; ensure SDL2 packages installed; see `TROUBLESHOOTING.md`.
- Runtime: check console logs for missing textures or TTF failures; expect bitmap fallback.
- Performance: try Release config (`cmake --build . --config Release`).

## Appendix A — Key APIs

- `Game::run()`: main loop; calls `handleEvents()`, `update(dt)`, `render()`.
- `Player::updateWeapons(dt, bullets)`: positions weapons and lets them fire.
- `Weapon::update(...)`: fires on cooldown; melee uses `muzzleFlashTimer` as attack window.
- `Game::checkMeleeAttacks()`: resolves melee damage via weapon tip radius.
- `Shop::openShop(wave)`: generates items; `Shop::buyItem(i, player)` equips the weapon and deducts materials.

## Appendix B — Sequence: Wave Completion

```mermaid
sequenceDiagram
  participant Game
  participant Player
  participant Shop

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
  Game->>Game: reset timers; resume spawning
```

---

Version: 1.0  
Last Updated: 2025‑08‑12


