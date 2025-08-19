#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "Vector2.h"
#include "Weapon.h"

struct PlayerStats {
    // Brotato-like stats
    int maxHealth;
    float moveSpeed;
    float pickupRange;
    float attackSpeed;
    int damage;
    float range;
    int armor;
    float healthRegen;
    float dodgeChance;
    int luck;
    int materials; // Add materials currency like Brotato
    
    PlayerStats() : maxHealth(100), moveSpeed(200), pickupRange(50), 
                   attackSpeed(1.0f), damage(10), range(200), 
                   armor(0), healthRegen(0), dodgeChance(0), luck(0), materials(50) {}
};

class Player {
public:
    Player(float x, float y);
    
    // Initialize player with renderer for texture loading
    void initialize(SDL_Renderer* renderer);
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    void handleInput(const Uint8* keyState);
    void updateShootDirection(const Vector2& mousePosition);
    void gainExperience(int exp);
    void levelUp();
    
    // Weapon management
    void addWeapon(std::unique_ptr<Weapon> weapon);
    void addWeapon(std::unique_ptr<Weapon> weapon, SDL_Renderer* renderer);
    void updateWeapons(float deltaTime, std::vector<std::unique_ptr<Bullet>>& bullets);
    void renderWeapons(SDL_Renderer* renderer);
    void initializeWeapons(SDL_Renderer* renderer);
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    int getHealth() const { return health; }
    void takeDamage(int damage);
    
    bool canShoot() const;
    void shoot();
    Vector2 getShootDirection() const { return shootDirection; }
    
    // Stats and progression
    PlayerStats& getStats() { return stats; }
    const PlayerStats& getStats() const { return stats; }
    int getExperience() const { return experience; }
    int getLevel() const { return level; }
    int getExperienceToNextLevel() const;
    void gainMaterials(int amount) { stats.materials += amount; }
    
    // Weapon getters
    int getWeaponCount() const { return weapons.size(); }
    const std::vector<std::unique_ptr<Weapon>>& getWeapons() const { return weapons; }
    const Weapon* getWeapon(int index) const { 
        if (index >= 0 && index < static_cast<int>(weapons.size())) {
            return weapons[index].get();
        }
        return nullptr;
    }

    // Temporary effects
    void applyFireRateBoost(float multiplier, float durationSeconds);
    float getFireRateMultiplier() const { return fireRateMultiplier; }
    
private:
    Vector2 position;
    Vector2 velocity;
    Vector2 shootDirection;
    float radius;
    int health;
    float shootCooldown;
    float timeSinceLastShot;
    
    // Progression system
    PlayerStats stats;
    int experience;
    int level;
    float healthRegenTimer;
    float healthRegenAccumulator; // Accumulates fractional health regeneration
    
    // Weapon inventory (max 6 weapons like Brotato)
    std::vector<std::unique_ptr<Weapon>> weapons;
    static const int MAX_WEAPONS = 6;
    
    // Player sprite
    SDL_Texture* playerTexture;

    // Temporary buff state
    float fireRateMultiplier = 1.0f;
    float fireRateBoostRemaining = 0.0f;
};