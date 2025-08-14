#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <memory>
#include "Vector2.h"
#include "Bullet.h"

class Enemy;
class Player;

enum class WeaponType {
    PISTOL,
    SMG,
    MELEE_STICK,
    SHOTGUN,
    SNIPER,
    ORBITING_BRICK
};

enum class WeaponTier {
    TIER_1 = 1,
    TIER_2 = 2,
    TIER_3 = 3,
    TIER_4 = 4
};

struct WeaponStats {
    int baseDamage;
    float attackSpeed;        // Time between shots in seconds
    float range;
    float critChance;        // 0.0 to 1.0
    float critMultiplier;
    int knockback;
    float lifesteal;
    
    // Scaling percentages (0.0 to 1.0+)
    float rangedDamageScaling;
    float meleeDamageScaling;
    float elementalDamageScaling;
    
    WeaponStats() : baseDamage(10), attackSpeed(1.0f), range(400), critChance(0.05f), 
                   critMultiplier(2.0f), knockback(0), lifesteal(0.0f),
                   rangedDamageScaling(1.0f), meleeDamageScaling(0.0f), elementalDamageScaling(0.0f) {}
};

class Weapon {
public:
    Weapon(WeaponType type, WeaponTier tier = WeaponTier::TIER_1);
    virtual ~Weapon();
    
    // Initialize weapon with renderer for texture loading
    void initialize(SDL_Renderer* renderer);
    
    virtual void update(float deltaTime, const Vector2& weaponPos, 
                       const Vector2& aimDirection,
                       std::vector<std::unique_ptr<Bullet>>& bullets,
                       const Player& player);
    
    virtual void render(SDL_Renderer* renderer, const Vector2& weaponPos, const Vector2& weaponDirection);
    
    // Getters
    WeaponType getType() const { return type; }
    WeaponTier getTier() const { return tier; }
    const WeaponStats& getStats() const { return stats; }
    bool canFire() const { return timeSinceLastShot >= stats.attackSpeed; }
    
    // Calculate final damage with player stats
    int calculateDamage(const Player& player) const;
    
    // Melee weapon support
    bool isMeleeWeapon() const { return type == WeaponType::MELEE_STICK; }
    bool isOrbitingWeapon() const { return type == WeaponType::ORBITING_BRICK; }
    bool isAttacking() const { return muzzleFlashTimer > 0.0f; } // Reuse muzzle flash timer for melee attack duration
    float getAttackProgress() const { return muzzleFlashTimer > 0.0f ? (0.3f - muzzleFlashTimer) / 0.3f : 0.0f; } // 0.0 = start, 1.0 = fully extended
    Vector2 getWeaponTipPosition(const Vector2& weaponPos, const Vector2& direction) const;
    Vector2 getOrbitingPosition(const Vector2& playerPos) const;
    float getOrbitingRadius() const;
    
protected:
    virtual void fire(const Vector2& weaponPos, const Vector2& direction, 
                     std::vector<std::unique_ptr<Bullet>>& bullets,
                     const Player& player);
    
    // Initialize weapon stats based on type
    void initializePistolStats();
    void initializeSMGStats();
    void initializeMeleeStickStats();

    void initializeShotgunStats();
    void initializeSniperStats();

    void initializeOrbitingBrickStats();

    
    WeaponType type;
    WeaponTier tier;
    WeaponStats stats;
    float timeSinceLastShot;
    
    // Visual/audio feedback
    float muzzleFlashTimer;
    Vector2 lastShotDirection;
    
    // Sprite rendering
    SDL_Texture* weaponTexture;
    void loadWeaponTexture(SDL_Renderer* renderer);

    // Orbiting weapon state
    float orbitAngle = 0.0f;
    float orbitRadius = 70.0f;
    float orbitAngularSpeed = 2.5f; // radians/sec
    float orbitHitRadius = 16.0f;
};

inline void Weapon::initializeOrbitingBrickStats() {
    switch (tier) {
        case WeaponTier::TIER_1: stats.baseDamage = 8; orbitRadius = 70.0f; orbitAngularSpeed = 2.5f; orbitHitRadius = 16.0f; break;
        case WeaponTier::TIER_2: stats.baseDamage = 12; orbitRadius = 78.0f; orbitAngularSpeed = 2.8f; orbitHitRadius = 18.0f; break;
        case WeaponTier::TIER_3: stats.baseDamage = 18; orbitRadius = 86.0f; orbitAngularSpeed = 3.1f; orbitHitRadius = 20.0f; break;
        case WeaponTier::TIER_4: stats.baseDamage = 26; orbitRadius = 96.0f; orbitAngularSpeed = 3.4f; orbitHitRadius = 22.0f; break;
    }
    stats.attackSpeed = 0.0f; stats.range = orbitRadius; stats.critChance = 0.0f; stats.critMultiplier = 1.0f; stats.knockback = 20; stats.rangedDamageScaling = 0.0f; stats.meleeDamageScaling = 1.0f;
}

inline Vector2 Weapon::getOrbitingPosition(const Vector2& playerPos) const {
    if (type != WeaponType::ORBITING_BRICK) return playerPos;
    return playerPos + Vector2(cos(orbitAngle), sin(orbitAngle)) * orbitRadius;
}

inline float Weapon::getOrbitingRadius() const {
    if (type != WeaponType::ORBITING_BRICK) return 0.0f;
    return orbitHitRadius;
}