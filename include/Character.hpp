#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include "pch.hpp"
#include "Util/GameObject.hpp"
#include "GameConfig.hpp"
#include "Util/Animation.hpp" 
#include "Item.hpp" 
#include "Pokemon.hpp" // <-- 1. ADD THIS INCLUDE
#include <vector>
#include <memory>
#include <unordered_map>

class Map;

struct InventoryData {
    int quantity;
    ItemCategory category = ItemCategory::GENERAL;;
};

class Character : public Util::GameObject {
public:
    enum class Direction { DOWN, UP, LEFT, RIGHT };
    enum class State { IDLE, MOVING };
    
    Character(float x, float y);
    virtual ~Character() = default; 

    int GetGridX() const { return m_GridX; }
    int GetGridY() const { return m_GridY; }
    void SetGridPosition(int x, int y) { m_GridX = x; m_GridY = y; }
    
    bool IsMoving() const { return m_IsMoving; }
    void StopMoving();
    void SetBaseZIndex(float z) { m_BaseZIndex = z; }
    
    virtual glm::vec2 Update(std::shared_ptr<Map> map);
    void SetDirection(Direction dir);
    Direction GetFacingDirection() const { return m_Direction; }

    // --- INVENTORY SYSTEM ---
    void AddItem(const std::string& itemName, ItemCategory category, int amount = 1);
    bool RemoveItem(const std::string& itemName, int amount = 1);
    int GetItemCount(const std::string& itemName) const;
    void PrintInventory() const; 
    
    std::unordered_map<std::string, InventoryData> GetInventory() const { return m_Inventory; }
    void SetInventory(const std::unordered_map<std::string, InventoryData>& loadedInv) { m_Inventory = loadedInv; }

    // --- NEW: POKEMON PARTY SYSTEM ---
    bool AddPokemon(std::shared_ptr<Pokemon> newPokemon) {
        if (m_PokemonParty.size() < 6) {
            m_PokemonParty.push_back(newPokemon);
            return true; // Successfully added!
        }
        return false; // Party is full!
    }

    std::vector<std::shared_ptr<Pokemon>>& GetParty() { return m_PokemonParty; }

    bool HasUsablePokemon() const {
        for (const auto& pkmn : m_PokemonParty) {
            if (!pkmn->IsFainted()) return true;
        }
        return false;
    }
    int  GetMoney() const { return m_Money; }
    void SetMoney(const int money) {m_Money = money;} 
    
    void AddMoney(int amount) { m_Money += amount; }
    bool SpendMoney(int amount) {
        if (m_Money >= amount) {
            m_Money -= amount;
            return true;
        }
        return false;
    }
    void SnapToGrid();

protected:
    // The Backpack
    std::unordered_map<std::string, InventoryData> m_Inventory;
    
    // The Pokemon Team (Max 6) <-- 2. ADD THIS VECTOR
    std::vector<std::shared_ptr<Pokemon>> m_PokemonParty;
    
    virtual void LoadSprites() = 0; 
    //bool m_UseDynamicZ = true;
    bool m_UseDynamicZ = true;
    virtual void UpdateSprite();
    float m_BaseZIndex = 0.8f;
    float m_FootOffsetY = 0.0f;
    
    bool TryMove(int dx, int dy, std::shared_ptr<Map> map);

    bool m_IsMoving = false;                
    glm::vec2 m_CurrentDirection = {0, 0};  
    float m_PixelsMoved = 0.0f;             
    float m_Speed = 200.0f; 

    int m_GridX = 6;
    int m_GridY = 6;

    Direction m_Direction = Direction::DOWN;
    State m_State = State::IDLE;

    std::shared_ptr<Util::Animation> m_AnimDown;
    std::shared_ptr<Util::Animation> m_AnimUp;
    std::shared_ptr<Util::Animation> m_AnimLeft;
    std::shared_ptr<Util::Animation> m_AnimRight;
    std::shared_ptr<Util::Animation> m_CurrentAnimation;
    float m_SpeedMultiplier = 1.0f;
    
    int m_Money = 3000; 

};

#endif