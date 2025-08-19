#include "Enemy.h"
#include <cstdlib>
#include <iostream>
#include <vector>
#include <memory>
#include "Bullet.h"

Enemy::Enemy(Vector2 pos, SDL_Renderer* renderer) 
    : position(pos), velocity(0, 0), radius(30), speed(80), 
      health(1), damage(10), alive(true), state(EnemyState::IDLE),
      animationTimer(0.0f), hitTimer(0.0f), currentFrame(0),
      idleFrame1(nullptr), idleFrame2(nullptr), hitFrame(nullptr),
      spriteWidth(64), spriteHeight(64) {
    loadSprites(renderer);
}

Enemy::~Enemy() {
    if (idleFrame1) SDL_DestroyTexture(idleFrame1);
    if (idleFrame2) SDL_DestroyTexture(idleFrame2);
    if (hitFrame) SDL_DestroyTexture(hitFrame);
}

void Enemy::loadSprites(SDL_Renderer* renderer) {
    idleFrame1 = loadTexture("monsters/landmonster/Transparent PNG/idle/frame-1.png", renderer);
    idleFrame2 = loadTexture("monsters/landmonster/Transparent PNG/idle/frame-2.png", renderer);
    hitFrame = loadTexture("monsters/landmonster/Transparent PNG/got hit/frame.png", renderer);
    
    if (!idleFrame1 || !idleFrame2 || !hitFrame) {
        std::cout << "Warning: Could not load some monster sprites!" << std::endl;
    }
}

SDL_Texture* Enemy::loadTexture(const char* path, SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load(path);
    if (!surface) {
        std::cout << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!texture) {
        std::cout << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
    }
    
    return texture;
}

void Enemy::update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& /*bullets*/) {
    Vector2 direction = (playerPos - position).normalized();
    velocity = direction * speed;
    position += velocity * deltaTime;
    
    // Update animation timing
    animationTimer += deltaTime;
    
    // Handle hit state
    if (state == EnemyState::HIT) {
        hitTimer += deltaTime;
        if (hitTimer > 0.2f) { // Hit animation lasts 0.2 seconds
            state = EnemyState::IDLE;
            hitTimer = 0.0f;
        }
    }
    
    // Handle idle animation (switch frames every 0.5 seconds)
    if (state == EnemyState::IDLE && animationTimer > 0.5f) {
        currentFrame = (currentFrame == 0) ? 1 : 0;
        animationTimer = 0.0f;
    }
}

void Enemy::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    SDL_Texture* currentTexture = nullptr;
    
    // Choose the right texture based on state
    if (state == EnemyState::HIT && hitFrame) {
        currentTexture = hitFrame;
    } else if (state == EnemyState::IDLE) {
        currentTexture = (currentFrame == 0 && idleFrame1) ? idleFrame1 : idleFrame2;
    }
    
    // If we have a texture, render it; otherwise fall back to circle
    if (currentTexture) {
        SDL_Rect destRect = {
            (int)(position.x - spriteWidth/2),
            (int)(position.y - spriteHeight/2),
            spriteWidth,
            spriteHeight
        };
        SDL_RenderCopy(renderer, currentTexture, nullptr, &destRect);
    } else {
        // Fallback to original circle rendering
        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255);
        
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
}

int Enemy::getMaxHealth() const {
    return 1; // Базовые враги имеют 1 HP
}