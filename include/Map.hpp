#ifndef MAP_HPP
#define MAP_HPP

#include "pch.hpp"
#include <functional>
#include "Vehicle.hpp"
#include "NPC.hpp"
#include "Character.hpp"
#include "Item.hpp"
#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Time.hpp"
#include "Util/Animation.hpp"
#include "ShopData.hpp" 
#include <vector>
#include <string>
#include <memory>
#include <unordered_map> 

// Blueprint for every tile
struct TileProperties {
    std::shared_ptr<Util::Image> texture;
    float zIndex = 0.0f;
    float yOffset = 0.0f;
    bool isWalkable = false; 
};

enum class PropAnimMode {
    STATIC,       // Default — never changes frame automatically
    LOOP,         // Cycles through all frames continuously  
    PING_PONG     // Plays forward then backward
};

struct PropProperties {
    std::vector<std::string> texturePaths;
    float zIndex = 0.8f;
    bool dynamicZ = true; 
    bool isWalkable = false;
    float visualOffsetX = 0.0f; 
    float visualOffsetY = 0.0f; 
    PropAnimMode animMode = PropAnimMode::STATIC;  
    int animFrameDelay = 8; 
};

struct PatrolPoint {
    int gridX;
    int gridY;
};


struct NPCProperties {
    std::string  texturePath;
    float        visualOffsetY;
    float        zIndex;
    bool         dynamicZ;
    NPCAction    actionType   = NPCAction::NONE;
    std::string  actionData   = "";
    std::vector<ShopItem> shopItems;
    ItemCategory itemCategory = ItemCategory::GENERAL;
    std::string  flagOnInteract;
    std::string  flagToHide;
    std::string  flagRequired;
    bool         visible = true;
    std::string initialFacing = "Down";
    MovementType             movementType = MovementType::STILL;
    float                    moveInterval = 2.0f;
    int                      wanderRadius = 3;
    std::vector<PatrolPoint> patrolPoints = {};
    int         sightRange     = 0;         
    std::string sightDirection = "Down";       

    // Dialogue — default first, then flag-conditional entries in priority order
    std::vector<std::string>      defaultDialogue;
    std::vector<NPCDialogueEntry> conditionalDialogue;
    std::string flagToShow;
    
    int rewardMoney;
    struct Reward {
        std::string itemName;
        int         quantity = 1;
        ItemCategory category = ItemCategory::GENERAL;
    } reward;
};

struct ItemProperties {
    std::string  texturePath;
    std::string  shopTexturePath;
    std::string  name;
    ItemCategory category;
    float        zIndex;
    int          buyPrice  = 0;   
    int          sellPrice = 0;  
};

class Item;
class Prop;
class Player; 

class Map : public Util::GameObject, public std::enable_shared_from_this<Map> {
public:
    Map(); 
    void WarpTo(int gridX, int gridY);
    int GetPropType(int gridX, int gridY);
    void Move(float dx, float dy);
    void Draw(); 
    bool IsWalkable(int x, int y);
    void Update();
    int GetTileType(int gridX, int gridY);
    void LoadLevel(const std::string& mapName);
    std::shared_ptr<NPC> GetNPCAt(int gridX, int gridY);
    std::string GetCurrentLevelPath() const { return m_CurrentLevelPath; }
    std::string CollectItemAt(int gridX, int gridY, Character& player);
    void SetRenderer(std::weak_ptr<Util::Renderer> renderer);
    void UpdateSteppedProps(int playerGridX, int playerGridY);

    void SetVisible(bool visible);
    void LoadConnections(const std::string& filepath);
    void LoadGeneratedLevel(const std::string& mapName,
                            std::vector<std::vector<int>> groundData,
                            std::vector<std::vector<int>> propData);

    void SetPlayerGridPosition(int x, int y) {
        m_PlayerGridX = x;
        m_PlayerGridY = y;
    }
    void SetPaused(bool paused) { m_Paused = paused; }
    bool IsPaused()       const { return m_Paused; }
    static NPCAction    StringToAction  (const std::string& s);
    static MovementType StringToMovement(const std::string& s);
    static ItemCategory StringToCategory(const std::string& s);   
    float GetWorldOffsetX() const { return m_WorldOffsetX; }
    float GetWorldOffsetY() const { return m_WorldOffsetY; }
    void EndNPCTrainerApproach();
    void StartNPCTrainerApproach(NPC* npc, int targetX, int targetY);
    bool IsNPCTrainerApproachActive() const { return m_NPCApproachActive; }
    void InitiateBattleWithNPC(NPC* npc);
    void TriggerInteraction(NPC* npc);

    void SetPlayer(Player* player) { m_Player = player; }
    Player* GetPlayer() const { return m_Player; }

    // Player grid getters
    int GetPlayerGridX() const { return m_PlayerGridX; }
    int GetPlayerGridY() const { return m_PlayerGridY; }
    void SetBattleCallback(std::function<void(NPC*)> callback) { m_BattleCallback = callback; }
    void SetInteractionCallback(std::function<void(NPC*)> cb) { m_InteractionCallback = cb; }
    void UpdateApproachTarget();

private:

    Player* m_Player = nullptr;
    bool    m_NPCApproachActive = false;
    // --- MAP DATA ---
    std::function<void(NPC*)> m_BattleCallback;
    std::vector<std::shared_ptr<NPC>> m_NPCs;
    std::vector<std::shared_ptr<Vehicle>> m_Vehicles;
    std::vector<std::vector<int>> m_PropData;
    std::vector<std::vector<int>> m_LevelData;
    std::vector<std::shared_ptr<Util::GameObject>> m_Tiles;
    std::vector<std::shared_ptr<Prop>> m_Props;
    std::vector<std::shared_ptr<Item>> m_Items;
    std::string m_CurrentLevelPath;
    std::weak_ptr<Util::Renderer> m_Renderer;
    std::function<void(NPC*)> m_InteractionCallback;

    float m_WorldOffsetX = 0.0f;
    float m_WorldOffsetY = 0.0f;


    //
    int  m_PlayerGridX = -1;
    int  m_PlayerGridY = -1;
    bool m_Paused      = false;
    NPC* m_ApproachNPC = nullptr;
    // --- THE REGISTRY ---
    std::unordered_map<int, TileProperties> m_TileRegistry;
    std::unordered_map<int, NPCProperties> m_NPCRegistry;
    std::unordered_map<int, PropProperties> m_PropRegistry; 
    std::unordered_map<int, ItemProperties> m_ItemRegistry;
    
    void InitTileRegistry(); 
    void InitPropRegistry();
    void InitNPCRegistry(); 
    void InitItemRegistry();
    void AddToRenderer(std::shared_ptr<Util::GameObject> obj);
    void SpawnTilesAndProps();

    // --- WATER ANIMATION (independent of tile visibility) ---
    std::shared_ptr<Util::Animation> m_FollowerWater;   // the actual animation seen on all water tiles
    int   m_WaterFrameCount   = 0;     // number of frames in the water animation
    float m_WaterFrameTimer   = 0.0f;  // accumulator (ms)
    int   m_WaterCurrentFrame = 0;     // which frame the follower should display
    float m_WaterFrameDelay   = 500.0f; // milliseconds between frames

    // --- HELPER FUNCTIONS ---
    std::vector<std::vector<int>> LoadCSV(const std::string& filepath);
    void ClearMap();

    int m_OutOfBoundsPropID = -1;
    std::vector<bool> m_TileVisible; // parallel to m_Tiles
    
    void LoadNPCsFromJSON(const std::string& path);
    void LoadTilesFromJSON(const std::string& path);
    void LoadPropsFromJSON(const std::string& path);
    void LoadItemsFromJSON(const std::string& path);
};

#endif // MAP_HPP