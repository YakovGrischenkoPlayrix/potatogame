#pragma once
#include "Booster.h"

class SpeedUpBooster : public Booster {
public:
    SpeedUpBooster(const Vector2& spawnPosition);
    ~SpeedUpBooster();

    void initialize(SDL_Renderer* renderer) override;
    void update(float deltaTime) override;
    void render(SDL_Renderer* renderer) override;
    void collect() override;
};


