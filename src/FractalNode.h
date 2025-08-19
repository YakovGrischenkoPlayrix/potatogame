#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include "Vector2.h"

class Bullet;

class FractalNode {
public:
    Vector2 localOffset;              // Смещение относительно родителя
    std::vector<FractalNode> children; // Дочерние узлы
    float shootTimer;                 // Таймер до следующего выстрела
    float shootInterval;              // Интервал между выстрелами
    int health;                       // Здоровье узла
    int maxHealth;                    // Максимальное здоровье узла
    bool alive;                       // Состояние узла
    float nodeRadius;                 // Радиус узла для коллизий
    int nodeLevel;                    // Уровень узла в фрактале (0=центр, 1,2,3=периферия)
    SDL_Color bulletColor;            // Цвет пуль этого узла
    
    FractalNode(Vector2 offset = Vector2(0, 0), int hp = 200, int level = 0);
    
    void update(float deltaTime, Vector2 parentWorldPos, float rotation, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);
    void tryShoot(Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets, Vector2 parentWorldPos, float rotation);
    Vector2 getWorldPosition(Vector2 parentPos, float rotation) const;
    void takeDamage(int damage);
    bool hasLivingChildren() const;
    int getTotalHealth() const;         // Суммарное здоровье узла и детей
    int countLivingNodes() const;       // Подсчет живых узлов в ветке
    void render(SDL_Renderer* renderer, Vector2 parentPos, float rotation) const;
    
    // Поиск ближайшего живого узла к точке
    FractalNode* findClosestNode(Vector2 hitPosition, Vector2 parentPos, float rotation, float& minDistance);
    
private:
    void updateChildren(float deltaTime, Vector2 worldPos, float rotation, Vector2 playerPos, std::vector<std::unique_ptr<Bullet>>& bullets);
    void renderChildren(SDL_Renderer* renderer, Vector2 worldPos, float rotation) const;
}; 