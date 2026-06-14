#include "NPC.hpp"
#include "Map.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"
#include "GameFlags.hpp"
#include "Util/Animation.hpp"
#include "Util/GameObject.hpp"    // for M_Transform.translation()

#include <fstream>
#include <cstdlib>   // rand()
#include <cmath>     // std::abs()
#include <algorithm> // std::swap

// ============================================================
//  Helper: StringToDirection (static)
// ============================================================
Character::Direction NPC::StringToDirection(const std::string& s) {
    std::string upper = s;
    for (auto& c : upper) c = static_cast<char>(toupper(c));
    if (upper == "UP")    return Character::Direction::UP;
    if (upper == "DOWN")  return Character::Direction::DOWN;
    if (upper == "LEFT")  return Character::Direction::LEFT;
    if (upper == "RIGHT") return Character::Direction::RIGHT;
    return Character::Direction::DOWN; // default
}
// ============================================================
//  Constructor
// ============================================================
NPC::NPC(float x, float y, const std::string& spritePath, bool visible)
    : Character(x, y)
    , m_SpritePath(spritePath)
    , m_IsVisibleFromConfig(visible)
{
    m_Speed = 100.0f;

    // Stagger first decision so grouped NPCs don't all step simultaneously.
    m_MoveTimer = m_MoveInterval * (0.5f + (rand() % 100) / 100.0f);

    LoadSprites();
}

// ============================================================
//  SetSight
// ============================================================
void NPC::SetSight(int range, const std::string& facing) {
    m_HasSight = true;
    m_SightRange = range;
    m_SightDirection = StringToDirection(facing);
}

// ============================================================
//  Active state
// ============================================================
bool NPC::IsActive() const {
    if (!m_FlagToHide.empty()) {
        return !GameFlags::Get(m_FlagToHide);
    }
    return true;
}

// ============================================================
//  Update
// ============================================================
glm::vec2 NPC::Update(std::shared_ptr<Map> map) {
    // Locked NPCs (during dialogue or cutscenes) do not move.
    if (m_Locked) return glm::vec2(0.0f, 0.0f); 

    // If chasing but mid‑step, finish the current tile first
    if (m_Chasing && m_IsMoving) {
        glm::vec2 movement = Character::Update(map);
        m_Transform.translation += movement;
        return movement;
    }

    // If chasing and idle, decide the next action
    if (m_Chasing) {
        return ChasePlayer(map);
    }

    // Sight check
    if (m_HasSight && !m_Triggered && m_ActionType == NPCAction::BATTLE) {
        if (PlayerInSight(map)) {
            m_PlayerTargetX = map->GetPlayerGridX();
            m_PlayerTargetY = map->GetPlayerGridY();
            map->StartNPCTrainerApproach(this, m_PlayerTargetX, m_PlayerTargetY);
            m_Chasing = true;
            return m_Transform.translation;
        }
    } 

    // Visibility based on flags / config
    if (!IsActive() || !m_IsVisibleFromConfig) {
        SetVisible(false);
        if (!IsActive()) return glm::vec2(0.0f, 0.0f);
    } else {
        SetVisible(true);
    }

    // Finish any in‑progress tile movement
    if (m_IsMoving) {
        glm::vec2 movement = Character::Update(map);
        m_Transform.translation += movement;
        return movement;
    }

    // Freeze decisions during dialogue (but we already returned if locked)
    if (m_Locked) {
        return Character::Update(map);
    }

    // Decision timer
    float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;
    m_MoveTimer -= dt;
    if (m_MoveTimer > 0.0f) {
        glm::vec2 movement = Character::Update(map);
        m_Transform.translation += movement;
        return movement;
    }

    // Movement decision
    switch (m_MovementType) {
        case MovementType::LOOK_AROUND: DoLookAround(); break;
        case MovementType::WANDER:      DoWander(map);  break;
        case MovementType::PATROL:      DoPatrol(map);  break;
        case MovementType::STILL:       break;
    }

    // Reset timer with organic jitter
    float jitter = (rand() % 100) / 100.0f * m_MoveInterval * 0.4f;
    m_MoveTimer = m_MoveInterval + jitter;

    // Apply the first frame of any new movement
    glm::vec2 movement = Character::Update(map);
    m_Transform.translation += movement;
    return movement;
}

// ============================================================
//  Interaction
// ============================================================
void NPC::FaceToward(int playerGridX, int playerGridY) {
    int dx = playerGridX - m_GridX;
    int dy = playerGridY - m_GridY;

    if (std::abs(dx) >= std::abs(dy)) {
        SetDirection(dx > 0 ? Direction::RIGHT : Direction::LEFT);
    } else {
        SetDirection(dy > 0 ? Direction::DOWN : Direction::UP);
    }
}

std::vector<std::string> NPC::Interact(const Character& player) {
    FaceToward(player.GetGridX(), player.GetGridY());
    SetLocked(true);

    // If the interact flag is set, disable battle (one‑time fight)
    if (!m_InteractFlag.empty() && GameFlags::Get(m_InteractFlag)) {
        if (m_ActionType == NPCAction::BATTLE) {
            m_ActionType = NPCAction::NONE;
        }
    }

    // If a required flag is missing, suppress some actions
    if (!m_FlagRequired.empty() && !GameFlags::Get(m_FlagRequired)) {
        if (m_ActionType == NPCAction::WARP) {
            m_ActionType = NPCAction::NONE;
        }
    }

    // Return the first matching conditional dialogue (or default)
    for (const auto& entry : m_ConditionalDialogue) {
        if (!entry.condition.empty() && GameFlags::Get(entry.condition))
            return entry.lines;
    }
    return m_DefaultDialogue;
}

// ============================================================
//  Action API
// ============================================================
void NPC::SetAction(NPCAction type,
                    const std::string& data,
                    ItemCategory itemCategory) {
    m_ActionType     = type;
    m_ActionData     = data;
    m_ActionCategory = itemCategory;
}

// ============================================================
//  Movement helpers
// ============================================================
static Character::Direction DeltaToDirection(int dx, int dy) {
    if (dx > 0) return Character::Direction::RIGHT;
    if (dx < 0) return Character::Direction::LEFT;
    if (dy > 0) return Character::Direction::DOWN;
    return Character::Direction::UP;
}

bool NPC::TryMoveInDirection(int dx, int dy, std::shared_ptr<Map> map) {
    SetDirection(DeltaToDirection(dx, dy));
    return TryMove(dx, dy, map);
}

void NPC::DoLookAround() {
    static const Direction dirs[] = {
        Direction::DOWN, Direction::UP,
        Direction::LEFT, Direction::RIGHT
    };
    Direction next = m_Direction;
    int attempts = 0;
    while (next == m_Direction && attempts < 10) {
        next = dirs[rand() % 4];
        ++attempts;
    }
    SetDirection(next);
}

void NPC::DoWander(std::shared_ptr<Map> map) {
    if (rand() % 10 < 3) {
        DoLookAround();
        return;
    }

    int dx[] = { 0,  0, -1, 1 };
    int dy[] = { -1, 1,  0, 0 };

    for (int i = 3; i > 0; --i) {
        int j = rand() % (i + 1);
        std::swap(dx[i], dx[j]);
        std::swap(dy[i], dy[j]);
    }

    for (int i = 0; i < 4; ++i) {
        int nextX = m_GridX + dx[i];
        int nextY = m_GridY + dy[i];
        if (std::abs(nextX - m_SpawnGridX) > m_WanderRadius) continue;
        if (std::abs(nextY - m_SpawnGridY) > m_WanderRadius) continue;
        if (TryMoveInDirection(dx[i], dy[i], map)) return;
    }

    DoLookAround();
}

void NPC::DoPatrol(std::shared_ptr<Map> map) {
    if (m_PatrolPoints.size() < 2) return;

    auto [targetX, targetY] = m_PatrolPoints[m_PatrolIndex];
    int dx = 0, dy = 0;
    if      (targetX > m_GridX) dx =  1;
    else if (targetX < m_GridX) dx = -1;
    else if (targetY > m_GridY) dy =  1;
    else if (targetY < m_GridY) dy = -1;

    if (dx == 0 && dy == 0) {
        if (!m_PatrolReverse) {
            ++m_PatrolIndex;
            if (m_PatrolIndex >= static_cast<int>(m_PatrolPoints.size())) {
                m_PatrolIndex   = static_cast<int>(m_PatrolPoints.size()) - 2;
                m_PatrolReverse = true;
            }
        } else {
            --m_PatrolIndex;
            if (m_PatrolIndex < 0) {
                m_PatrolIndex   = 1;
                m_PatrolReverse = false;
            }
        }
        return;
    }

    TryMoveInDirection(dx, dy, map);
}

// ============================================================
//  Sprite loading
// ============================================================
bool NPC::FileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

std::vector<std::string> NPC::BuildWalkCycle(const std::string& base) const {
    const std::string f0 = base + ".png";
    const std::string f2 = base + "2.png";
    const std::string f3 = base + "3.png";

    if (!FileExists(f0)) {
        LOG_WARN("NPC::BuildWalkCycle — base frame missing: '{}'", f0);
        return {};
    }

    const bool has2 = FileExists(f2);
    const bool has3 = FileExists(f3);

    if ( has2 &&  has3) return { f0, f2, f0, f3 };
    if ( has2 && !has3) return { f0, f2 };
    if (!has2 &&  has3) return { f0, f3 };
    return { f0 };
}

void NPC::LoadSprites() {
    if (!m_IsVisibleFromConfig) {
        return;
    }

    auto downFrames  = BuildWalkCycle(m_SpritePath + "_Down");
    auto upFrames    = BuildWalkCycle(m_SpritePath + "_Up");
    auto leftFrames  = BuildWalkCycle(m_SpritePath + "_Left");
    auto rightFrames = BuildWalkCycle(m_SpritePath + "_Right");

    if (!downFrames.empty())
        m_AnimDown  = std::make_shared<Util::Animation>(downFrames,  false, 150, true, 0);
    if (!upFrames.empty())
        m_AnimUp    = std::make_shared<Util::Animation>(upFrames,    false, 150, true, 0);
    if (!leftFrames.empty())
        m_AnimLeft  = std::make_shared<Util::Animation>(leftFrames,  false, 150, true, 0);
    if (!rightFrames.empty())
        m_AnimRight = std::make_shared<Util::Animation>(rightFrames, false, 150, true, 0);

    // Fallback cascade
    if (!m_AnimDown && m_AnimUp)    m_AnimDown  = m_AnimUp;
    if (!m_AnimDown && m_AnimLeft)  m_AnimDown  = m_AnimLeft;
    if (!m_AnimDown && m_AnimRight) m_AnimDown  = m_AnimRight;

    if (!m_AnimUp)    m_AnimUp    = m_AnimDown;
    if (!m_AnimLeft)  m_AnimLeft  = (m_AnimRight ? m_AnimRight : m_AnimDown);
    if (!m_AnimRight) m_AnimRight = (m_AnimLeft  ? m_AnimLeft  : m_AnimDown);

    if (!m_AnimDown) {
        std::string dir = m_SpritePath;
        size_t slash = dir.find_last_of("/\\");
        if (slash != std::string::npos)
            dir = dir.substr(0, slash + 1);
        else
            dir.clear();

        std::string fallbackBase = dir + "Grant";
        LOG_WARN("NPC missing sprites for '{}', falling back to '{}'", m_SpritePath, fallbackBase);

        auto fallbackFrames = BuildWalkCycle(fallbackBase + "_Down");
        if (!fallbackFrames.empty()) {
            m_AnimDown  = std::make_shared<Util::Animation>(fallbackFrames, false, 150, true, 0);
            m_AnimUp    = m_AnimDown;
            m_AnimLeft  = m_AnimDown;
            m_AnimRight = m_AnimDown;
        } else {
            LOG_ERROR("FATAL: Could not load fallback sprite '{}'!", fallbackBase);
        }
    }

    m_CurrentAnimation = m_AnimDown;
    m_Drawable = m_CurrentAnimation;
}

// ============================================================
//  LoadDialogue (no-op, kept for interface)
// ============================================================
void NPC::LoadDialogue(const std::string&, std::vector<std::string>&) {
}

// ============================================================
//  Sight & Chase
// ============================================================
bool NPC::PlayerInSight(std::shared_ptr<Map> map) {
    int px = map->GetPlayerGridX();
    int py = map->GetPlayerGridY();

    int nx = m_GridX, ny = m_GridY;
    switch (m_SightDirection) {
        case Direction::UP:    return (px == nx && py < ny && py >= ny - m_SightRange);
        case Direction::DOWN:  return (px == nx && py > ny && py <= ny + m_SightRange);
        case Direction::LEFT:  return (py == ny && px < nx && px >= nx - m_SightRange);
        case Direction::RIGHT: return (py == ny && px > nx && px <= nx + m_SightRange);
    }
    return false;
}

glm::vec2 NPC::ChasePlayer(std::shared_ptr<Map> map) {
    int dx = m_PlayerTargetX - m_GridX;
    int dy = m_PlayerTargetY - m_GridY;

    // If already adjacent, make sure any in‑progress step is finished first
    if (std::abs(dx) <= 1 && std::abs(dy) <= 1 && !(dx == 0 && dy == 0)) {
        // Finish any in‑progress tile movement
        while (m_IsMoving) {
            Character::Update(map);
        }

        m_Chasing = false;
        m_Triggered = true;
        map->TriggerInteraction(this);
        return m_Transform.translation;
    }

    // Move one step towards the player
    int stepX = (dx > 0) ? 1 : (dx < 0) ? -1 : 0;
    int stepY = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;

    TryMoveInDirection(stepX, stepY, map);
    return m_Transform.translation;
}
