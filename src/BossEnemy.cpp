#include "BossEnemy.h"
#include "Bullet.h"
#include <SDL2/SDL_image.h>
#include <cmath>
#include <iostream>

BossEnemy::BossEnemy(Vector2 pos, SDL_Renderer* renderer)
    : Enemy(pos, renderer), fireCooldown(1.5f), timeSinceLastShot(0.0f),
      movementTimer(0.0f), isSprintPhase(false), bossTexture(nullptr), bossRushTexture(nullptr), facingRight(false), maxHealth(500) {
    
    // Босс характеристики - супер танковый
    speed = 68.0f;          // На 15% медленнее базового (80 * 0.85 = 68)
    normalSpeed = 68.0f;    // Нормальная скорость
    sprintSpeed = 160.0f;   // Спринт - в 2 раза быстрее базового
    radius = 60.0f;         // В 2 раза больше радиус (30 -> 60)
    damage = 15;            // Больше урона
    health = 1000;          // В 200 раз больше здоровья чем у базового (160 * 50 = 500)
    
    // Размеры спрайта - в 3.1 раза больше (еще +20%)
    spriteWidth = 200;      // Базовый 64 * 2 * 1.3 * 1.2 ≈ 200
    spriteHeight = 200;     // Базовый 64 * 2 * 1.3 * 1.2 ≈ 200
    
    loadSprites(renderer);
}

BossEnemy::~BossEnemy() {
    if (bossRushTexture) {
        SDL_DestroyTexture(bossRushTexture);
        bossRushTexture = nullptr;
    }
}

void BossEnemy::loadSprites(SDL_Renderer* renderer) {
    // Загружаем обычный спрайт босса
    SDL_Surface* surface = IMG_Load("assets/enemies/bossT.png");
    if (!surface) {
        std::cout << "Failed to load bossT.png: " << IMG_GetError() << std::endl;
        bossTexture = nullptr;
    } else {
        bossTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!bossTexture) {
            std::cout << "Failed to create boss texture: " << SDL_GetError() << std::endl;
        }
    }
    
    // Загружаем спрайт босса для режима спринта
    surface = IMG_Load("assets/enemies/bossT_rush.png");
    if (!surface) {
        std::cout << "Failed to load bossT_rush.png: " << IMG_GetError() << std::endl;
        bossRushTexture = nullptr;
    } else {
        bossRushTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!bossRushTexture) {
            std::cout << "Failed to create boss rush texture: " << SDL_GetError() << std::endl;
        }
    }
}

void BossEnemy::tryFireAtPlayer(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    timeSinceLastShot += deltaTime;
    if (timeSinceLastShot < fireCooldown) return;
    timeSinceLastShot = 0.0f;

    // Направление к игроку
    Vector2 toPlayer = playerPos - position;
    if (toPlayer.length() < 1.0f) return;

    Vector2 direction = toPlayer.normalized();
    
    // Босс стреляет быстрыми оранжевыми пулями
    // Скорость пули в 2 раза быстрее других врагов (обычно ~300, делаем 600)
    bullets.push_back(std::make_unique<Bullet>(
        position, 
        direction, 
        /*damage*/ 12, 
        /*range*/ 700.0f, 
        /*speed*/ 600.0f,  // В 2 раза быстрее
        BulletType::BOSS_BULLET,  // Специальный тип для оранжевого цвета
        /*enemyOwned*/ true
    ));
}

void BossEnemy::update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    // Двухфазное движение: 4 сек медленно + 2 сек спринт
    movementTimer += deltaTime;
    
    if (!isSprintPhase && movementTimer >= 4.0f) {
        // Переключаемся на спринт
        isSprintPhase = true;
        speed = sprintSpeed;
        movementTimer = 0.0f;
    } else if (isSprintPhase && movementTimer >= 2.0f) {
        // Переключаемся на медленную фазу
        isSprintPhase = false;
        speed = normalSpeed;
        movementTimer = 0.0f;
    }
    
    // Движение к игроку
    Vector2 direction = (playerPos - position).normalized();
    velocity = direction * speed;
    position += velocity * deltaTime;
    
    // Обновляем направление взгляда босса
    facingRight = (playerPos.x > position.x);

    // Стрельба
    tryFireAtPlayer(deltaTime, playerPos, bullets);

    // Анимация (медленнее чем у обычных врагов)
    animationTimer += deltaTime;
    if (state == EnemyState::HIT) {
        hitTimer += deltaTime;
        if (hitTimer > 0.3f) { // Дольше показываем урон для босса
            state = EnemyState::IDLE;
            hitTimer = 0.0f;
        }
    }
    if (state == EnemyState::IDLE && animationTimer > 1.0f) { // Медленная анимация
        currentFrame = (currentFrame == 0) ? 1 : 0;
        animationTimer = 0.0f;
    }
}

void BossEnemy::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Выбираем спрайт в зависимости от фазы движения
    SDL_Texture* currentTexture = nullptr;
    if (isSprintPhase && bossRushTexture) {
        currentTexture = bossRushTexture; // Спрайт для режима спринта
    } else if (bossTexture) {
        currentTexture = bossTexture; // Обычный спрайт
    }
    
    if (currentTexture) {
        // Рендерим босса с увеличенным размером + отзеркаливание
        // Исходный спрайт 360x360, отображаем как 200x200
        SDL_Rect dst{
            (int)(position.x - spriteWidth/2),
            (int)(position.y - spriteHeight/2),
            spriteWidth,
            spriteHeight
        };
        
        // Отзеркаливание если игрок справа (босс смотрит вправо)
        SDL_RendererFlip flip = facingRight ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
        SDL_RenderCopyEx(renderer, currentTexture, nullptr, &dst, 0.0, nullptr, flip);
    } else {
        // Fallback - большой темно-красный круг (увеличен на 56%)
        SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255);
        int cx = (int)position.x;
        int cy = (int)position.y;
        int r = (int)(radius * 1.56f); // Соответствует увеличенному размеру (1.3 * 1.2)
        
        for (int x = -r; x <= r; ++x) {
            for (int y = -r; y <= r; ++y) {
                if (x*x + y*y <= r*r) {
                    SDL_RenderDrawPoint(renderer, cx + x, cy + y);
                }
            }
        }
    }
}

int BossEnemy::getMaxHealth() const {
    return maxHealth;
}

std::unique_ptr<Enemy> CreateBossEnemy(const Vector2& pos, SDL_Renderer* renderer) {
    return std::make_unique<BossEnemy>(pos, renderer);
}
