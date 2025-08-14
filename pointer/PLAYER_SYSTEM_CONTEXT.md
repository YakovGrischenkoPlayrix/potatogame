# Player System Context

## Player Stats Overview

The Player system is built around the `PlayerStats` struct which contains all Brotato-style character attributes:

### Core Stats
```cpp
struct PlayerStats {
    int maxHealth;        // Maximum health points (starts at 100, +1 per level)
    float moveSpeed;      // Movement speed in pixels/second (default: 200)
    float pickupRange;    // Range for collecting items in pixels (default: 50)
    float attackSpeed;    // Attack speed multiplier (default: 1.0f)
    int damage;          // Base damage for calculations (default: 10)
    float range;         // Attack range in pixels (default: 200)
    int armor;           // Damage reduction (default: 0)
    float healthRegen;   // Health regeneration per second (default: 0)
    float dodgeChance;   // Chance to avoid damage 0.0-1.0 (default: 0)
    int luck;            // Affects item generation (default: 0)
    int materials;       // Currency for shop purchases (starts at 50)
};
```

## Health System Implementation

### Health Management
- **Current Health**: Tracked separately from `maxHealth` in `Player::health`
- **Damage Processing**: Applied through `Player::takeDamage(int damage)`
- **Armor Calculation**: `actualDamage = damage - stats.armor` (minimum 1 damage)
- **Dodge System**: Percentage-based avoidance check before damage application
- **Death Condition**: Game ends when `health <= 0`

### Health Regeneration
```cpp
// Regeneration timer in Player::update()
if (stats.healthRegen > 0) {
    healthRegenTimer += deltaTime;
    if (healthRegenTimer >= 1.0f) { // Regen every second
        health += (int)stats.healthRegen;
        if (health > stats.maxHealth) health = stats.maxHealth;
        healthRegenTimer = 0.0f;
    }
}
```

## Timer System Details

### Core Timers
- **`healthRegenTimer`**: Tracks time for health regeneration (1-second intervals)
- **`timeSinceLastShot`**: Global player shooting cooldown
- **`shootCooldown`**: Base shooting speed (0.15f seconds, modified by `attackSpeed`)

### Timer Usage Patterns
```cpp
// Delta time accumulation
timeSinceLastShot += deltaTime;

// Cooldown checks
bool canShoot() const {
    return timeSinceLastShot >= (shootCooldown / stats.attackSpeed);
}

// Timer reset on action
void shoot() {
    timeSinceLastShot = 0;
}
```

## Weapon Management System

### Weapon Inventory
- **Capacity**: Maximum 6 weapons (Brotato standard)
- **Storage**: `std::vector<std::unique_ptr<Weapon>> weapons`
- **Addition**: `addWeapon()` methods with renderer initialisation
- **Access**: Const getters for safe weapon access

### Weapon Positioning
```cpp
// Circular positioning around player
int numWeapons = weapons.size();
float radius = 50.0f;

for (int i = 0; i < numWeapons; i++) {
    float angleOffset = (2.0f * M_PI * i) / numWeapons;
    Vector2 offsetDirection(cos(angleOffset), sin(angleOffset));
    Vector2 weaponPos = position + offsetDirection * radius;
    
    // All weapons aim toward mouse cursor
    weapons[i]->update(deltaTime, weaponPos, shootDirection, bullets, *this);
}
```

### Weapon Initialisation
- **Startup Weapons**: Player starts with Brick on Stick (melee) + Pistol
- **Level Rewards**: Automatic weapon grants at levels 2, 3, and 5
- **Shop Integration**: Weapons purchased through shop system

## Experience and Leveling Mechanics

### Experience Formula
Brotato-style exponential scaling:
```cpp
int getExperienceToNextLevel() const {
    int nextLevel = level + 1;
    return (nextLevel + 3) * (nextLevel + 3);
}
```

### Level Progression
- **Level 1 → 2**: 16 experience required
- **Level 2 → 3**: 25 experience required  
- **Level 3 → 4**: 36 experience required
- **Pattern**: `(level + 3)²` experience for next level

### Level Up Rewards
```cpp
void levelUp() {
    level++;
    stats.maxHealth += 1;  // +1 Max HP per level (Brotato standard)
    
    // Automatic weapon rewards for testing
    if (level == 2) addWeapon(MELEE_STICK);
    if (level == 3) addWeapon(SMG);
    if (level == 5) addWeapon(PISTOL, TIER_2);
}
```

## Key Methods and Their Purposes

### Input and Movement
```cpp
void handleInput(const Uint8* keyState);
// - Processes WASD/arrow keys for movement
// - Sets velocity based on moveSpeed stat
// - No diagonal speed penalty

void updateShootDirection(const Vector2& mousePosition);
// - Calculates aim direction from player to mouse
// - Updates shootDirection for all weapons
// - Normalises direction vector
```

### Combat Integration
```cpp
void updateWeapons(float deltaTime, std::vector<std::unique_ptr<Bullet>>& bullets);
// - Updates all equipped weapons
// - Passes player reference for damage calculations
// - Manages bullet creation and positioning

void takeDamage(int damage);
// - Applies armor reduction
// - Checks dodge chance
// - Updates health and handles death
```

### Progression System
```cpp
void gainExperience(int exp);
// - Adds experience points
// - Triggers level up when threshold reached
// - Handles multiple level ups in one call

void gainMaterials(int amount);
// - Adds materials for shop purchases
// - Simple accumulation (no limits)
```

### Resource Collection
```cpp
// Automatic collection in Game::updateExperienceCollection()
Vector2 playerPos = player->getPosition();
float pickupRange = player->getStats().pickupRange;

for (auto& orb : experienceOrbs) {
    float distance = playerPos.distance(orb->getPosition());
    if (distance <= pickupRange) {
        player->gainExperience(orb->getExperienceValue());
        orb->collect();
    }
}
```

## Rendering System

### Player Sprite
- **Texture**: `assets/character/brick.png`
- **Scaling**: 0.8x scale for appropriate size
- **Fallback**: Orange circle if texture fails
- **Centering**: Sprite centered on player position

### Visual Indicators
```cpp
// Pickup range indicator (faint green circle)
SDL_SetRenderDrawColor(renderer, 100, 255, 100, 30);
int pickupR = (int)stats.pickupRange;
for (int angle = 0; angle < 360; angle += 10) {
    float rad = angle * M_PI / 180.0f;
    int x1 = centerX + cos(rad) * pickupR;
    int y1 = centerY + sin(rad) * pickupR;
    SDL_RenderDrawPoint(renderer, x1, y1);
}
```

## Boundary Constraints

### Screen Bounds
```cpp
// Applied in Player::update()
if (position.x < radius) position.x = radius;
if (position.x > 1920 - radius) position.x = 1920 - radius;
if (position.y < radius) position.y = radius;
if (position.y > 1080 - radius) position.y = 1080 - radius;
```

### Movement Physics
- **Velocity Reset**: Velocity zeroed each frame, set by input
- **No Momentum**: Instant acceleration/deceleration
- **Frame Independence**: Movement scaled by `deltaTime`

## Integration with Other Systems

### Weapon System
- Player stats affect weapon damage calculations
- Player position used for weapon positioning
- Player aim direction controls all weapons

### Shop System
- Materials spent on purchases
- Weapon capacity checked before adding
- Stats displayed in shop interface

### Game System
- Health checked for game over condition
- Position used for collision detection
- Stats accessed for UI display

### Enemy System
- Player position used for enemy AI targeting
- Damage received from enemy collisions
- Pickup range affects resource collection

## Common Usage Patterns

### Stat Modifications
```cpp
// Always check if stat > 0 before applying effects
if (stats.healthRegen > 0) {
    // Apply regeneration
}

// Use stats as multipliers
float finalSpeed = baseSpeed * stats.attackSpeed;
```

### Safe Weapon Access
```cpp
const Weapon* getWeapon(int index) const {
    if (index >= 0 && index < static_cast<int>(weapons.size())) {
        return weapons[index].get();
    }
    return nullptr;
}
```

### Resource Management
```cpp
// RAII with smart pointers
std::vector<std::unique_ptr<Weapon>> weapons;

// Automatic cleanup in destructor
if (playerTexture) {
    SDL_DestroyTexture(playerTexture);
}
```

This player system provides a robust foundation for character progression and combat mechanics, following Brotato's design principles while maintaining clean, extensible code architecture.
