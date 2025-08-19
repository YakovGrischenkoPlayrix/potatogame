#include "MiniBossEnemy.h"
#include "Bullet.h"
#include <SDL2/SDL_image.h>
#include <cmath>
#include <iostream>

MiniBossEnemy::MiniBossEnemy(Vector2 pos, SDL_Renderer* renderer, int variantIndex, bool isLeaderPart)
    : Enemy(pos, renderer), fireCooldown(2.0f), timeSinceLastShot(0.0f), bulletSpeed(500.0f), bulletDamage(8),
      variant(variantIndex), bossTexture(nullptr), bossRushTexture(nullptr), facingRight(false), leader(isLeaderPart), maxHealth(900) {
    // Stats
    if (leader) {
        maxHealth = 1500;
        health = maxHealth;
        radius = 38.0f;
        spriteWidth = 120;
        spriteHeight = 120;
        speed = 95.0f;
    } else {
        maxHealth = 900;
        health = maxHealth;
        radius = 30.0f;
        spriteWidth = 96;
        spriteHeight = 96;
        speed = 110.0f;
    }

    // Variant-based shooting tuning
    switch (variant) {
        case 1: fireCooldown = 2.1f; bulletSpeed = 420.0f; bulletDamage = 7; break;
        case 2: fireCooldown = 2.0f; bulletSpeed = 480.0f; bulletDamage = 7; break;
        case 3: fireCooldown = 1.9f; bulletSpeed = 540.0f; bulletDamage = 8; break;
        case 4: fireCooldown = 1.8f; bulletSpeed = 600.0f; bulletDamage = 8; break;
        case 5: fireCooldown = 1.7f; bulletSpeed = 660.0f; bulletDamage = 9; break;
        default: break;
    }

    loadSprites(renderer);
}

MiniBossEnemy::~MiniBossEnemy() {
    if (bossRushTexture) {
        SDL_DestroyTexture(bossRushTexture);
        bossRushTexture = nullptr;
    }
}

void MiniBossEnemy::loadSprites(SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load("assets/enemies/bossT.png");
    if (!surface) {
        std::cout << "Failed to load bossT.png: " << IMG_GetError() << std::endl;
        bossTexture = nullptr;
    } else {
        bossTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!bossTexture) {
            std::cout << "Failed to create miniboss texture: " << SDL_GetError() << std::endl;
        }
    }

    surface = IMG_Load("assets/enemies/bossT_rush.png");
    if (!surface) {
        std::cout << "Failed to load bossT_rush.png: " << IMG_GetError() << std::endl;
        bossRushTexture = nullptr;
    } else {
        bossRushTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!bossRushTexture) {
            std::cout << "Failed to create miniboss rush texture: " << SDL_GetError() << std::endl;
        }
    }
}

void MiniBossEnemy::tryFireAtPlayer(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    timeSinceLastShot += deltaTime;
    if (timeSinceLastShot < fireCooldown) return;
    timeSinceLastShot = 0.0f;

    Vector2 toPlayer = playerPos - position;
    if (toPlayer.length() < 1.0f) return;
    Vector2 direction = toPlayer.normalized();

    BulletType btype = BulletType::MINIBOSS_1;
    switch (variant) {
        case 1: btype = BulletType::MINIBOSS_1; break;
        case 2: btype = BulletType::MINIBOSS_2; break;
        case 3: btype = BulletType::MINIBOSS_3; break;
        case 4: btype = BulletType::MINIBOSS_4; break;
        case 5: btype = BulletType::MINIBOSS_5; break;
        default: break;
    }

    bullets.push_back(std::make_unique<Bullet>(
        position,
        direction,
        /*damage*/ bulletDamage,
        /*range*/ 650.0f,
        /*speed*/ bulletSpeed,
        btype,
        /*enemyOwned*/ true
    ));
}

void MiniBossEnemy::update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // Movement towards player
    Vector2 direction = (playerPos - position).normalized();
    velocity = direction * speed;
    position += velocity * deltaTime;

    // Facing
    facingRight = (playerPos.x > position.x);

    // Shooting
    tryFireAtPlayer(deltaTime, playerPos, bullets);

    // Animation
    animationTimer += deltaTime;
    if (state == EnemyState::HIT) {
        hitTimer += deltaTime;
        if (hitTimer > 0.2f) {
            state = EnemyState::IDLE;
            hitTimer = 0.0f;
        }
    }
    if (state == EnemyState::IDLE && animationTimer > 0.7f) {
        currentFrame = (currentFrame == 0) ? 1 : 0;
        animationTimer = 0.0f;
    }
}

void MiniBossEnemy::render(SDL_Renderer* renderer) {
    if (!alive) return;

    SDL_Texture* currentTexture = nullptr;
    // Simple state without sprint phases for now
    if (bossTexture) {
        currentTexture = bossTexture;
    }

    if (currentTexture) {
        SDL_Rect dst{
            (int)(position.x - spriteWidth/2),
            (int)(position.y - spriteHeight/2),
            spriteWidth,
            spriteHeight
        };
        SDL_RendererFlip flip = facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, currentTexture, nullptr, &dst, 0.0, nullptr, flip);
    } else {
        // Fallback circle
        SDL_SetRenderDrawColor(renderer, 120, 20, 20, 255);
        int cx = (int)position.x;
        int cy = (int)position.y;
        int r = (int)radius;
        for (int x = -r; x <= r; ++x) {
            for (int y = -r; y <= r; ++y) {
                if (x*x + y*y <= r*r) {
                    SDL_RenderDrawPoint(renderer, cx + x, cy + y);
                }
            }
        }
    }

    // Small HP bar above head
    int barWidth = leader ? 80 : 60;
    int barHeight = leader ? 8 : 6;
    int barX = (int)position.x - barWidth/2;
    int barY = (int)(position.y - spriteHeight/2) - 12;
    if (barY < 0) barY = 0;

    // Background
    SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
    SDL_Rect bg = {barX, barY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &bg);
    // Foreground proportional to health
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    int fgWidth = (health * barWidth) / maxHealth;
    if (fgWidth < 0) fgWidth = 0;
    SDL_Rect fg = {barX, barY, fgWidth, barHeight};
    SDL_RenderFillRect(renderer, &fg);
    // Border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bg);
}

int MiniBossEnemy::getMaxHealth() const {
    return maxHealth;
}

std::unique_ptr<Enemy> CreateMiniBossEnemy(const Vector2& pos, SDL_Renderer* renderer, int variantIndex, bool isLeader) {
    return std::make_unique<MiniBossEnemy>(pos, renderer, variantIndex, isLeader);
}


