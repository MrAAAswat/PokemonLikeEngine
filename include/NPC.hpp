#ifndef NPC_HPP
#define NPC_HPP

#include "Character.hpp"
#include "Player.hpp"          // for Player::SetLocked / StopMoving
#include "Item.hpp"
#include "ShopData.hpp"

#include <string>
#include <vector>
#include <utility>
#include <memory>              // for std::shared_ptr

// ============================================================
//  NPCAction — what happens after dialogue closes
// ============================================================
enum class NPCAction {
    NONE, HEAL, SHOP, GIVE_ITEM, BATTLE, CHECK_ITEM,
    WARP, SELECT_STARTER, BUY_POKEMON
};

// ============================================================
//  MovementType — how the NPC behaves between interactions
// ============================================================
enum class MovementType {
    STILL, LOOK_AROUND, WANDER, PATROL
};

// ============================================================
//  NPCDialogueEntry — one flag-conditional dialogue block
// ============================================================
struct NPCDialogueEntry {
    std::string              condition;
    std::vector<std::string> lines;
};

// ============================================================
//  NPC
// ============================================================
class NPC : public Character {
public:
    NPC(float x, float y, const std::string& spritePath, bool visible = true);

    glm::vec2 Update(std::shared_ptr<Map> map) override;

    void SetDialogue(const std::vector<std::string>&      defaultLines,
                     const std::vector<NPCDialogueEntry>&  conditionalLines) {
        m_DefaultDialogue     = defaultLines;
        m_ConditionalDialogue = conditionalLines;
    }

    std::vector<std::string> Interact(const Character& player);
    void FaceToward(int playerGridX, int playerGridY);

    void SetLocked(bool locked) { m_Locked = locked; }
    bool IsLocked() const       { return m_Locked; }

    // Action
    void SetAction(NPCAction type, const std::string& data = "",
                   ItemCategory itemCategory = ItemCategory::GENERAL);
    NPCAction    GetActionType()     const { return m_ActionType; }
    std::string  GetActionData()     const { return m_ActionData; }
    ItemCategory GetActionCategory() const { return m_ActionCategory; }

    // Movement
    void SetMovementType(MovementType type) { m_MovementType = type; }
    void SetMoveInterval(float seconds)     { m_MoveInterval = seconds; }
    void SetWanderRadius(int radius)        { m_WanderRadius = radius; }
    void SetSpawnPoint(int x, int y)        { m_SpawnGridX = x; m_SpawnGridY = y; }
    void AddPatrolPoint(int gridX, int gridY) {
        m_PatrolPoints.emplace_back(gridX, gridY);
    }

    // Misc
    void SetDynamicZ(bool dynamic) { m_UseDynamicZ = dynamic; }

    // Flags
    void SetInteractFlag(const std::string& flag) { m_InteractFlag = flag; }
    const std::string& GetInteractFlag() const { return m_InteractFlag; }
    void SetFlagToHide(const std::string& flag) { m_FlagToHide = flag; }
    const std::string& GetFlagToHide() const { return m_FlagToHide; }

    // --- Sight & Chase ---
    void SetSight(int range, const std::string& facing);
    bool PlayerInSight(std::shared_ptr<Map> map);
    glm::vec2 ChasePlayer(std::shared_ptr<Map> map);
    void SetChaseTarget(int x, int y) { m_PlayerTargetX = x; m_PlayerTargetY = y; }

    // Helper to convert string to Direction
    static Character::Direction StringToDirection(const std::string& s);

    // Shop
    void SetShopItems(const std::vector<ShopItem>& items) { m_ShopItems = items; }
    const std::vector<ShopItem>& GetShopItems() const { return m_ShopItems; }

    // Reward (post‑battle item / money)
    void SetReward(const std::string& itemName, int qty) {
        m_RewardItemName = itemName;
        m_RewardQty      = qty;
    }
    void SetRewardMoney(int money) { m_RewardMoney = money; }
    std::string GetRewardItemName() const { return m_RewardItemName; }
    int         GetRewardQuantity()  const { return m_RewardQty; }
    int         GetRewardMoney()     const { return m_RewardMoney; }

    bool IsActive() const;
    void SetRequiredFlag(const std::string& flag) { m_FlagRequired = flag; }
    void SetAlwaysVisible(bool visible) { m_IsVisibleFromConfig = visible; }
    void SetVisualOffsetY(float offset) { m_VisualOffsetY = offset; }

protected:
    void LoadSprites() override;

    // Sight data
    bool      m_HasSight      = false;
    int       m_SightRange    = 3;
    Direction m_SightDirection = Direction::DOWN;

    // Chase / trigger state
    bool m_Chasing       = false;
    bool m_Triggered     = false;
    int  m_PlayerTargetX = -1;
    int  m_PlayerTargetY = -1;

private:
    std::string m_SpritePath;
    bool        m_IsVisibleFromConfig = true;
    std::vector<ShopItem> m_ShopItems;

    std::vector<std::string>      m_DefaultDialogue;
    std::vector<NPCDialogueEntry> m_ConditionalDialogue;
    std::string                   m_FlagCondition;   // unused, kept for legacy

    NPCAction    m_ActionType     = NPCAction::NONE;
    std::string  m_ActionData     = "";
    ItemCategory m_ActionCategory = ItemCategory::GENERAL;
    std::string  m_FlagRequired;

    MovementType m_MovementType = MovementType::STILL;
    bool         m_Locked       = false;
    float        m_MoveTimer    = 0.0f;
    float        m_MoveInterval = 2.0f;
    float m_VisualOffsetY = 0.0f;

    int m_SpawnGridX = 0, m_SpawnGridY = 0;
    int m_WanderRadius = 3;

    std::vector<std::pair<int, int>> m_PatrolPoints;
    int  m_PatrolIndex   = 0;
    bool m_PatrolReverse = false;

    std::string m_RewardItemName;
    int         m_RewardQty    = 0;
    int         m_RewardMoney  = 0;
    std::string m_InteractFlag;
    std::string m_FlagToHide;

    // Helpers
    void LoadDialogue(const std::string& path, std::vector<std::string>& out);
    std::vector<std::string> BuildWalkCycle(const std::string& baseFramePath) const;
    static bool FileExists(const std::string& path);
    bool TryMoveInDirection(int dx, int dy, std::shared_ptr<Map> map);
    void DoLookAround();
    void DoWander(std::shared_ptr<Map> map);
    void DoPatrol(std::shared_ptr<Map> map);
};

#endif // NPC_HPP