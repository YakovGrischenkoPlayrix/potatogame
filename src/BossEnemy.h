#pragma once
#include "Enemy.h"
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

class Bullet;

class BossEnemy : public Enemy {
public:
    BossEnemy(Vector2 pos, SDL_Renderer* renderer);
    ~BossEnemy() override;
    
    void update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;
    void render(SDL_Renderer* renderer) override;
    int getMaxHealth() const override;
    bool isBossUnit() const override { return true; }
    bool isLeader() const override { return false; }

    
private:
    void loadSprites(SDL_Renderer* renderer);
    void tryFireAtPlayer(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);
    
    // Стрельба
    float fireCooldown;
    float timeSinceLastShot;
    
    // Двухфазное движение
    float movementTimer;
    bool isSprintPhase;
    float normalSpeed;
    float sprintSpeed;
    
    // Графика
    SDL_Texture* bossTexture;
    SDL_Texture* bossRushTexture; // Спрайт для режима спринта
    bool facingRight; // Направление взгляда босса
    
    // Здоровье
    int maxHealth; // Максимальное здоровье босса
};

// Factory function
std::unique_ptr<Enemy> CreateBossEnemy(const Vector2& pos, SDL_Renderer* renderer);
