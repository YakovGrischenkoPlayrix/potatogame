# Code Patterns and Implementation Guidelines

## Timer-Based Effects Implementation

### Delta Time Pattern
All time-based effects use consistent delta time accumulation:

```cpp
// Standard timer pattern
float timer = 0.0f;

void update(float deltaTime) {
    timer += deltaTime;
    if (timer >= threshold) {
        // Execute timed action
        timer = 0.0f; // or timer -= threshold for continuous
    }
}
```

### Common Timer Examples
```cpp
// Health regeneration (1-second intervals)
healthRegenTimer += deltaTime;
if (healthRegenTimer >= 1.0f) {
    health += (int)stats.healthRegen;
    if (health > stats.maxHealth) health = stats.maxHealth;
    healthRegenTimer = 0.0f;
}

// Weapon cooldowns
timeSinceLastShot += deltaTime;
bool canFire = timeSinceLastShot >= (stats.attackSpeed);

// Animation timing
animationTimer += deltaTime;
if (animationTimer > frameDelay) {
    currentFrame = (currentFrame + 1) % totalFrames;
    animationTimer = 0.0f;
}

// Muzzle flash effects
muzzleFlashTimer = std::max(0.0f, muzzleFlashTimer - deltaTime);
bool showFlash = muzzleFlashTimer > 0.05f;
```

## Stat Application Patterns

### Safe Stat Checks
Always validate stats before applying effects:

```cpp
// Check if stat is meaningful before using
if (stats.healthRegen > 0) {
    // Apply regeneration logic
}

if (stats.dodgeChance > 0) {
    float dodgeRoll = (rand() % 100) / 100.0f;
    if (dodgeRoll < stats.dodgeChance / 100.0f) {
        return; // Dodged attack
    }
}

// Ensure minimum values
int actualDamage = damage - stats.armor;
if (actualDamage < 1) actualDamage = 1; // Always deal at least 1 damage
```

### Stat Modifiers
```cpp
// Multiplicative scaling
float finalSpeed = baseSpeed * stats.attackSpeed;
float finalDamage = baseDamage * stats.damageMultiplier;

// Additive bonuses
int totalDamage = stats.baseDamage + stats.damage;
int maxHealth = baseHealth + stats.maxHealth;

// Percentage-based calculations
float critRoll = randomFloat(0.0f, 1.0f);
if (critRoll < stats.critChance) {
    damage = (int)(damage * stats.critMultiplier);
}
```

## Resource Management Patterns

### Smart Pointer Usage
Consistent RAII with unique_ptr for automatic cleanup:

```cpp
// Entity collections
std::vector<std::unique_ptr<Enemy>> enemies;
std::vector<std::unique_ptr<Bullet>> bullets;
std::vector<std::unique_ptr<Weapon>> weapons;

// Creation
enemies.push_back(std::make_unique<Enemy>(position, renderer));
weapons.push_back(std::make_unique<Weapon>(WeaponType::PISTOL, WeaponTier::TIER_1));

// Safe access
const Weapon* getWeapon(int index) const {
    if (index >= 0 && index < static_cast<int>(weapons.size())) {
        return weapons[index].get();
    }
    return nullptr;
}
```

### SDL Resource Management
```cpp
// Texture loading with error handling
SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cout << "Failed to load " << path << ": " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Always free surface
    
    if (!texture) {
        std::cout << "Failed to create texture: " << SDL_GetError() << std::endl;
    }
    
    return texture;
}

// Destructor cleanup
~Player() {
    if (playerTexture) {
        SDL_DestroyTexture(playerTexture);
        playerTexture = nullptr;
    }
}
```

## Collision Detection Patterns

### Circle-Circle Collision
Standard pattern for entity collision:

```cpp
void checkCollisions() {
    for (auto& bullet : bullets) {
        if (!bullet->isAlive()) continue;
        
        for (auto& enemy : enemies) {
            if (bullet->isAlive() && enemy->isAlive()) {
                float distance = bullet->getPosition().distance(enemy->getPosition());
                if (distance < bullet->getRadius() + enemy->getRadius()) {
                    bullet->destroy();
                    enemy->hit();
                    enemy->destroy();
                }
            }
        }
    }
}
```

### Range-Based Collection
```cpp
void updateExperienceCollection() {
    Vector2 playerPos = player->getPosition();
    float pickupRange = player->getStats().pickupRange;
    
    for (auto& orb : experienceOrbs) {
        if (orb->isAlive()) {
            float distance = playerPos.distance(orb->getPosition());
            if (distance <= pickupRange) {
                player->gainExperience(orb->getExperienceValue());
                orb->collect();
            }
        }
    }
}
```

## Rendering and Update Loop Patterns

### Consistent Update Order
```cpp
void Game::update(float deltaTime) {
    // 1. Update player first
    player->update(deltaTime);
    
    // 2. Update weapons (depends on player)
    player->updateWeapons(deltaTime, bullets);
    
    // 3. Update projectiles
    for (auto& bullet : bullets) {
        bullet->update(deltaTime);
    }
    
    // 4. Update enemies (may create bullets)
    for (auto& enemy : enemies) {
        enemy->update(deltaTime, player->getPosition(), bullets);
    }
    
    // 5. Update collectibles
    for (auto& orb : experienceOrbs) {
        orb->update(deltaTime);
    }
    
    // 6. Handle interactions
    checkCollisions();
    updateExperienceCollection();
    
    // 7. Clean up dead entities
    cleanupEntities();
}
```

### Render Layer Organisation
```cpp
void Game::render() {
    // Clear background
    SDL_SetRenderDrawColor(renderer, 120, 110, 100, 255);
    SDL_RenderClear(renderer);
    
    // Background elements
    player->render(renderer);
    player->renderWeapons(renderer);
    
    // Projectiles
    for (auto& bullet : bullets) {
        bullet->render(renderer);
    }
    
    // Enemies
    for (auto& enemy : enemies) {
        enemy->render(renderer);
    }
    
    // Effects and indicators
    renderSpawnIndicators();
    
    // Collectibles
    for (auto& orb : experienceOrbs) {
        orb->render(renderer);
    }
    
    // UI on top
    renderUI();
    shop->render(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Present frame
    SDL_RenderPresent(renderer);
}
```

## Error Handling and Bounds Checking

### Safe Array Access
```cpp
// Check bounds before access
if (index >= 0 && index < static_cast<int>(weapons.size())) {
    return weapons[index].get();
}
return nullptr;

// Range-based loops for safety
for (const auto& weapon : weapons) {
    weapon->update(deltaTime);
}
```

### Null Pointer Checks
```cpp
// Check pointers before use
if (playerTexture) {
    SDL_RenderCopy(renderer, playerTexture, nullptr, &destRect);
} else {
    // Fallback rendering
    renderFallbackSprite(renderer);
}

// Check game references
if (gameRef) {
    int materials = gameRef->getPlayerMaterials();
}
```

### Value Validation
```cpp
// Clamp values to valid ranges
health = std::max(0, std::min(health, stats.maxHealth));
position.x = std::max(radius, std::min(position.x, WINDOW_WIDTH - radius));

// Validate before calculations
if (length > 0.1f) {
    shootDirection = direction.normalized();
}
```

## Entity Cleanup Patterns

### Remove-If Idiom
Standard pattern for removing dead entities:

```cpp
// Remove dead bullets
bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
    [](const std::unique_ptr<Bullet>& bullet) {
        return !bullet->isAlive();
    }), bullets.end());

// Remove dead enemies with additional processing
enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
    [&](const std::unique_ptr<Enemy>& enemy) {
        if (!enemy->isAlive()) {
            // Process death (drop materials, add score, etc.)
            handleEnemyDeath(enemy.get());
            return true;
        }
        return false;
    }), enemies.end());
```

### Lifetime Management
```cpp
// Entities manage their own lifetime
class ExperienceOrb {
    float lifetime;
    float maxLifetime;
    bool alive;
    
    void update(float deltaTime) {
        lifetime += deltaTime;
        if (lifetime >= maxLifetime) {
            alive = false; // Auto-expire
        }
    }
};
```

## State Management Patterns

### Enum-Based States
```cpp
enum class EnemyState {
    IDLE,
    HIT,
    ATTACKING
};

void Enemy::update(float deltaTime) {
    switch (state) {
        case EnemyState::IDLE:
            // Normal behaviour
            break;
        case EnemyState::HIT:
            hitTimer += deltaTime;
            if (hitTimer > HIT_DURATION) {
                state = EnemyState::IDLE;
                hitTimer = 0.0f;
            }
            break;
    }
}
```

### Game State Control
```cpp
void Game::update(float deltaTime) {
    // Check global state conditions
    if (player->getHealth() <= 0) {
        running = false;
        return;
    }
    
    // Don't update game if shop is active
    if (shop->isShopActive()) {
        return;
    }
    
    // Normal game update
    updateGameplay(deltaTime);
}
```

## Performance Patterns

### Early Exit Optimisations
```cpp
// Skip processing for inactive entities
if (!bullet->isAlive()) continue;
if (!enemy->isAlive()) continue;

// Skip expensive operations when not needed
if (weapons.empty()) return;
if (muzzleFlashTimer <= 0.0f) return;
```

### Efficient Vector Operations
```cpp
// Reserve capacity for known sizes
std::vector<SpawnIndicator> remaining;
remaining.reserve(spawnIndicators.size());

// Use const references to avoid copies
for (const auto& weapon : weapons) {
    weapon->render(renderer, weaponPos, weaponDirection);
}
```

### Batch Operations
```cpp
// Process similar operations together
void updateAllWeapons(float deltaTime) {
    for (int i = 0; i < numWeapons; i++) {
        Vector2 weaponPos = calculateWeaponPosition(i);
        weapons[i]->update(deltaTime, weaponPos, shootDirection, bullets, *this);
    }
}
```

## Input Handling Patterns

### Key State Management
```cpp
// Track key press states to prevent repeats
bool keyPressed[4] = {false};

void handleInput(const Uint8* keyState) {
    for (int i = 0; i < 4; i++) {
        SDL_Scancode key = (SDL_Scancode)(SDL_SCANCODE_1 + i);
        if (keyState[key] && !keyPressed[i]) {
            keyPressed[i] = true;
            handleKeyPress(i);
        } else if (!keyState[key]) {
            keyPressed[i] = false;
        }
    }
}
```

### Mouse Input Processing
```cpp
void handleMouseInput(int mouseX, int mouseY, bool mousePressed) {
    // Update aim direction
    Vector2 mousePos(static_cast<float>(mouseX), static_cast<float>(mouseY));
    player->updateShootDirection(mousePos);
    
    // Handle clicks with debouncing
    if (mousePressed && !lastMousePressed) {
        handleMouseClick(mouseX, mouseY);
    }
    lastMousePressed = mousePressed;
}
```

These patterns provide a consistent, maintainable foundation for implementing new features while following the established architectural principles of the codebase.
