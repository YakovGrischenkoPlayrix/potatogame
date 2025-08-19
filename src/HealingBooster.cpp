#include "HealingBooster.h"
#include <iostream>

HealingBooster::HealingBooster(const Vector2& spawnPosition)
    : Booster(spawnPosition, 5.0f) {}

HealingBooster::~HealingBooster() {}

void HealingBooster::initialize(SDL_Renderer* renderer) {
    SDL_Surface* surface = IMG_Load("assets/ui/healing_booster.png");
    if (!surface) {
        std::cout << "Failed to load healing_booster.png: " << IMG_GetError() << std::endl;
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    textureWidth = surface->w;
    textureHeight = surface->h;
    SDL_FreeSurface(surface);
}

void HealingBooster::update(float deltaTime) {
    if (!alive) return;
    lifetime += deltaTime;
    if (lifetime >= maxLifetime) {
        alive = false;
    }
}

void HealingBooster::render(SDL_Renderer* renderer) {
    if (!alive) return;
    
    if (texture) {
        // Draw centered at position
        int scaledW = textureWidth;
        int scaledH = textureHeight;
        SDL_Rect dst{ static_cast<int>(position.x - scaledW / 2), static_cast<int>(position.y - scaledH / 2), scaledW, scaledH };
        SDL_RenderCopy(renderer, texture, nullptr, &dst);
    } else {
        // Fallback: draw green cross-like circle
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
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
    }
    
    // Render progress bar from base class
    renderProgressBar(renderer);
}

void HealingBooster::collect() {
    alive = false;
}
