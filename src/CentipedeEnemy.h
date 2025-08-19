#pragma once
#include "Enemy.h"
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

class Bullet;

struct CentipedeSegment {
    Vector2 position;
    Vector2 direction;  // Направление движения для дискретного движения
    float timeSinceLastShot;
    SDL_Color bulletColor;  // Цвет пули для этого звена
    bool isHead;  // Является ли звено головой
};

class CentipedeEnemy : public Enemy {
public:
    CentipedeEnemy(Vector2 pos, SDL_Renderer* renderer);
    ~CentipedeEnemy() override;
    
    void update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;
    void render(SDL_Renderer* renderer) override;
    int getMaxHealth() const override;
    void takeDamage(int damage) override;
    
private:
    void loadSegmentSprite(SDL_Renderer* renderer);
    void initializeSegments();
    void updateMovement(float deltaTime, Vector2 playerPos);
    void updateShooting(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);
    void removeSegment();
    Vector2 getNextDirection(Vector2 currentPos, Vector2 targetPos);
    SDL_Color generateRandomBulletColor();
    
    // Константы
    static constexpr int INITIAL_SEGMENTS = 10;
    static constexpr float SEGMENT_SPACING = 50.0f;  // Увеличено для больших спрайтов (64px)
    static constexpr float MOVEMENT_SPEED = 120.0f;  // Увеличено в 2 раза
    static constexpr float MOVEMENT_TIMER = 0.6f;    // Быстрее движение
    static constexpr float BASE_FIRE_COOLDOWN = 2.0f;
    
    // Данные сегментов
    std::vector<CentipedeSegment> segments;
    SDL_Texture* segmentTexture;
    
    // Движение змейки
    float movementTimer;
    float currentSpeed;
    
    // Здоровье и урон
    int maxHealth;
    int segmentsLost;  // Количество потерянных сегментов
};

// Factory function
std::unique_ptr<Enemy> CreateCentipedeEnemy(const Vector2& pos, SDL_Renderer* renderer);
