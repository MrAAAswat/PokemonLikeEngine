#ifndef PLAYER_HPP
#define PLAYER_HPP
#include "Character.hpp"
#include "Item.hpp"

class Player : public Character {
public:
    Player(float x, float y);
    glm::vec2 Update(std::shared_ptr<Map> map) override;    // We override the base update to add our input logic
    bool HasHitDoor()           const { return m_HitDoor; }
    void ClearDoorFlag()              { m_HitDoor = false; }
    void HandleInput(std::shared_ptr<Map> map);
    bool JustFinishedMoving()   const { return m_JustFinishedMoving; }
    bool HasWildEncounter()     const { return m_WildEncounterTriggered; }
    void ClearEncounterFlag()         { m_WildEncounterTriggered = false; }
    void SwapPokemon(int index1, int index2) {
        if (index1 >= 0 && index1 < static_cast<int>(m_PokemonParty.size()) && 
            index2 >= 0 && index2 < static_cast<int>(m_PokemonParty.size())) {
            std::swap(m_PokemonParty[index1], m_PokemonParty[index2]);
        }
    }
    void SetLocked(bool locked) { m_Locked = locked; }
    bool IsLocked() const { return m_Locked; }
    float GetVisualOffsetY() const { return m_VisualOffsetY; }
    void SnapToGrid();

protected:
    void LoadSprites() override; // Loads the Red character sprites

private:
    float m_VisualOffsetY = -12.0f;
    void UpdateSprite() override;
    bool m_HitDoor                  = false;
    bool m_JustFinishedMoving       = false;
    bool m_WildEncounterTriggered   = false;
    bool m_IsRunning = false;
    bool m_Locked = false;
    
    std::shared_ptr<Util::Animation> m_AnimRunDown;
    std::shared_ptr<Util::Animation> m_AnimRunUp;
    std::shared_ptr<Util::Animation> m_AnimRunLeft;
    std::shared_ptr<Util::Animation> m_AnimRunRight;
};

#endif