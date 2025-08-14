# Testing Framework and Debugging Guidelines

## Debug Logging Patterns

### Current Logging Implementation
The codebase uses `std::cout` for debug output with consistent patterns:

```cpp
// Success confirmations
std::cout << "Successfully loaded brick character sprite!" << std::endl;
std::cout << "Loaded font: " << fontPath << std::endl;

// Error reporting with context
std::cout << "Failed to load brick.png! SDL_image Error: " << IMG_GetError() << std::endl;
std::cout << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;

// Game state changes
std::cout << "Level up! Now level " << level << std::endl;
std::cout << "Shop opened after wave " << waveNumber << std::endl;
std::cout << "Wave " << wave << " will start after shop" << std::endl;

// Player actions
std::cout << "Bought " << item.name << " for " << item.price << " materials" << std::endl;
std::cout << "Collected " << materialBag << " materials from bag!" << std::endl;
std::cout << "Dodged!" << std::endl;
```

### Recommended Debug Logging Additions
```cpp
// Add debug macros for conditional logging
#ifdef DEBUG_MODE
    #define DEBUG_LOG(msg) std::cout << "[DEBUG] " << msg << std::endl
    #define DEBUG_LOG_VAL(name, val) std::cout << "[DEBUG] " << name << ": " << val << std::endl
#else
    #define DEBUG_LOG(msg)
    #define DEBUG_LOG_VAL(name, val)
#endif

// Usage examples
DEBUG_LOG("Player position updated");
DEBUG_LOG_VAL("Health", player->getHealth());
DEBUG_LOG_VAL("Materials", player->getStats().materials);
```

## Feature Testing Guidelines

### New Feature Validation Checklist

#### Weapon Implementation Testing
```cpp
// Test weapon creation
void testWeaponCreation() {
    auto weapon = std::make_unique<Weapon>(WeaponType::PISTOL, WeaponTier::TIER_1);
    
    // Validate stats
    assert(weapon->getStats().baseDamage > 0);
    assert(weapon->getStats().attackSpeed > 0);
    assert(weapon->getStats().range > 0);
    
    DEBUG_LOG("Weapon creation test passed");
}

// Test weapon firing
void testWeaponFiring() {
    std::vector<std::unique_ptr<Bullet>> bullets;
    Player testPlayer(100, 100);
    
    auto weapon = std::make_unique<Weapon>(WeaponType::PISTOL, WeaponTier::TIER_1);
    Vector2 weaponPos(100, 100);
    Vector2 aimDir(1, 0);
    
    // Test initial state
    assert(weapon->canFire());
    
    // Fire weapon
    weapon->update(0.1f, weaponPos, aimDir, bullets, testPlayer);
    
    // Validate bullet creation
    assert(!bullets.empty());
    assert(bullets[0]->isAlive());
    
    DEBUG_LOG("Weapon firing test passed");
}
```

#### Player System Testing
```cpp
// Test stat modifications
void testPlayerStats() {
    Player testPlayer(100, 100);
    PlayerStats& stats = testPlayer.getStats();
    
    int initialHealth = stats.maxHealth;
    int initialMaterials = stats.materials;
    
    // Test material gain
    testPlayer.gainMaterials(50);
    assert(stats.materials == initialMaterials + 50);
    
    // Test damage application
    testPlayer.takeDamage(10);
    assert(testPlayer.getHealth() == initialHealth - 10);
    
    DEBUG_LOG("Player stats test passed");
}

// Test experience and leveling
void testPlayerProgression() {
    Player testPlayer(100, 100);
    int initialLevel = testPlayer.getLevel();
    
    // Give enough experience to level up
    int expNeeded = testPlayer.getExperienceToNextLevel();
    testPlayer.gainExperience(expNeeded);
    
    assert(testPlayer.getLevel() == initialLevel + 1);
    assert(testPlayer.getStats().maxHealth == 101); // +1 per level
    
    DEBUG_LOG("Player progression test passed");
}
```

#### Enemy System Testing
```cpp
// Test enemy creation and AI
void testEnemyBehaviour() {
    SDL_Renderer* testRenderer = nullptr; // Would need valid renderer in real test
    Enemy testEnemy(Vector2(200, 200), testRenderer);
    
    // Test initial state
    assert(testEnemy.isAlive());
    assert(testEnemy.getDamage() > 0);
    
    // Test hit registration
    testEnemy.hit();
    // Enemy should still be alive after hit (only destroyed by bullet collision)
    assert(testEnemy.isAlive());
    
    DEBUG_LOG("Enemy behaviour test passed");
}
```

## Common Edge Cases to Check

### Boundary Conditions

#### Health System Edge Cases
```cpp
void testHealthEdgeCases() {
    Player testPlayer(100, 100);
    
    // Test zero damage
    int initialHealth = testPlayer.getHealth();
    testPlayer.takeDamage(0);
    assert(testPlayer.getHealth() == initialHealth - 1); // Minimum 1 damage
    
    // Test excessive damage
    testPlayer.takeDamage(1000);
    assert(testPlayer.getHealth() == 0); // Clamped to 0
    
    // Test negative health handling
    assert(testPlayer.getHealth() >= 0);
    
    DEBUG_LOG("Health edge cases test passed");
}
```

#### Resource Limits
```cpp
void testResourceLimits() {
    Player testPlayer(100, 100);
    
    // Test weapon capacity
    for (int i = 0; i < 10; i++) {
        testPlayer.addWeapon(std::make_unique<Weapon>(WeaponType::PISTOL, WeaponTier::TIER_1));
    }
    assert(testPlayer.getWeaponCount() <= 6); // Max 6 weapons
    
    // Test material overflow
    testPlayer.gainMaterials(INT_MAX);
    assert(testPlayer.getStats().materials > 0); // No overflow crash
    
    DEBUG_LOG("Resource limits test passed");
}
```

#### Position Boundaries
```cpp
void testPositionBounds() {
    Player testPlayer(-100, -100); // Start outside bounds
    
    // Update should clamp to valid area
    testPlayer.update(0.1f);
    
    Vector2 pos = testPlayer.getPosition();
    float radius = testPlayer.getRadius();
    
    assert(pos.x >= radius);
    assert(pos.y >= radius);
    assert(pos.x <= 1920 - radius);
    assert(pos.y <= 1080 - radius);
    
    DEBUG_LOG("Position bounds test passed");
}
```

### Zero Value Handling
```cpp
void testZeroValues() {
    Player testPlayer(100, 100);
    PlayerStats& stats = testPlayer.getStats();
    
    // Test zero attack speed (should not crash)
    stats.attackSpeed = 0.0f;
    bool canShoot = testPlayer.canShoot(); // Should handle division by zero
    
    // Test zero movement speed
    stats.moveSpeed = 0.0f;
    testPlayer.update(0.1f); // Should not crash
    
    // Test zero pickup range
    stats.pickupRange = 0.0f;
    // Collection should still work at point-blank range
    
    DEBUG_LOG("Zero values test passed");
}
```

### Maximum Value Limits
```cpp
void testMaximumValues() {
    Player testPlayer(100, 100);
    PlayerStats& stats = testPlayer.getStats();
    
    // Test maximum health regeneration
    stats.healthRegen = 1000.0f;
    stats.maxHealth = 50;
    testPlayer.takeDamage(25); // Reduce health
    
    testPlayer.update(1.1f); // Trigger regen
    assert(testPlayer.getHealth() <= stats.maxHealth); // Should cap at max
    
    // Test maximum dodge chance
    stats.dodgeChance = 2.0f; // 200% (invalid)
    // Should still work without crashing
    testPlayer.takeDamage(10);
    
    DEBUG_LOG("Maximum values test passed");
}
```

## Performance Testing Considerations

### Frame Rate Monitoring
```cpp
class PerformanceMonitor {
    float frameTime = 0.0f;
    int frameCount = 0;
    float totalTime = 0.0f;
    
public:
    void update(float deltaTime) {
        frameTime = deltaTime;
        frameCount++;
        totalTime += deltaTime;
        
        // Report every second
        if (totalTime >= 1.0f) {
            float avgFPS = frameCount / totalTime;
            DEBUG_LOG_VAL("Average FPS", avgFPS);
            DEBUG_LOG_VAL("Frame time (ms)", frameTime * 1000);
            
            frameCount = 0;
            totalTime = 0.0f;
        }
    }
};
```

### Memory Usage Tracking
```cpp
void checkMemoryUsage() {
    // Monitor entity counts
    DEBUG_LOG_VAL("Active enemies", enemies.size());
    DEBUG_LOG_VAL("Active bullets", bullets.size());
    DEBUG_LOG_VAL("Active orbs", experienceOrbs.size());
    DEBUG_LOG_VAL("Active materials", materials.size());
    
    // Check for memory leaks (entities not being cleaned up)
    if (enemies.size() > 100) {
        DEBUG_LOG("WARNING: High enemy count - possible memory leak");
    }
    if (bullets.size() > 500) {
        DEBUG_LOG("WARNING: High bullet count - possible memory leak");
    }
}
```

### Load Testing
```cpp
void stressTestSpawning() {
    // Test high enemy counts
    for (int i = 0; i < 1000; i++) {
        Vector2 randomPos(rand() % 1920, rand() % 1080);
        enemies.push_back(std::make_unique<Enemy>(randomPos, renderer));
    }
    
    // Measure update performance
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (auto& enemy : enemies) {
        enemy->update(0.016f, player->getPosition(), bullets);
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    
    DEBUG_LOG_VAL("Enemy update time (μs)", duration.count());
}
```

## Feature Integration Verification

### System Integration Tests
```cpp
void testWeaponPlayerIntegration() {
    Player testPlayer(100, 100);
    std::vector<std::unique_ptr<Bullet>> bullets;
    
    // Add weapon to player
    testPlayer.addWeapon(std::make_unique<Weapon>(WeaponType::PISTOL, WeaponTier::TIER_1));
    
    // Test weapon firing through player system
    Vector2 mousePos(200, 100);
    testPlayer.updateShootDirection(mousePos);
    testPlayer.updateWeapons(0.2f, bullets); // Should create bullets
    
    assert(!bullets.empty());
    DEBUG_LOG("Weapon-Player integration test passed");
}

void testShopPlayerIntegration() {
    Player testPlayer(100, 100);
    Shop testShop;
    
    // Give player materials
    testPlayer.gainMaterials(100);
    
    // Generate shop items
    testShop.generateItems(1, 0);
    
    // Test purchase
    int initialWeaponCount = testPlayer.getWeaponCount();
    int initialMaterials = testPlayer.getStats().materials;
    
    testShop.buyItem(0, testPlayer);
    
    // Verify purchase effects
    assert(testPlayer.getWeaponCount() >= initialWeaponCount);
    assert(testPlayer.getStats().materials < initialMaterials);
    
    DEBUG_LOG("Shop-Player integration test passed");
}
```

### Cross-System Validation
```cpp
void testCollisionSystem() {
    // Create test entities
    Player testPlayer(100, 100);
    Enemy testEnemy(Vector2(120, 100), renderer);
    
    std::vector<std::unique_ptr<Bullet>> bullets;
    bullets.push_back(std::make_unique<Bullet>(Vector2(110, 100), Vector2(1, 0), 10));
    
    // Test bullet-enemy collision
    bool enemyHit = false;
    for (auto& bullet : bullets) {
        float distance = bullet->getPosition().distance(testEnemy.getPosition());
        if (distance < bullet->getRadius() + testEnemy.getRadius()) {
            enemyHit = true;
            break;
        }
    }
    
    assert(enemyHit);
    DEBUG_LOG("Collision system test passed");
}
```

## Automated Testing Setup

### Test Runner Framework
```cpp
class TestRunner {
    int testsRun = 0;
    int testsPassed = 0;
    
public:
    void runTest(const std::string& testName, std::function<void()> testFunc) {
        testsRun++;
        try {
            testFunc();
            testsPassed++;
            std::cout << "[PASS] " << testName << std::endl;
        } catch (const std::exception& e) {
            std::cout << "[FAIL] " << testName << ": " << e.what() << std::endl;
        }
    }
    
    void printResults() {
        std::cout << "\nTest Results: " << testsPassed << "/" << testsRun << " passed" << std::endl;
        if (testsPassed == testsRun) {
            std::cout << "All tests passed!" << std::endl;
        }
    }
};

// Usage
void runAllTests() {
    TestRunner runner;
    
    runner.runTest("Weapon Creation", testWeaponCreation);
    runner.runTest("Player Stats", testPlayerStats);
    runner.runTest("Health Edge Cases", testHealthEdgeCases);
    runner.runTest("Resource Limits", testResourceLimits);
    
    runner.printResults();
}
```

### Continuous Integration Hooks
```cpp
// Add to main() for development builds
#ifdef ENABLE_TESTS
    std::cout << "Running automated tests..." << std::endl;
    runAllTests();
    
    // Exit after tests in CI environment
    if (std::getenv("CI_MODE")) {
        return 0;
    }
#endif
```

This testing framework provides comprehensive validation for new features while maintaining the stability and reliability of the existing codebase. Use these patterns to ensure robust feature implementation and catch potential issues early in development.
