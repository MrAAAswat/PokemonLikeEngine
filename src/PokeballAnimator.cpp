#include "PokeballAnimator.hpp"
#include "ResourceManager.hpp"
#include "Util/Logger.hpp"



// Constructor

const std::string POKEBALL_RES = std::string(RESOURCE_DIR) + "/items/";

PokeballAnimator::PokeballAnimator(std::shared_ptr<Util::Renderer> renderer) {
    m_PokeballSprite = std::make_shared<Util::GameObject>();
    
    try {
        // I am hardcoding the path here temporarily to bypass any POKEBALL_RES typos.
        // Change "battle" to "Items" or wherever your image actually is!
        std::string safePath = std::string(RESOURCE_DIR) + "/items/Pokeball.png"; 
        
        m_PokeballSprite->SetDrawable(ResourceManager::GetImageStore().Get(safePath));
    } 
    catch (...) {
        // IF IT FAILS, IT PRINTS THIS INSTEAD OF CRASHING!
        LOG_ERROR("==================================================");
        LOG_ERROR("CRASH PREVENTED: Could not find pokeball.png!");
        LOG_ERROR("Check your assets folder spelling and capitalization.");
        LOG_ERROR("==================================================");
    }

    //m_PokeballSprite->m_Transform.scale = {3.5f, 3.5f}; 
    m_PokeballSprite->SetZIndex(50); 
    
    renderer->AddChild(m_PokeballSprite);
    m_PokeballSprite->SetVisible(false);
}

// Start the animation
void PokeballAnimator::StartCatch(glm::vec2 startPos, glm::vec2 targetPos, bool willSucceed, std::shared_ptr<Util::GameObject> targetSprite) {
    m_StartPos = startPos;
    m_TargetPos = targetPos;
    m_WillSucceed = willSucceed;
    m_TargetSprite = targetSprite;
    
    m_PokeballSprite->SetVisible(true);
    m_PokeballSprite->m_Transform.translation = m_StartPos;
    
    m_Timer = 0;
    m_Shakes = 0;
    m_TargetShakes = m_WillSucceed ? 3 : (rand() % 4);
    
    m_State = CatchState::THROWING;
}

// Update loop (called every frame)
bool PokeballAnimator::Update() {
    if (m_State == CatchState::IDLE) return true; // Animation is done or hasn't started

    m_Timer++;

    if (m_State == CatchState::THROWING) {
        float duration = 45.0f; // How many frames the throw takes
        float progress = std::clamp(m_Timer / duration, 0.0f, 1.0f);
        
        // Linear X movement
        float currentX = m_StartPos.x + (m_TargetPos.x - m_StartPos.x) * progress;
        
        // Y movement with an arc (Sine wave makes it go up, then down)
        float arcHeight = 200.0f; // How high the ball goes
        float currentY = m_StartPos.y + (m_TargetPos.y - m_StartPos.y) * progress 
                         + (std::sin(progress * 3.14159f) * arcHeight);

        // Apply translations and spin the ball
        m_PokeballSprite->m_Transform.translation = {currentX, currentY};
        m_PokeballSprite->m_Transform.rotation = progress * -10.0f; 

        // Transition: When the ball hits the target
        if (m_Timer >= duration) {
            // "Absorb" the Pokemon if the reference is valid
            if (m_TargetSprite != nullptr) {
                m_TargetSprite->SetVisible(false); 
            }
            
            // Reset rotation to normal for the shake phase
            m_PokeballSprite->m_Transform.rotation = 0.0f; 
            
            m_Timer = 0;
            m_State = CatchState::SHAKING;
        }
    }
    else if (m_State == CatchState::SHAKING) {
        // Shake logic: Sit still for 40 frames, then quickly bob left/right, repeat.
        int cycle = m_Timer % 60; 

        if (cycle == 40) {
            m_PokeballSprite->m_Transform.rotation = 0.5f; // Tilt right
            m_PokeballSprite->m_Transform.translation.x += 10.0f; 
        } 
        else if (cycle == 45) {
            m_PokeballSprite->m_Transform.rotation = -0.5f; // Tilt left
            m_PokeballSprite->m_Transform.translation.x -= 20.0f; 
        }
        else if (cycle == 50) {
            m_PokeballSprite->m_Transform.rotation = 0.0f; // Center
            m_PokeballSprite->m_Transform.translation.x += 10.0f; 
        }
        else if (cycle == 59) {
            m_Shakes++;
            
            // Check if we've completed the required amount of shakes
            if (m_Shakes >= m_TargetShakes) {
                m_Timer = 0;
                m_State = CatchState::RESULT;
            }
        }
    }
    else if (m_State == CatchState::RESULT) {
        // Wait a brief moment for dramatic effect before returning to the UI
        if (m_Timer > 20) {
            m_State = CatchState::IDLE;
            m_PokeballSprite->SetVisible(false);
            return true; // Signals to BattleUI that we are completely finished!
        }
    }


    return false; // Still animating
}
    