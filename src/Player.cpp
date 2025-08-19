#include "Player.h"
#include <cmath>
#include <iostream>
#include <SDL2/SDL_image.h>

Player::Player(float x, float y) 
    : position(x, y), velocity(0, 0), shootDirection(1, 0), 
      radius(20), health(100), shield(100), shootCooldown(0.15f), timeSinceLastShot(0),
      experience(0), level(1), healthRegenTimer(0), healthRegenAccumulator(0.0f), playerTexture(nullptr) {
    // Initialize health to match max health
    health = stats.maxHealth;
    shield = stats.maxShield; // Инициализируем щит
    
    // Start with a brick on stick melee weapon for testing
    addWeapon(std::make_unique<Weapon>(WeaponType::MELEE_STICK, WeaponTier::TIER_1));
    // Also add a pistol for comparison
    addWeapon(std::make_unique<Weapon>(WeaponType::PISTOL, WeaponTier::TIER_1));
    // Add orbiting brick weapon
    addWeapon(std::make_unique<Weapon>(WeaponType::ORBITING_BRICK, WeaponTier::TIER_1));
}

void Player::initialize(SDL_Renderer* renderer) {
    // Load brick character sprite
    SDL_Surface* surface = IMG_Load("assets/character/brick.png");
    if (!surface) {
        std::cout << "Failed to load brick.png! SDL_image Error: " << IMG_GetError() << std::endl;
        return;
    }
    
    playerTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!playerTexture) {
        std::cout << "Failed to create texture from brick.png! SDL Error: " << SDL_GetError() << std::endl;
    } else {
        std::cout << "Successfully loaded brick character sprite!" << std::endl;
    }
}

void Player::update(float deltaTime) {
    position += velocity * deltaTime;
    
    if (position.x < radius) position.x = radius;
    if (position.x > 1920 - radius) position.x = 1920 - radius;
    if (position.y < radius) position.y = radius;
    if (position.y > 1080 - radius) position.y = 1080 - radius;
    
    velocity = Vector2(0, 0);
    
    timeSinceLastShot += deltaTime;

    // Update temporary fire rate boost timer
    if (fireRateBoostRemaining > 0.0f) {
        fireRateBoostRemaining -= deltaTime;
        if (fireRateBoostRemaining <= 0.0f) {
            fireRateBoostRemaining = 0.0f;
            fireRateMultiplier = 1.0f;
        }
    }
    
    // Health regeneration
    if (stats.healthRegen > 0) {
        healthRegenTimer += deltaTime;
        if (healthRegenTimer >= 1.0f) { // Regen every second
            // Add health regeneration to accumulator
            healthRegenAccumulator += stats.healthRegen;
            
            // Apply whole HP points and keep fractional part
            int healthToAdd = (int)healthRegenAccumulator;
            if (healthToAdd > 0) {
                health += healthToAdd;
                if (health > stats.maxHealth) health = stats.maxHealth;
                healthRegenAccumulator -= healthToAdd; // Keep fractional part
                

                

            }
            
            healthRegenTimer = 0.0f;
        }
    }
}

void Player::render(SDL_Renderer* renderer) {
    int centerX = (int)position.x;
    int centerY = (int)position.y;
    
    if (playerTexture) {
        // Get texture dimensions
        int textureWidth, textureHeight;
        SDL_QueryTexture(playerTexture, nullptr, nullptr, &textureWidth, &textureHeight);
        
        // Scale the brick sprite appropriately
        float scale = 0.8f; // Adjust size as needed
        int scaledWidth = (int)(textureWidth * scale);
        int scaledHeight = (int)(textureHeight * scale);
        
        // Create destination rectangle centered on player position
        SDL_Rect destRect = {
            centerX - scaledWidth / 2,
            centerY - scaledHeight / 2,
            scaledWidth,
            scaledHeight
        };
        
        // Render the brick sprite
        SDL_RenderCopy(renderer, playerTexture, nullptr, &destRect);
    } else {
        // Fallback to orange circle if texture fails to load
        SDL_SetRenderDrawColor(renderer, 255, 200, 100, 255);
        int r = (int)radius;
        
        for (int x = -r; x <= r; x++) {
            for (int y = -r; y <= r; y++) {
                if (x*x + y*y <= r*r) {
                    SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
                }
            }
        }
    }
    
    // Remove the orange direction line - user doesn't want it
    // SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
    // SDL_RenderDrawLine(renderer, centerX, centerY, 
    //                   centerX + shootDirection.x * 30, 
    //                   centerY + shootDirection.y * 30);
    
    // Draw pickup range indicator (faint circle)
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 30);
    int pickupR = (int)stats.pickupRange;
    for (int angle = 0; angle < 360; angle += 10) {
        float rad = angle * M_PI / 180.0f;
        int x1 = centerX + cos(rad) * pickupR;
        int y1 = centerY + sin(rad) * pickupR;
        SDL_RenderDrawPoint(renderer, x1, y1);
    }
}

void Player::handleInput(const Uint8* keyState) {
    float currentSpeed = stats.moveSpeed;
    
    if (keyState[SDL_SCANCODE_W] || keyState[SDL_SCANCODE_UP]) {
        velocity.y = -currentSpeed;
    }
    if (keyState[SDL_SCANCODE_S] || keyState[SDL_SCANCODE_DOWN]) {
        velocity.y = currentSpeed;
    }
    if (keyState[SDL_SCANCODE_A] || keyState[SDL_SCANCODE_LEFT]) {
        velocity.x = -currentSpeed;
    }
    if (keyState[SDL_SCANCODE_D] || keyState[SDL_SCANCODE_RIGHT]) {
        velocity.x = currentSpeed;
    }
}

void Player::updateShootDirection(const Vector2& mousePosition) {
    Vector2 direction = mousePosition - position;
    float length = direction.length();
    
    // Only update direction if mouse is not exactly on the player
    if (length > 0.1f) {
        shootDirection = direction.normalized();
    }
}

void Player::gainExperience(int exp) {
    experience += exp;
    
    // Check for level up
    while (experience >= getExperienceToNextLevel()) {
        levelUp();
    }
}

void Player::levelUp() {
    level++;
    std::cout << "Level up! Now level " << level << std::endl;
    
    // Brotato-style: +1 Max HP per level
    stats.maxHealth += 1;
    
    // Add weapons at certain levels for testing
    if (level == 2 && weapons.size() < MAX_WEAPONS) {
        addWeapon(std::make_unique<Weapon>(WeaponType::MELEE_STICK, WeaponTier::TIER_1));
        std::cout << "Got Brick on Stick!" << std::endl;
    } else if (level == 3 && weapons.size() < MAX_WEAPONS) {
        addWeapon(std::make_unique<Weapon>(WeaponType::SMG, WeaponTier::TIER_1));
        std::cout << "Got SMG!" << std::endl;
    } else if (level == 4 && weapons.size() < MAX_WEAPONS) {
        addWeapon(std::make_unique<Weapon>(WeaponType::SHOTGUN, WeaponTier::TIER_1));
        std::cout << "Got Shotgun!" << std::endl;
    } else if (level == 5 && weapons.size() < MAX_WEAPONS) {
        addWeapon(std::make_unique<Weapon>(WeaponType::SNIPER, WeaponTier::TIER_1));
        std::cout << "Got Sniper Rifle!" << std::endl;
    }
    
    // In Brotato, upgrades are chosen by the player at wave end
    // For now, we'll apply small automatic upgrades
    // TODO: Implement upgrade selection system
    
    // Don't heal on level up in Brotato
}

int Player::getExperienceToNextLevel() const {
    // Brotato's experience formula: (Level + 3) * (Level + 3)
    int nextLevel = level + 1;
    return (nextLevel + 3) * (nextLevel + 3);
}

void Player::takeDamage(int damage) {
    // Apply armor reduction
    int actualDamage = damage - stats.armor;
    if (actualDamage < 1) actualDamage = 1; // Always take at least 1 damage
    
    // Check dodge chance
    if (stats.dodgeChance > 0) {
        float dodgeRoll = (rand() % 100) / 100.0f;
        if (dodgeRoll < stats.dodgeChance / 100.0f) {
            std::cout << "Dodged!" << std::endl;
            return; // Dodged the attack
        }
    }
    
    // First damage goes to shield, then to health
    if (shield > 0) {
        if (shield >= actualDamage) {
            shield -= actualDamage;
            actualDamage = 0;
        } else {
            actualDamage -= shield;
            shield = 0;
        }
    }
    
    // Remaining damage goes to health
    if (actualDamage > 0) {
        health -= actualDamage;
        if (health < 0) health = 0;
    }
}

void Player::heal(int amount) {
    health += amount;
    if (health > stats.maxHealth) {
        health = stats.maxHealth;
    }
}

void Player::takeShieldDamage(int damage) {
    shield -= damage;
    if (shield < 0) shield = 0;
}

void Player::restoreShield(int amount) {
    shield += amount;
    if (shield > stats.maxShield) {
        shield = stats.maxShield;
    }
}

bool Player::canShoot() const {
    // Apply both player attackSpeed stat and temporary fireRateMultiplier
    float effectiveAttackSpeed = stats.attackSpeed * fireRateMultiplier;
    return timeSinceLastShot >= (shootCooldown / effectiveAttackSpeed);
}

void Player::shoot() {
    timeSinceLastShot = 0;
}

void Player::applyFireRateBoost(float multiplier, float durationSeconds) {
    // If a boost is already active, refresh duration and take the higher multiplier
    fireRateMultiplier = std::max(fireRateMultiplier, multiplier);
    fireRateBoostRemaining = durationSeconds;
}

void Player::addWeapon(std::unique_ptr<Weapon> weapon) {
    if (weapons.size() < MAX_WEAPONS) {
        weapons.push_back(std::move(weapon));
    }
}

void Player::addWeapon(std::unique_ptr<Weapon> weapon, SDL_Renderer* renderer) {
    if (weapons.size() < MAX_WEAPONS) {
        weapon->initialize(renderer);
        weapons.push_back(std::move(weapon));
    }
}

void Player::initializeWeapons(SDL_Renderer* renderer) {
    for (auto& weapon : weapons) {
        weapon->initialize(renderer);
    }
}

void Player::updateWeapons(float deltaTime, std::vector<std::unique_ptr<Bullet>>& bullets) {
    if (weapons.empty()) return;
    
    // Calculate positioning for multiple weapons
    int numWeapons = weapons.size();
    float circleRadius = 50.0f;
    for (int i = 0; i < numWeapons; i++) {
        Vector2 weaponPos;
        if (weapons[i]->isOrbitingWeapon()) {
            weaponPos = weapons[i]->getOrbitingPosition(position);
        } else {
            float angleOffset = (2.0f * M_PI * i) / numWeapons;
            float positionAngle = angleOffset;
            Vector2 offsetDirection(cos(positionAngle), sin(positionAngle));
            weaponPos = position + offsetDirection * circleRadius;
        }
        weapons[i]->update(deltaTime, weaponPos, shootDirection, bullets, *this);
    }
}

void Player::renderWeapons(SDL_Renderer* renderer) {
    if (weapons.empty()) return;
    
    // Calculate positions and render
    int numWeapons = weapons.size();
    float circleRadius = 50.0f;
    for (int i = 0; i < numWeapons; i++) {
        Vector2 weaponPos;
        if (weapons[i]->isOrbitingWeapon()) {
            weaponPos = weapons[i]->getOrbitingPosition(position);
        } else {
            float angleOffset = (2.0f * M_PI * i) / numWeapons;
            float positionAngle = angleOffset;
            Vector2 offsetDirection(cos(positionAngle), sin(positionAngle));
            weaponPos = position + offsetDirection * circleRadius;
        }
        Vector2 weaponDirection = shootDirection;
        weapons[i]->render(renderer, weaponPos, weaponDirection);
    }
}