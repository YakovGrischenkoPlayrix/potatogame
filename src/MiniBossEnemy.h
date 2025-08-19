#pragma once
#include "Enemy.h"
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

class Bullet;

class MiniBossEnemy : public Enemy {
public:
    MiniBossEnemy(Vector2 pos, SDL_Renderer* renderer, int variantIndex, bool isLeaderPart);
    ~MiniBossEnemy() override;

    void update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;
    void render(SDL_Renderer* renderer) override;
    int getMaxHealth() const override;
    bool isBossUnit() const override { return true; }
    bool isLeader() const override { return leader; }

private:
    void loadSprites(SDL_Renderer* renderer);
    void tryFireAtPlayer(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);

    // Shooting
    float fireCooldown;
    float timeSinceLastShot;
    float bulletSpeed;
    int bulletDamage;
    int variant; // 1..5

    // Graphics
    SDL_Texture* bossTexture;
    SDL_Texture* bossRushTexture;
    bool facingRight;
    bool leader;

    // Health
    int maxHealth;
};

// Factory
std::unique_ptr<Enemy> CreateMiniBossEnemy(const Vector2& pos, SDL_Renderer* renderer, int variantIndex, bool isLeader);


