#include "Vehicle.hpp"
#include "Map.hpp"
#include "Util/Time.hpp"
#include "Util/Logger.hpp"

Vehicle::Vehicle(float x, float y, const std::string& texturePath)
    : Character(x, y)
    , m_TexturePath(texturePath)
{
    m_Speed = 200.0f; 
    m_Direction = Direction::UP;
    m_State = State::MOVING;
    m_IsMoving = false;
    m_UseDynamicZ = false; 
    m_BaseZIndex = 0.5f; 
    LoadSprites();
}

glm::vec2 Vehicle::Update(std::shared_ptr<Map> map) {
    float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    // 1. If waiting for spawn, tick timer
    if (!m_IsActive) {
        m_SpawnTimer -= dt;
        if (m_SpawnTimer <= 0.0f) {
            m_IsActive = true;
            SetVisible(true);
            SetGridPosition(m_SpawnGridX, m_SpawnGridY);
            
            // Calculate world position based on current map scroll
            float worldX = GameConfig::CAMERA_START_X + (m_GridX * GameConfig::EFFECTIVE_TILE_SIZE) + map->GetWorldOffsetX();
            float worldY = GameConfig::CAMERA_START_Y - (m_GridY * GameConfig::EFFECTIVE_TILE_SIZE) + map->GetWorldOffsetY();
            
            m_Transform.translation = {worldX, worldY + 24.0f}; 
            m_PixelsMoved = 0.0f;
            m_IsMoving = false;
        }
        return glm::vec2(0.0f, 0.0f);
    }

    // 2. If currently moving between tiles, finish that first.
    if (m_IsMoving) {
        glm::vec2 movement = Character::Update(map);
        m_Transform.translation += movement;
        
        // If we just finished a move, check if we are now at the reset point
        if (!m_IsMoving && m_GridY <= m_ResetGridY) {
            m_IsActive = false;
            SetVisible(false);
            m_SpawnTimer = 15.0f;
            return movement;
        }
        return movement;
    }

    // 3. Decide to start a new move
    // Drive forward (Up)
    m_GridY -= 1; 
    m_State = State::MOVING;
    m_IsMoving = true;
    m_PixelsMoved = 0.0f; // Ensure clean start
    m_CurrentDirection = {0.0f, 1.0f}; // Up is +Y visually in your engine setup

    glm::vec2 movement = Character::Update(map);
    m_Transform.translation += movement;
    return movement;
}

void Vehicle::LoadSprites() {
    if (m_TexturePath.empty()) return;
    
    std::vector<std::string> frames = { m_TexturePath };
    auto anim = std::make_shared<Util::Animation>(frames, false, 100, true, 0);
    
    m_AnimUp    = anim;
    m_AnimDown  = anim;
    m_AnimLeft  = anim;
    m_AnimRight = anim;
    
    m_CurrentAnimation = m_AnimUp;
    m_Drawable = m_CurrentAnimation;
}

void Vehicle::UpdateSprite() {
    m_Drawable = m_CurrentAnimation;
    if (m_CurrentAnimation) {
        if (m_State == State::MOVING) m_CurrentAnimation->Play();
        else m_CurrentAnimation->Pause();
    }
}
