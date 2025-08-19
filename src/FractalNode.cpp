#include "FractalNode.h"
#include "Bullet.h"
#include <cmath>
#include <algorithm>
#include <iostream>

FractalNode::FractalNode(Vector2 offset, int hp, int level) 
    : localOffset(offset), shootTimer(0.0f), health(hp), maxHealth(hp), 
      alive(true), nodeLevel(level) {
    
    // Настройка параметров в зависимости от уровня
    switch (level) {
        case 0: // Центральный узел
            nodeRadius = 25.0f;
            shootInterval = 0.8f;
            bulletColor = {50, 200, 50, 255}; // Ярко-зеленый
            break;
        case 1: // Первый уровень
            nodeRadius = 20.0f;
            shootInterval = 1.0f;
            bulletColor = {80, 160, 80, 255}; // Средне-зеленый
            break;
        case 2: // Второй уровень
            nodeRadius = 15.0f;
            shootInterval = 1.2f;
            bulletColor = {120, 180, 120, 255}; // Светло-зеленый
            break;
        case 3: // Третий уровень (на всякий случай)
            nodeRadius = 12.0f;
            shootInterval = 1.5f;
            bulletColor = {60, 120, 60, 255}; // Темно-зеленый
            break;
        default:
            nodeRadius = 10.0f;
            shootInterval = 2.0f;
            bulletColor = {255, 255, 255, 255}; // Белый
            break;
    }
    
    // Случайный разброс в интервале стрельбы для асинхронности
    shootTimer = static_cast<float>(rand()) / RAND_MAX * shootInterval;
}

void FractalNode::update(float deltaTime, Vector2 parentWorldPos, float rotation, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    if (!alive) return;
    
    shootTimer += deltaTime;
    
    // Попытка выстрела
    tryShoot(playerPos, bullets, parentWorldPos, rotation);
    
    // Обновление дочерних узлов
    updateChildren(deltaTime, getWorldPosition(parentWorldPos, rotation), rotation, playerPos, bullets);
}

void FractalNode::tryShoot(Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets, Vector2 parentWorldPos, float rotation) {
    if (!alive || shootTimer < shootInterval) return;
    
    Vector2 worldPos = getWorldPosition(parentWorldPos, rotation);
    Vector2 toPlayer = (playerPos - worldPos).normalized();
    
    // Создаем пулю с цветом в зависимости от уровня узла
    BulletType bulletType;
    switch (nodeLevel) {
        case 0: bulletType = BulletType::FRACTAL_CENTER; break;  // Ярко-зеленые пули центра
        case 1: bulletType = BulletType::FRACTAL_LEVEL1; break;  // Средне-зеленые пули уровня 1
        case 2: bulletType = BulletType::FRACTAL_LEVEL2; break;  // Светло-зеленые пули уровня 2
        default: bulletType = BulletType::BOSS_BULLET; break;    // Оранжевые пули по умолчанию
    }
    
    bullets.push_back(std::make_unique<Bullet>(
        worldPos, 
        toPlayer, 
        8 + nodeLevel * 2,  // Урон зависит от уровня
        500.0f, 
        300.0f + nodeLevel * 50.0f,  // Скорость зависит от уровня
        bulletType, 
        true
    ));
    
    shootTimer = 0.0f;
}

Vector2 FractalNode::getWorldPosition(Vector2 parentPos, float rotation) const {
    // Поворот локального смещения на угол rotation
    float cosR = std::cos(rotation);
    float sinR = std::sin(rotation);
    
    Vector2 rotatedOffset(
        localOffset.x * cosR - localOffset.y * sinR,
        localOffset.x * sinR + localOffset.y * cosR
    );
    
    return parentPos + rotatedOffset;
}

void FractalNode::takeDamage(int damage) {
    health -= damage;
    if (health <= 0) {
        health = 0;
        alive = false;
        std::cout << "Fractal node level " << nodeLevel << " destroyed!" << std::endl;
    }
}

bool FractalNode::hasLivingChildren() const {
    for (const auto& child : children) {
        if (child.alive || child.hasLivingChildren()) {
            return true;
        }
    }
    return false;
}

int FractalNode::getTotalHealth() const {
    int totalHealth = alive ? health : 0;
    for (const auto& child : children) {
        totalHealth += child.getTotalHealth();
    }
    return totalHealth;
}

int FractalNode::countLivingNodes() const {
    int count = alive ? 1 : 0;
    for (const auto& child : children) {
        count += child.countLivingNodes();
    }
    return count;
}

void FractalNode::render(SDL_Renderer* renderer, Vector2 parentPos, float rotation) const {
    if (!alive) return;
    
    Vector2 worldPos = getWorldPosition(parentPos, rotation);
    
    // Рендер соединительных линий к живым детям (темно-зеленый)
    SDL_SetRenderDrawColor(renderer, 30, 100, 30, 255);
    for (const auto& child : children) {
        if (child.alive) {
            Vector2 childWorldPos = child.getWorldPosition(worldPos, rotation);
            SDL_RenderDrawLine(renderer, 
                (int)worldPos.x, (int)worldPos.y,
                (int)childWorldPos.x, (int)childWorldPos.y);
        }
    }
    
    // Основной узел - квадрат с зеленым цветом в зависимости от уровня и здоровья
    float healthPercent = static_cast<float>(health) / maxHealth;
    
    // Зеленые оттенки в зависимости от уровня узла
    Uint8 red, green, blue;
    switch (nodeLevel) {
        case 0: // Центральный узел - яркий зеленый (как у Creeper)
            red = static_cast<Uint8>(20 + 40 * healthPercent);   // 20-60
            green = static_cast<Uint8>(120 + 100 * healthPercent); // 120-220  
            blue = static_cast<Uint8>(20 + 40 * healthPercent);   // 20-60
            break;
        case 1: // Уровень 1 - средний зеленый
            red = static_cast<Uint8>(40 + 30 * healthPercent);   // 40-70
            green = static_cast<Uint8>(100 + 80 * healthPercent); // 100-180
            blue = static_cast<Uint8>(40 + 30 * healthPercent);   // 40-70
            break;
        case 2: // Уровень 2 - светло-зеленый
            red = static_cast<Uint8>(60 + 40 * healthPercent);   // 60-100
            green = static_cast<Uint8>(140 + 80 * healthPercent); // 140-220
            blue = static_cast<Uint8>(60 + 40 * healthPercent);   // 60-100
            break;
        default: // Темно-зеленый по умолчанию
            red = static_cast<Uint8>(30 + 20 * healthPercent);
            green = static_cast<Uint8>(80 + 60 * healthPercent);
            blue = static_cast<Uint8>(30 + 20 * healthPercent);
            break;
    }
    
    SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
    SDL_Rect nodeRect = {
        (int)(worldPos.x - nodeRadius),
        (int)(worldPos.y - nodeRadius),
        (int)(nodeRadius * 2),
        (int)(nodeRadius * 2)
    };
    SDL_RenderFillRect(renderer, &nodeRect);
    
    // Контур узла (темно-зеленый)
    SDL_SetRenderDrawColor(renderer, 20, 80, 20, 255);
    SDL_RenderDrawRect(renderer, &nodeRect);
    
    // Индикатор уровня узла (маленькая точка в центре)
    SDL_SetRenderDrawColor(renderer, bulletColor.r, bulletColor.g, bulletColor.b, 255);
    SDL_Rect centerDot = {
        (int)(worldPos.x - 2),
        (int)(worldPos.y - 2),
        4, 4
    };
    SDL_RenderFillRect(renderer, &centerDot);
    
    // Рендер дочерних узлов
    renderChildren(renderer, worldPos, rotation);
}

FractalNode* FractalNode::findClosestNode(Vector2 hitPosition, Vector2 parentPos, float rotation, float& minDistance) {
    if (!alive) return nullptr;
    
    Vector2 worldPos = getWorldPosition(parentPos, rotation);
    float distance = worldPos.distance(hitPosition);
    
    FractalNode* closestNode = nullptr;
    
    // Проверяем попадание в этот узел
    if (distance <= nodeRadius && distance < minDistance) {
        minDistance = distance;
        closestNode = this;
    }
    
    // Проверяем дочерние узлы
    for (auto& child : children) {
        FractalNode* childClosest = child.findClosestNode(hitPosition, worldPos, rotation, minDistance);
        if (childClosest) {
            closestNode = childClosest;
        }
    }
    
    return closestNode;
}

void FractalNode::updateChildren(float deltaTime, Vector2 worldPos, float rotation, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets) {
    for (auto& child : children) {
        child.update(deltaTime, worldPos, rotation, playerPos, bullets);
    }
}

void FractalNode::renderChildren(SDL_Renderer* renderer, Vector2 worldPos, float rotation) const {
    for (const auto& child : children) {
        child.render(renderer, worldPos, rotation);
    }
} 