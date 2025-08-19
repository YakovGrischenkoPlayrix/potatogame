#include "CentipedeEnemy.h"
#include "Bullet.h"
#include <SDL2/SDL_image.h>
#include <cmath>
#include <iostream>
#include <random>

CentipedeEnemy::CentipedeEnemy(Vector2 pos, SDL_Renderer* renderer)
    : Enemy(pos, renderer), segmentTexture(nullptr), movementTimer(0.0f), 
      maxHealth(1000), segmentsLost(0) {
    
    // Характеристики многоножки
    speed = MOVEMENT_SPEED;
    currentSpeed = MOVEMENT_SPEED;
    radius = 30.0f;  // Радиус для каждого сегмента (увеличен для больших спрайтов)
    damage = 8;      // Урон при столкновении
    health = maxHealth;
    
    // Размеры сегмента (равны обычному мобу)
    spriteWidth = 64;   // 32 * 2 = 64
    spriteHeight = 64;  // 32 * 2 = 64
    
    loadSegmentSprite(renderer);
    initializeSegments();
}

CentipedeEnemy::~CentipedeEnemy() {
    if (segmentTexture) {
        SDL_DestroyTexture(segmentTexture);
        segmentTexture = nullptr;
    }
}

void CentipedeEnemy::loadSegmentSprite(SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load("assets/enemies/pebblin.png");
    if (!surface) {
        std::cout << "Failed to load pebblin.png for centipede segments: " << IMG_GetError() << std::endl;
        segmentTexture = nullptr;
    } else {
        segmentTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!segmentTexture) {
            std::cout << "Failed to create segment texture: " << SDL_GetError() << std::endl;
        }
    }
}

void CentipedeEnemy::initializeSegments() {
    segments.clear();
    segments.reserve(INITIAL_SEGMENTS);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Создаем сегменты вертикально вниз от начальной позиции
    for (int i = 0; i < INITIAL_SEGMENTS; ++i) {
        CentipedeSegment segment;
        segment.position = Vector2(position.x, position.y + i * SEGMENT_SPACING);
        segment.direction = Vector2(1, 0);  // Начальное направление - вправо
        segment.timeSinceLastShot = static_cast<float>(i) * 0.3f;  // Разные интервалы стрельбы
        segment.bulletColor = generateRandomBulletColor();
        segment.isHead = (i == 0);  // Первый сегмент - голова
        segments.push_back(segment);
    }
    
    // Обновляем позицию врага на позицию головы
    position = segments[0].position;
}

SDL_Color CentipedeEnemy::generateRandomBulletColor() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 5);
    
    // Набор ярких цветов для пуль
    SDL_Color colors[] = {
        {255, 100, 100, 255},  // Красный
        {100, 255, 100, 255},  // Зеленый
        {100, 100, 255, 255},  // Синий
        {255, 255, 100, 255},  // Желтый
        {255, 100, 255, 255},  // Пурпурный
        {100, 255, 255, 255}   // Голубой
    };
    
    return colors[dis(gen)];
}

Vector2 CentipedeEnemy::getNextDirection(Vector2 currentPos, Vector2 targetPos) {
    // Дискретное движение в 4 направлениях
    Vector2 diff = targetPos - currentPos;
    
    // Определяем, в какую сторону двигаться - по X или по Y
    if (std::abs(diff.x) > std::abs(diff.y)) {
        // Движение по горизонтали
        return Vector2(diff.x > 0 ? 1 : -1, 0);
    } else {
        // Движение по вертикали  
        return Vector2(0, diff.y > 0 ? 1 : -1);
    }
}

void CentipedeEnemy::updateMovement(float deltaTime, Vector2 playerPos) {
    movementTimer += deltaTime;
    
    // Обновляем скорость в зависимости от количества сегментов
    float speedMultiplier = 1.0f + (static_cast<float>(segmentsLost) * 0.1f);  // +10% за каждый потерянный сегмент
    currentSpeed = MOVEMENT_SPEED * speedMultiplier;
    
    if (movementTimer >= MOVEMENT_TIMER) {
        movementTimer = 0.0f;
        
        if (!segments.empty()) {
            // Сохраняем предыдущие позиции для змейкового движения
            std::vector<Vector2> prevPositions;
            for (const auto& segment : segments) {
                prevPositions.push_back(segment.position);
            }
            
            // Двигаем голову к игроку
            Vector2 headDirection = getNextDirection(segments[0].position, playerPos);
            segments[0].direction = headDirection;
            segments[0].position += headDirection * SEGMENT_SPACING;
            
            // Остальные сегменты следуют за предыдущими
            for (size_t i = 1; i < segments.size(); ++i) {
                segments[i].position = prevPositions[i - 1];
            }
            
            // Обновляем позицию врага на позицию головы
            position = segments[0].position;
        }
    }
}

void CentipedeEnemy::updateShooting(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    for (auto& segment : segments) {
        segment.timeSinceLastShot += deltaTime;
        
        // Каждый сегмент стреляет с разными интервалами
        float fireCooldown = BASE_FIRE_COOLDOWN + (static_cast<float>(rand() % 1000) / 1000.0f);  // 2.0-3.0 секунд
        
        if (segment.timeSinceLastShot >= fireCooldown) {
            segment.timeSinceLastShot = 0.0f;
            
            // Направление к игроку
            Vector2 toPlayer = playerPos - segment.position;
            if (toPlayer.length() > 1.0f) {
                Vector2 direction = toPlayer.normalized();
                
                // Создаем цветную пулю под цвет сегмента
                auto bullet = std::make_unique<Bullet>(
                    segment.position,
                    direction,
                    /*damage*/ 10,
                    /*range*/ 500.0f,
                    /*speed*/ 250.0f,  // Уменьшена скорость пуль
                    BulletType::CENTIPEDE_BULLET,  // Специальный тип для цветных пуль
                    /*enemyOwned*/ true,
                    segment.bulletColor  // Цвет пули соответствует цвету сегмента
                );
                
                bullets.push_back(std::move(bullet));
            }
        }
    }
}

void CentipedeEnemy::removeSegment() {
    if (!segments.empty()) {
        segments.pop_back();  // Удаляем последний сегмент
        segmentsLost++;
        
        if (segments.empty()) {
            alive = false;  // Многоножка умирает когда не остается сегментов
        }
    }
}

void CentipedeEnemy::update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    if (!alive) return;
    
    updateMovement(deltaTime, playerPos);
    updateShooting(deltaTime, playerPos, bullets);
    
    // Анимация
    animationTimer += deltaTime;
    if (state == EnemyState::HIT) {
        hitTimer += deltaTime;
        if (hitTimer > 0.3f) {
            state = EnemyState::IDLE;
            hitTimer = 0.0f;
        }
    }
    if (state == EnemyState::IDLE && animationTimer > 0.5f) {
        currentFrame = (currentFrame == 0) ? 1 : 0;
        animationTimer = 0.0f;
    }
}

void CentipedeEnemy::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Рендерим все сегменты
    for (size_t i = 0; i < segments.size(); ++i) {
        const auto& segment = segments[i];
        
        if (segmentTexture) {
            SDL_Rect dst{
                (int)(segment.position.x - spriteWidth/2),
                (int)(segment.position.y - spriteHeight/2),
                spriteWidth,
                spriteHeight
            };
            
            // Голова немного больше остальных сегментов
            if (segment.isHead) {
                dst.w = (int)(spriteWidth * 1.3f);
                dst.h = (int)(spriteHeight * 1.3f);
                dst.x = (int)(segment.position.x - dst.w/2);
                dst.y = (int)(segment.position.y - dst.h/2);
            }
            
            // Меняем цвет текстуры в зависимости от сегмента
            SDL_SetTextureColorMod(segmentTexture, 
                                   segment.bulletColor.r, 
                                   segment.bulletColor.g, 
                                   segment.bulletColor.b);
            
            SDL_RenderCopy(renderer, segmentTexture, nullptr, &dst);
            
            // Сбрасываем цветовую модуляцию
            SDL_SetTextureColorMod(segmentTexture, 255, 255, 255);
        } else {
            // Fallback - цветные круги
            SDL_SetRenderDrawColor(renderer, 
                                   segment.bulletColor.r, 
                                   segment.bulletColor.g, 
                                   segment.bulletColor.b, 255);
            
            int cx = (int)segment.position.x;
            int cy = (int)segment.position.y;
            int r = segment.isHead ? (int)(radius * 1.3f) : (int)radius;
            
            for (int x = -r; x <= r; ++x) {
                for (int y = -r; y <= r; ++y) {
                    if (x*x + y*y <= r*r) {
                        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
                    }
                }
            }
        }
    }
}

void CentipedeEnemy::takeDamage(int damage) {
    health -= damage;
    
    // Удаляем сегменты пропорционально урону
    float healthPercentage = static_cast<float>(health) / static_cast<float>(maxHealth);
    int targetSegments = static_cast<int>(INITIAL_SEGMENTS * healthPercentage);
    
    // Убеждаемся что у нас не меньше целевого количества сегментов
    while (static_cast<int>(segments.size()) > targetSegments && !segments.empty()) {
        removeSegment();
    }
    
    if (health <= 0) {
        alive = false;
        health = 0;
    }
    
    hit();
}

int CentipedeEnemy::getMaxHealth() const {
    return maxHealth;
}

std::unique_ptr<Enemy> CreateCentipedeEnemy(const Vector2& pos, SDL_Renderer* renderer) {
    return std::make_unique<CentipedeEnemy>(pos, renderer);
}
