#include "FractalBoss.h"
#include "Bullet.h"
#include <cmath>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

FractalBoss::FractalBoss(Vector2 pos, SDL_Renderer* renderer)
    : Enemy(pos, renderer), rotationSpeed(0.785f), currentRotation(0.0f),
      fractalDepth(2), baseSize(300.0f), totalNodes(0), maxHealth(0) {
    
    // Характеристики фрактального босса
    speed = 50.0f;          // Медленный босс
    radius = 200.0f;        // Большой радиус коллизии
    damage = 20;            // Урон при столкновении
    
    // Генерируем фрактальную структуру
    initializeFractal();
    
    // Подсчитываем общее здоровье и количество узлов
    totalNodes = rootNode.countLivingNodes();
    maxHealth = rootNode.getTotalHealth();
    health = maxHealth;
    
    std::cout << "FractalBoss created with " << totalNodes << " nodes and " << maxHealth << " total health" << std::endl;
}

FractalBoss::~FractalBoss() {
    // Автоматическая очистка FractalNode деструктором
}

void FractalBoss::initializeFractal() {
    // Инициализируем корневой узел с ограниченным здоровьем
    rootNode = FractalNode(Vector2(0, 0), 100, 0); // Центральный узел: 100 HP
    
    // Генерируем фрактальную структуру
    generateFractal(rootNode, fractalDepth, baseSize);
}

void FractalBoss::generateFractal(FractalNode& node, int currentDepth, float size) {
    if (currentDepth <= 0) return;
    
    // Создаем 4 дочерних узла в углах квадрата
    float offset = size / 3.0f; // Расстояние от центра до угла
    node.children.resize(4);
    
    // Позиции углов квадрата с ограниченным здоровьем  
    // currentDepth 2 = level 1: 80 HP, currentDepth 1 = level 2: 60 HP
    int nodeHealth = 40 + currentDepth * 20; // 60, 80 HP в зависимости от глубины
    node.children[0] = FractalNode(Vector2(-offset, -offset), nodeHealth, 4 - currentDepth); // Верх-лево
    node.children[1] = FractalNode(Vector2(offset, -offset), nodeHealth, 4 - currentDepth);  // Верх-право
    node.children[2] = FractalNode(Vector2(-offset, offset), nodeHealth, 4 - currentDepth);  // Низ-лево
    node.children[3] = FractalNode(Vector2(offset, offset), nodeHealth, 4 - currentDepth);   // Низ-право
    
    // Рекурсивно создаем подструктуры
    for (auto& child : node.children) {
        generateFractal(child, currentDepth - 1, size * 0.5f);
    }
}

void FractalBoss::update(float deltaTime, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    if (!alive) return;
    
    // Медленное движение к игроку
    Vector2 direction = (playerPos - position).normalized();
    velocity = direction * speed;
    position += velocity * deltaTime;
    
    // Постоянное вращение структуры
    updateRotation(deltaTime);
    
    // Обновление всех узлов фрактала
    rootNode.update(deltaTime, position, currentRotation, playerPos, bullets);
    
    // Обновление здоровья босса на основе живых узлов
    updateHealthFromNodes();
    
    // Проверка на смерть босса - только когда ВСЕ узлы уничтожены
    int livingNodes = rootNode.countLivingNodes();
    if (livingNodes == 0) {
        alive = false;
        health = 0;
        std::cout << "FractalBoss defeated! All nodes destroyed!" << std::endl;
    } else if (health <= 0 && livingNodes > 0) {
        // Если здоровье 0, но узлы еще живы - восстанавливаем здоровье
        health = rootNode.getTotalHealth();
        std::cout << "Boss health restored: " << health << " (living nodes: " << livingNodes << ")" << std::endl;
    }
}

void FractalBoss::updateRotation(float deltaTime) {
    currentRotation += rotationSpeed * deltaTime;
    
    // Нормализация угла
    while (currentRotation > 2.0f * M_PI) {
        currentRotation -= 2.0f * M_PI;
    }
}

void FractalBoss::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Рендер всей фрактальной структуры
    rootNode.render(renderer, position, currentRotation);
    
    // Центральная точка босса (для отладки)
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect centerPoint = {
        (int)(position.x - 3),
        (int)(position.y - 3),
        6, 6
    };
    SDL_RenderFillRect(renderer, &centerPoint);
}

void FractalBoss::takeDamage(int damage) {
    // Найти случайный живой узел для нанесения урона
    std::vector<FractalNode*> livingNodes;
    collectLivingNodes(&rootNode, livingNodes);
    
    if (!livingNodes.empty()) {
        // Выбираем случайный живой узел
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, livingNodes.size() - 1);
        
        FractalNode* targetNode = livingNodes[dis(gen)];
        targetNode->takeDamage(damage);
        
        std::cout << "Damaged fractal node level " << targetNode->nodeLevel 
                  << ", remaining health: " << targetNode->health << std::endl;
    }
    
    // Обновляем здоровье босса
    updateHealthFromNodes();
    
    // Вызываем анимацию попадания
    hit();
}

FractalNode* FractalBoss::findHitNode(Vector2 hitPosition) {
    float minDistance = std::numeric_limits<float>::max();
    return rootNode.findClosestNode(hitPosition, position, currentRotation, minDistance);
}

void FractalBoss::updateHealthFromNodes() {
    int currentTotalHealth = rootNode.getTotalHealth();
    int currentLivingNodes = rootNode.countLivingNodes();
    
    // Обновляем здоровье босса пропорционально количеству живых узлов
    health = currentTotalHealth;
    
    // Информация для отладки
    static int lastNodeCount = -1;
    if (currentLivingNodes != lastNodeCount) {
        std::cout << "Living nodes: " << currentLivingNodes << "/" << totalNodes 
                  << ", Health: " << health << "/" << maxHealth << std::endl;
        lastNodeCount = currentLivingNodes;
    }
}

int FractalBoss::getMaxHealth() const {
    return maxHealth;
}

void FractalBoss::collectLivingNodes(FractalNode* node, std::vector<FractalNode*>& livingNodes) {
    if (node->alive) {
        livingNodes.push_back(node);
    }
    
    // Рекурсивно проверяем дочерние узлы
    for (auto& child : node->children) {
        collectLivingNodes(&child, livingNodes);
    }
}

// Factory function
std::unique_ptr<Enemy> CreateFractalBoss(const Vector2& pos, SDL_Renderer* renderer) {
    return std::make_unique<FractalBoss>(pos, renderer);
} 