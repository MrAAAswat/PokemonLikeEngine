#pragma once
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include <memory>
#include <glm/glm.hpp>

class PokeballAnimator {
public:
    enum class CatchState { IDLE, THROWING, SHAKING, RESULT };

    PokeballAnimator(std::shared_ptr<Util::Renderer> renderer);

    // Call this to kick off the animation
    void StartCatch(glm::vec2 startPos, glm::vec2 targetPos, bool willSucceed, std::shared_ptr<Util::GameObject> targetSprite);
    
    // Call this every frame in BattleUI's Update(). 
    // Returns true if the entire sequence is completely finished.
    bool Update(); 
    bool IsFinished() const { return m_State == CatchState::IDLE; }
    bool CatchSucceeded() const { return m_WillSucceed; }
    

private:
    CatchState m_State = CatchState::IDLE;
    std::shared_ptr<Util::GameObject> m_PokeballSprite;
    std::shared_ptr<Util::GameObject> m_TargetSprite; // The wild Pokemon

    glm::vec2 m_StartPos;
    glm::vec2 m_TargetPos;
    
    bool m_WillSucceed;
    int m_Timer = 0;
    int m_Shakes = 0;
    int m_TargetShakes = 0;
};