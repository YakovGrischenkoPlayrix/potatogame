#pragma once
#include <SDL2/SDL.h>
#include <vector>
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
    SNIPER
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
    bool isAttacking() const { return muzzleFlashTimer > 0.0f; } // Reuse muzzle flash timer for melee attack duration
    float getAttackProgress() const { return muzzleFlashTimer > 0.0f ? (0.3f - muzzleFlashTimer) / 0.3f : 0.0f; } // 0.0 = start, 1.0 = fully extended
    Vector2 getWeaponTipPosition(const Vector2& weaponPos, const Vector2& direction) const;
    
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
};