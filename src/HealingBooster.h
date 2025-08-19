#pragma once
#include "Booster.h"

class HealingBooster : public Booster {
public:
    HealingBooster(const Vector2& spawnPosition);
    ~HealingBooster();

    void initialize(SDL_Renderer* renderer) override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    void collect() override;
};
