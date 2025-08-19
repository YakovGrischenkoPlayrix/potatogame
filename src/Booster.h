#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Vector2.h"

class Booster {
public:
    Booster(const Vector2& spawnPosition, float maxLifetime);
    virtual ~Booster();

    virtual void initialize(SDL_Renderer* renderer);
    virtual void update(float deltaTime);
    virtual void render(SDL_Renderer* renderer);
    void renderProgressBar(SDL_Renderer* renderer);

    Vector2 getPosition() const { return position; }
    float getRadius() const { return radius; }
    bool isAlive() const { return alive; }
    virtual void collect();
    
    // Getters for progress bar
    float getLifetime() const { return lifetime; }
    float getMaxLifetime() const { return maxLifetime; }
    float getRemainingTime() const { return maxLifetime - lifetime; }

protected:
    Vector2 position;
    float radius;
    bool alive;
    float lifetime;
    float maxLifetime; // seconds before auto-despawn

    // Visuals
    SDL_Texture* texture;
    int textureWidth;
    int textureHeight;
    
    // Progress bar properties
    static const int PROGRESS_BAR_WIDTH = 64;
    static const int PROGRESS_BAR_HEIGHT = 8;
    static const int PROGRESS_BAR_OFFSET_Y = 20; // Offset below the booster
};
