#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <vector>
#include <memory>
#include "Player.h"
#include "Enemy.h"
#include "Bullet.h"
#include "ExperienceOrb.h"
#include "Material.h"
#include "Weapon.h"
#include "Shop.h"
#include "SpeedUpBooster.h"
#include "HealingBooster.h"

// Forward declarations
class SlimeEnemy;
class PebblinEnemy;
class BossEnemy;
class FractalBoss;

enum class EnemySpawnType {
    BASE,
    SLIME,
    PEBBLIN,
    BOSS,
    MINIBOSS
};

struct SpawnIndicator {
    Vector2 position;
    float elapsed;
    float duration; // seconds
    EnemySpawnType enemyType;
    
    SpawnIndicator(const Vector2& pos, float dur, EnemySpawnType type)
        : position(pos), elapsed(0.0f), duration(dur), enemyType(type) {}
    
    bool isComplete() const { return elapsed >= duration; }
};

class Game {
public:
    Game();
    ~Game();
    
    bool init();
    void run();
    void cleanup();
    
    void renderNumber(int number, int x, int y, int scale = 1);
    void renderText(const char* text, int x, int y, int scale = 1);
    void renderTTFText(const char* text, int x, int y, SDL_Color color, int fontSize = 16);
    int getPlayerMaterials() const { return player ? player->getStats().materials : 0; }
    const Player* getPlayer() const { return player.get(); }
    SDL_Renderer* getRenderer() const { return renderer; }
    
private:
    void handleEvents();
    void update(float deltaTime);
    void render();
    void spawnEnemies();
    void updateSpawnIndicators(float deltaTime);
    void renderSpawnIndicators();
    void checkCollisions();
    void checkMeleeAttacks();
    void updateExperienceCollection();
    void updateMaterialCollection();
    void updateBoosterCollection();
    float getMaterialDropChance() const;
    void renderUI();
    
    // Boss spawning helpers
    bool shouldSpawnFractalBoss() const;
    
private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Bullet>> bullets;
    
    // Босс система - только один босс за волну
    std::unique_ptr<Enemy> currentBoss;
    bool bossSpawnedThisWave;
    bool swarmSpawnedThisWave;
    
    // Система предотвращения повторения боссов подряд
    enum class BossType {
        NONE,
        REGULAR,
        FRACTAL
    };
    BossType lastBossType;
    
    std::vector<SpawnIndicator> spawnIndicators;
    std::vector<std::unique_ptr<ExperienceOrb>> experienceOrbs;
    std::vector<std::unique_ptr<Material>> materials;
    std::unique_ptr<SpeedUpBooster> speedUpBooster; // at most one
    std::unique_ptr<HealingBooster> healingBooster; // at most one
    float boosterSpawnTimer = 0.0f; // spawns every 10 seconds
    float healingBoosterSpawnTimer = 0.0f; // spawns every 15 seconds
    
    float timeSinceLastSpawn;
    int score;
    int wave;
    Vector2 mousePos;
    
    // Brotato-like wave system
    float waveTimer;
    float waveDuration;
    bool waveActive;
    
    // Materials system (Brotato-style)
    int materialBag; // Materials stored for next wave
    static const int MAX_MATERIALS_ON_MAP = 50;
    
    // Shop system
    std::unique_ptr<Shop> shop;
    
    // TTF Font system
    TTF_Font* defaultFont;
    
    // Telegraph duration for spawn indicators (seconds)
    float spawnTelegraphSeconds = 2.0f;
    
    static const int WINDOW_WIDTH = 1920;
    static const int WINDOW_HEIGHT = 1080;
};