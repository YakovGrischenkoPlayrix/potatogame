#pragma once
#include <SDL2/SDL.h>
#include "Vector2.h"

enum class BulletType {
    PISTOL,
    SMG,
    ENEMY_LOB,
    SHOTGUN,
    SNIPER,
    BOSS_BULLET,
    MINIBOSS_1,
    MINIBOSS_2,
    MINIBOSS_3,
    MINIBOSS_4,
    MINIBOSS_5,
    FRACTAL_CENTER,   // Красные пули центрального узла
    FRACTAL_LEVEL1,   // Зеленые пули уровня 1
    FRACTAL_LEVEL2,    // Синие пули уровня 2
    CENTIPEDE_BULLET

};

class Bullet {
public:
    Bullet(Vector2 pos, Vector2 dir, int damage = 10, float range = 200.0f, float speed = 400.0f, BulletType type = BulletType::PISTOL, bool enemyOwned = false, SDL_Color color = {255, 255, 0, 255});
    
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);
    
    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    int getDamage() const { return damage; }
    bool isAlive() const { return alive; }
    void destroy() { alive = false; }
    bool isEnemyOwned() const { return enemyOwned; }
    
private:
    Vector2 position;
    Vector2 startPosition;
    Vector2 direction;
    float speed;
    float radius;
    float maxRange;
    int damage;
    bool alive;
    BulletType bulletType;
    
    // For enemy lob projectiles
    Vector2 velocity; // used when bulletType == ENEMY_LOB
    float gravity;    // positive value pulls "down" on screen
    bool enemyOwned;
    
    // Custom bullet color
    SDL_Color bulletColor;
};