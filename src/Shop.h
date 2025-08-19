#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <memory>
#include <string>
#include "Weapon.h"

class Player;
class Game;

enum class ShopItemType {
    WEAPON,
    ITEM  // For future items
};

// New enum for specific item types
enum class ItemType {
    HEALTH_REGENERATION,
    // Future items can be added here: ARMOR, SPEED_BOOST, etc.
};

struct ShopItem {
    ShopItemType type;
    
    // Weapon-specific fields
    WeaponType weaponType;
    WeaponTier tier;
    
    // Item-specific fields
    ItemType itemType;
    int itemLevel;  // Level of the item (1-4, similar to weapon tiers)
    
    int price;
    bool locked;
    std::string name;
    std::string description;
    
    // Constructor for weapons
    ShopItem(WeaponType wType, WeaponTier wTier, int cost) 
        : type(ShopItemType::WEAPON), weaponType(wType), tier(wTier), 
          itemType(ItemType::HEALTH_REGENERATION), itemLevel(1),
          price(cost), locked(false) {
        
        // Generate name and description
        name = getWeaponName(wType, wTier);
        description = getWeaponDescription(wType, wTier);
    }
    
    // Constructor for items
    ShopItem(ItemType iType, int iLevel, int cost)
        : type(ShopItemType::ITEM), weaponType(WeaponType::PISTOL), tier(WeaponTier::TIER_1),
          itemType(iType), itemLevel(iLevel), price(cost), locked(false) {
        
        // Generate name and description for items
        name = getItemName(iType, iLevel);
        description = getItemDescription(iType, iLevel);
    }
    
private:
    std::string getWeaponName(WeaponType wType, WeaponTier wTier);
    std::string getWeaponDescription(WeaponType wType, WeaponTier wTier);
    std::string getItemName(ItemType iType, int level);
    std::string getItemDescription(ItemType iType, int level);
};

class Shop {
public:
    Shop();
    ~Shop();
    
    void setGame(Game* game) { gameRef = game; }
    void loadAssets(SDL_Renderer* renderer);
    void unloadAssets();
    
    void generateItems(int waveNumber, int playerLuck = 0);
    void render(SDL_Renderer* renderer, int windowWidth, int windowHeight);
    void handleInput(const Uint8* keyState, Player& player);
    void handleMouseInput(int mouseX, int mouseY, bool mousePressed, Player& player);
    
    bool isShopActive() const { return active; }
    void openShop(int waveNumber);
    void closeShop();
    
    // Shop actions
    void buyItem(int index, Player& player);
    void lockItem(int index);
    void reroll(Player& player, int waveNumber);
    
private:
    bool active;
    std::vector<ShopItem> items;
    int rerollCount;
    int currentWave;
    
    // UI helpers
    void renderShopItem(SDL_Renderer* renderer, const ShopItem& item, int x, int y, int width, int height, bool highlighted, int index);
    void renderCharacterStats(SDL_Renderer* renderer, int x, int y, int width, int height);
    void renderText(SDL_Renderer* renderer, const char* text, int x, int y, int scale = 1);
    void renderNumber(SDL_Renderer* renderer, int number, int x, int y, int scale = 1);
    void renderTTFText(SDL_Renderer* renderer, const char* text, int x, int y, SDL_Color color, int fontSize = 16);
    SDL_Texture* loadTexture(const char* path, SDL_Renderer* renderer);
    
    // Shop logic
    int calculateItemPrice(WeaponType weaponType, WeaponTier tier, int waveNumber);
    int calculateItemPrice(ItemType itemType, int itemLevel, int waveNumber);
    int calculateRerollPrice(int waveNumber, int rerollCount);
    bool canItemAppear(WeaponType weaponType, WeaponTier tier, int waveNumber);
    bool canItemAppear(ItemType itemType, int itemLevel, int waveNumber);
    
    // Input handling
    int selectedItem;
    bool keyPressed[4]; // Track key states to prevent multiple presses
    int hoveredItem;
    bool lastMousePressed;
    int selectedOwnedWeapon; // index of selected owned weapon in player's array (-1 if none)
    
    Game* gameRef = nullptr; // Reference to game for text rendering
    
    static const int MAX_SHOP_ITEMS = 4;

    // UI textures
    SDL_Texture* texCardNormal = nullptr;
    SDL_Texture* texCardSelected = nullptr;
    SDL_Texture* texCardLocked = nullptr;
    SDL_Texture* texCoin = nullptr;
    SDL_Texture* texLock = nullptr;
    SDL_Texture* texLockLocked = nullptr;
    SDL_Texture* texReroll = nullptr;
    SDL_Texture* texWeaponPistol = nullptr;
    SDL_Texture* texWeaponSMG = nullptr;
    SDL_Texture* texWeaponShotgun = nullptr;
    SDL_Texture* texWeaponSniper = nullptr;
    SDL_Texture* texHealthRegen = nullptr;
};