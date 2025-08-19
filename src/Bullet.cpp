#include "Bullet.h"

Bullet::Bullet(Vector2 pos, Vector2 dir, int dmg, float range, float speed, BulletType type, bool enemyOwnedFlag) 
    : position(pos), startPosition(pos), direction(dir.normalized()), 
      speed(speed), radius(7), maxRange(range), damage(dmg), alive(true), bulletType(type),
      velocity(dir.normalized() * speed), gravity(550.0f), enemyOwned(enemyOwnedFlag) {
}

void Bullet::update(float deltaTime) {
    if (bulletType == BulletType::ENEMY_LOB) {
        // simple parabolic motion: integrate velocity with gravity downward
        velocity.y += gravity * deltaTime;
        position += velocity * deltaTime;
    } else {
        position += direction * speed * deltaTime;
    }
    
    // Check if bullet is out of bounds
    if (position.x < 0 || position.x > 1920 || position.y < 0 || position.y > 1080) {
        alive = false;
    }
    
    // Check if bullet has exceeded its range for non-lob; for lob, use bigger cap
    float distanceTraveled = startPosition.distance(position);
    float allowedRange = (bulletType == BulletType::ENEMY_LOB) ? maxRange * 1.5f : maxRange;
    if (distanceTraveled > allowedRange) {
        alive = false;
    }
}

void Bullet::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Color based on bullet type and ownership
    if (bulletType == BulletType::BOSS_BULLET) {
        SDL_SetRenderDrawColor(renderer, 255, 140, 0, 255); // Orange for boss bullets
    } else if (bulletType == BulletType::FRACTAL_CENTER) {
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // Red for fractal center
    } else if (bulletType == BulletType::FRACTAL_LEVEL1) {
        SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255); // Green for fractal level 1
    } else if (bulletType == BulletType::FRACTAL_LEVEL2) {
        SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255); // Blue for fractal level 2
    } else if (enemyOwned) {
        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); // Red for enemy bullets
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow for player bullets
    }
    
    int centerX = (int)position.x;
    int centerY = (int)position.y;
    int r = (int)radius;
    
    for (int x = -r; x <= r; x++) {
        for (int y = -r; y <= r; y++) {
            if (x*x + y*y <= r*r) {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
}