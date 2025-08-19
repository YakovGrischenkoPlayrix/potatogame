#include "Booster.h"
#include <iostream>

Booster::Booster(const Vector2& spawnPosition, float maxLifetime)
    : position(spawnPosition), radius(16.0f), alive(true), lifetime(0.0f), maxLifetime(maxLifetime),
      texture(nullptr), textureWidth(0), textureHeight(0) {}

Booster::~Booster() {}

void Booster::initialize(SDL_Renderer* renderer) {
    // Base class doesn't load specific texture - derived classes should override
}

void Booster::update(float deltaTime) {
    if (!alive) return;
    lifetime += deltaTime;
    if (lifetime >= maxLifetime) {
        alive = false;
    }
}

void Booster::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Base class doesn't render specific texture - derived classes should override
    // Fallback: draw colored circle
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow default
    int r = static_cast<int>(radius);
    int cx = static_cast<int>(position.x);
    int cy = static_cast<int>(position.y);
    for (int x = -r; x <= r; ++x) {
        for (int y = -r; y <= r; ++y) {
            if (x * x + y * y <= r * r) {
                SDL_RenderDrawPoint(renderer, cx + x, cy + y);
            }
        }
    }
    
    // Render progress bar
    renderProgressBar(renderer);
}

void Booster::renderProgressBar(SDL_Renderer* renderer) {
    if (!alive) return;
    
    // Calculate progress bar position (centered below the booster)
    int barX = static_cast<int>(position.x - PROGRESS_BAR_WIDTH / 2);
    int barY = static_cast<int>(position.y + radius + PROGRESS_BAR_OFFSET_Y);
    
    // Draw background (dark gray)
    SDL_SetRenderDrawColor(renderer, 64, 64, 64, 255);
    SDL_Rect bgRect = {barX, barY, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &bgRect);
    
    // Calculate remaining time percentage
    float remainingTime = getRemainingTime();
    float progress = remainingTime / maxLifetime;
    int progressWidth = static_cast<int>(PROGRESS_BAR_WIDTH * progress);
    
    // Draw progress bar (green to yellow to red based on remaining time)
    if (progress > 0.5f) {
        // Green for more than 50% remaining
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    } else if (progress > 0.25f) {
        // Yellow for 25-50% remaining
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    } else {
        // Red for less than 25% remaining
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }
    
    if (progressWidth > 0) {
        SDL_Rect progressRect = {barX, barY, progressWidth, PROGRESS_BAR_HEIGHT};
        SDL_RenderFillRect(renderer, &progressRect);
    }
    
    // Draw border
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &bgRect);
}

void Booster::collect() {
    alive = false;
}
