#include "Weapon.h"
#include "Enemy.h"
#include "Player.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <SDL2/SDL_image.h>
#include <iostream>

Weapon::Weapon(WeaponType weaponType, WeaponTier weaponTier) 
    : type(weaponType), tier(weaponTier), timeSinceLastShot(0.0f), 
      muzzleFlashTimer(0.0f), lastShotDirection(1, 0), weaponTexture(nullptr) {
    
    // Initialize stats based on weapon type and tier
    switch (type) {
        case WeaponType::PISTOL:
            initializePistolStats();
            break;
        case WeaponType::SMG:
            initializeSMGStats();
            break;
        case WeaponType::MELEE_STICK:
            initializeMeleeStickStats();
            break;
    }
}

Weapon::~Weapon() {
    if (weaponTexture) {
        SDL_DestroyTexture(weaponTexture);
        weaponTexture = nullptr;
    }
}

void Weapon::initialize(SDL_Renderer* renderer) {
    loadWeaponTexture(renderer);
}

void Weapon::loadWeaponTexture(SDL_Renderer* renderer) {
    std::string texturePath;
    
    switch (type) {
        case WeaponType::PISTOL:
            // Use different pistol sprites based on tier
            switch (tier) {
                case WeaponTier::TIER_1:
                    texturePath = "assets/weapons/pistol.png";
                    break;
                case WeaponTier::TIER_2:
                    texturePath = "assets/weapons/pistol2.png";
                    break;
                case WeaponTier::TIER_3:
                case WeaponTier::TIER_4:
                    texturePath = "assets/weapons/pistol3.png";
                    break;
            }
            break;
        case WeaponType::SMG:
            texturePath = "assets/weapons/smg.png";
            break;
        case WeaponType::MELEE_STICK:
            texturePath = "assets/weapons/brickonstick.png";
            break;
        default:
            texturePath = "assets/weapons/pistol.png";
            break;
    }
    
    SDL_Surface* surface = IMG_Load(texturePath.c_str());
    if (!surface) {
        std::cout << "Failed to load weapon texture: " << texturePath << " - " << IMG_GetError() << std::endl;
        return;
    }
    
    weaponTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!weaponTexture) {
        std::cout << "Failed to create weapon texture: " << SDL_GetError() << std::endl;
    }
}

void Weapon::initializePistolStats() {
    // Pistol stats based on tier
    switch (tier) {
        case WeaponTier::TIER_1:
            stats.baseDamage = 12;
            stats.attackSpeed = 1.2f;
            stats.critChance = 0.05f;
            break;
        case WeaponTier::TIER_2:
            stats.baseDamage = 20;
            stats.attackSpeed = 1.12f;
            stats.critChance = 0.10f;
            break;
        case WeaponTier::TIER_3:
            stats.baseDamage = 30;
            stats.attackSpeed = 1.03f;
            stats.critChance = 0.15f;
            break;
        case WeaponTier::TIER_4:
            stats.baseDamage = 50;
            stats.attackSpeed = 0.87f;
            stats.critChance = 0.20f;
            break;
    }
    
    stats.range = 400;
    stats.critMultiplier = 2.0f;
    stats.knockback = 15;
    stats.rangedDamageScaling = 1.0f;
}

void Weapon::initializeSMGStats() {
    // SMG stats based on tier
    switch (tier) {
        case WeaponTier::TIER_1:
            stats.baseDamage = 3;
            stats.attackSpeed = 0.17f;
            break;
        case WeaponTier::TIER_2:
            stats.baseDamage = 4;
            stats.attackSpeed = 0.16f;
            break;
        case WeaponTier::TIER_3:
            stats.baseDamage = 6;
            stats.attackSpeed = 0.155f;
            break;
        case WeaponTier::TIER_4:
            stats.baseDamage = 8;
            stats.attackSpeed = 0.15f;
            break;
    }
    
    stats.range = 400;
    stats.critChance = 0.01f;
    stats.critMultiplier = 1.5f;
    stats.knockback = 0;
    stats.rangedDamageScaling = 1.0f;
}

void Weapon::initializeMeleeStickStats() {
    // Brick on Stick stats based on tier - Brotato style melee weapon
    switch (tier) {
        case WeaponTier::TIER_1:
            stats.baseDamage = 15; // Higher base damage than ranged weapons
            stats.attackSpeed = 0.8f; // Slower than SMG but faster than pistol
            break;
        case WeaponTier::TIER_2:
            stats.baseDamage = 25;
            stats.attackSpeed = 0.75f;
            break;
        case WeaponTier::TIER_3:
            stats.baseDamage = 40;
            stats.attackSpeed = 0.7f;
            break;
        case WeaponTier::TIER_4:
            stats.baseDamage = 65;
            stats.attackSpeed = 0.65f;
            break;
    }
    
    // Melee weapon characteristics
    stats.range = 80; // Short melee range
    stats.critChance = 0.08f; // Decent crit chance
    stats.critMultiplier = 2.5f; // High crit multiplier
    stats.knockback = 25; // Strong knockback
    stats.rangedDamageScaling = 0.0f; // No ranged scaling
    stats.meleeDamageScaling = 1.0f; // Scales with melee damage
}

void Weapon::update(float deltaTime, const Vector2& weaponPos, 
                   const Vector2& aimDirection,
                   std::vector<std::unique_ptr<Bullet>>& bullets,
                   const Player& player) {
    timeSinceLastShot += deltaTime;
    muzzleFlashTimer = std::max(0.0f, muzzleFlashTimer - deltaTime);

    // Effective fire interval accounts for player's attackSpeed stat and temporary fire-rate boosts
    float effectiveMultiplier = player.getStats().attackSpeed * player.getFireRateMultiplier();
    if (effectiveMultiplier < 0.1f) effectiveMultiplier = 0.1f; // safety clamp
    float requiredCooldown = stats.attackSpeed / effectiveMultiplier;

    // Fire in the direction player is aiming if ready
    if (timeSinceLastShot >= requiredCooldown) {
        fire(weaponPos, aimDirection, bullets, player);
        timeSinceLastShot = 0.0f;
        muzzleFlashTimer = 0.1f; // Show muzzle flash for 0.1 seconds
        lastShotDirection = aimDirection;
    }
}

void Weapon::render(SDL_Renderer* renderer, const Vector2& weaponPos, const Vector2& weaponDirection) {
    // Special rendering for melee weapons
    if (type == WeaponType::MELEE_STICK) {
        // Show weapon extending and retracting
        if (muzzleFlashTimer > 0.0f) {
            Vector2 weaponTip = getWeaponTipPosition(weaponPos, weaponDirection);
            
            // Draw the weapon as a thick line from player to current tip position
            SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255); // Brown color for stick
            
            // Draw multiple lines to make it thicker
            for (int offset = -2; offset <= 2; offset++) {
                Vector2 perpendicular(-weaponDirection.y, weaponDirection.x);
                Vector2 startPos = weaponPos + perpendicular * offset;
                Vector2 endPos = weaponTip + perpendicular * offset;
                
                SDL_RenderDrawLine(renderer, 
                                  (int)startPos.x, (int)startPos.y,
                                  (int)endPos.x, (int)endPos.y);
            }
            
            // Draw the brick at the tip
            SDL_SetRenderDrawColor(renderer, 160, 82, 45, 255); // Darker brown for brick
            int brickSize = 6;
            SDL_Rect brickRect = {
                (int)weaponTip.x - brickSize/2,
                (int)weaponTip.y - brickSize/2,
                brickSize,
                brickSize
            };
            SDL_RenderFillRect(renderer, &brickRect);
        }
        
        // Don't render the normal weapon texture for melee weapons during attack
        if (muzzleFlashTimer > 0.0f) {
            return;
        }
    }
    
    if (!weaponTexture) {
        // Fallback to line rendering if no texture
        SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
        Vector2 weaponEnd = weaponPos + weaponDirection * 15;
        SDL_RenderDrawLine(renderer, 
                          (int)weaponPos.x, (int)weaponPos.y,
                          (int)weaponEnd.x, (int)weaponEnd.y);
        return;
    }
    
    // Get texture dimensions
    int textureWidth, textureHeight;
    SDL_QueryTexture(weaponTexture, nullptr, nullptr, &textureWidth, &textureHeight);
    
    // Scale down the weapon sprite to much smaller size
    float scale = 0.33f;
    int scaledWidth = (int)(textureWidth * scale);
    int scaledHeight = (int)(textureHeight * scale);
    
    // Calculate rotation angle in degrees using weapon direction
    double angle = atan2(weaponDirection.y, weaponDirection.x) * 180.0 / M_PI;
    
    // Create destination rectangle
    SDL_Rect destRect = {
        (int)(weaponPos.x - scaledWidth / 2),
        (int)(weaponPos.y - scaledHeight / 2),
        scaledWidth,
        scaledHeight
    };
    
    // Render rotated weapon sprite
    SDL_RenderCopyEx(renderer, weaponTexture, nullptr, &destRect, angle, nullptr, SDL_FLIP_NONE);
    
    // Only show muzzle flash if this weapon just fired (timer > 0.05 means very recent)
    if (muzzleFlashTimer > 0.05f) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
        
        Vector2 muzzlePos = weaponPos + weaponDirection * 15;
        
        // Flash circle - smaller and less intrusive
        int flashRadius = 4;
        for (int x = -flashRadius; x <= flashRadius; x++) {
            for (int y = -flashRadius; y <= flashRadius; y++) {
                if (x*x + y*y <= flashRadius*flashRadius) {
                    SDL_RenderDrawPoint(renderer, (int)muzzlePos.x + x, (int)muzzlePos.y + y);
                }
            }
        }
    }
}

void Weapon::fire(const Vector2& weaponPos, const Vector2& direction, 
                 std::vector<std::unique_ptr<Bullet>>& bullets,
                 const Player& player) {
    
    // Melee weapons don't create bullets - they will be handled by Game's melee collision detection
    if (type == WeaponType::MELEE_STICK) {
        // Set attack timer for melee weapons (reuse muzzle flash timer)
        // This indicates the weapon is actively attacking
        muzzleFlashTimer = 0.3f; // Melee attack duration
        return;
    }
    
    Vector2 fireDirection = direction;
    
    // Add inaccuracy for SMG
    if (type == WeaponType::SMG) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> inaccuracy(-0.2f, 0.2f);
        
        float angle = atan2(fireDirection.y, fireDirection.x);
        angle += inaccuracy(gen);
        fireDirection = Vector2(cos(angle), sin(angle));
    }
    
    int finalDamage = calculateDamage(player);
    
    // Check for critical hit
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> critRoll(0.0f, 1.0f);
    
    if (critRoll(gen) < stats.critChance) {
        finalDamage = (int)(finalDamage * stats.critMultiplier);
    }
    
    // Create bullet with appropriate type from weapon position
    BulletType bulletType = (type == WeaponType::SMG) ? BulletType::SMG : BulletType::PISTOL;
    bullets.push_back(std::make_unique<Bullet>(weaponPos, fireDirection, finalDamage, stats.range, 400.0f, bulletType));
    
    // Special weapon effects
    if (type == WeaponType::PISTOL) {
        // Pistol pierces 1 enemy with -50% damage
        // This will be handled in the Bullet class collision detection
    }
}


int Weapon::calculateDamage(const Player& player) const {
    float totalDamage = stats.baseDamage;
    
    // Apply scaling from player stats
    if (stats.rangedDamageScaling > 0) {
        // For now, we'll use player's base damage as ranged damage
        // In a full implementation, Player would have separate ranged damage stat
        totalDamage += player.getStats().damage * stats.rangedDamageScaling;
    }
    
    return (int)totalDamage;
}

Vector2 Weapon::getWeaponTipPosition(const Vector2& weaponPos, const Vector2& direction) const {
    if (!isMeleeWeapon()) {
        return weaponPos;
    }
    
    float progress = getAttackProgress();
    // Create a smooth extension/retraction animation
    // Use a sine wave for smooth motion: extend quickly, pause, retract
    float animationPhase;
    if (progress < 0.6f) {
        // Extension phase (0.0 to 0.6 = 60% of animation)
        animationPhase = (progress / 0.6f) * (3.14159f / 2.0f); // 0 to π/2
        float extension = sin(animationPhase); // 0 to 1
        return weaponPos + direction * (stats.range * extension);
    } else {
        // Retraction phase (0.6 to 1.0 = 40% of animation)
        animationPhase = ((progress - 0.6f) / 0.4f) * (3.14159f / 2.0f); // 0 to π/2
        float retraction = cos(animationPhase); // 1 to 0
        return weaponPos + direction * (stats.range * retraction);
    }
}