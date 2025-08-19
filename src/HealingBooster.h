#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Vector2.h"

class HealingBooster {
public:
    HealingBooster(const Vector2& spawnPosition);
    ~HealingBooster();

    void initialize(SDL_Renderer* renderer);
    void update(float deltaTime);
    void render(SDL_Renderer* renderer);

    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    bool isAlive() const { return alive; }
    void collect();

private:
    Vector2 position;
    float radius;
    bool alive;
    float lifetime;
    float maxLifetime; // seconds before auto-despawn

    // Visuals
    SDL_Texture* texture;
    int textureWidth;
    int textureHeight;
};
