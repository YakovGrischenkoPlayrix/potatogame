#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <memory>
#include "Vector2.h"

enum class EnemyState {
    IDLE,
    HIT
};

class Bullet;

class Enemy {
public:
    Enemy(Vector2 pos, SDL_Renderer* renderer);
    virtual ~Enemy();
    
    virtual void update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);
    virtual void render(SDL_Renderer* renderer);
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    bool isAlive() const { return alive; }
    void destroy() { alive = false; }
    virtual void hit() { 
        state = EnemyState::HIT; 
        hitTimer = 0.0f; 
    }
    
    virtual void takeDamage(int damage) {
        health -= damage;
        if (health <= 0) {
            alive = false;
            health = 0;
        }
        hit();
    }
    
    int getDamage() const { return damage; }
    int getHealth() const { return health; }
    virtual int getMaxHealth() const; // Виртуальный метод для получения максимального здоровья
    // Является ли юнит боссом (для изменения логики спавна/индикаторов)
    virtual bool isBossUnit() const { return false; }
    // Является ли юнит лидером (центральной частью роя)
    virtual bool isLeader() const { return false; }
    
private:
    void loadSprites(SDL_Renderer* renderer);
    SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer);
    
protected:
    Vector2 position;
    Vector2 velocity;
    float radius;
    float speed;
    int health;
    int damage;
    bool alive;
    
    // Animation and sprites
    EnemyState state;
    float animationTimer;
    float hitTimer;
    int currentFrame;
    
    SDL_Texture* idleFrame1;
    SDL_Texture* idleFrame2;
    SDL_Texture* hitFrame;
    
    int spriteWidth;
    int spriteHeight;
};