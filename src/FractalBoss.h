#pragma once
#include "Enemy.h"
#include "FractalNode.h"
#include <SDL2/SDL.h>
#include <memory>
#include <vector>

class Bullet;

class FractalBoss : public Enemy {
public:
    FractalBoss(Vector2 pos, SDL_Renderer* renderer);
    ~FractalBoss() override;
    
    void update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) override;
    void render(SDL_Renderer* renderer) override;
    void takeDamage(int damage) override;
    int getMaxHealth() const override;
    
private:
    FractalNode rootNode;           // Корневой узел фрактала
    float rotationSpeed;            // Скорость вращения (в радианах/сек)
    float currentRotation;          // Текущий угол поворота
    int fractalDepth;              // Глубина фрактала (3 уровня)
    float baseSize;                // Базовый размер структуры (400px)
    
    // Система урона по звеньям
    int totalNodes;                // Общее количество узлов (10 звеньев)
    int maxHealth;                 // Максимальное здоровье босса
    
    void generateFractal(FractalNode& node, int currentDepth, float size);
    void initializeFractal();
    void updateRotation(float deltaTime);
    FractalNode* findHitNode(Vector2 hitPosition);
    void updateHealthFromNodes();
    void collectLivingNodes(FractalNode* node, std::vector<FractalNode*>& livingNodes);
};

// Factory function
std::unique_ptr<Enemy> CreateFractalBoss(const Vector2& pos, SDL_Renderer* renderer); 