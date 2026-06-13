#pragma once
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Renderer.hpp"
#include "Pokemon.hpp"
#include <memory>
#include <vector>

class PokemonMenu {
public:
    enum class Mode { PARTY_LIST, PREVIEW };

    explicit PokemonMenu(std::shared_ptr<Util::Renderer> renderer);
    ~PokemonMenu() = default;

    void Show(const std::vector<std::shared_ptr<Pokemon>>& party);
    void ShowPreview(const Pokemon& pkmn);
    void Hide();
    
    bool Update();                            
    int  GetSelectedIndex() const { return m_CursorIndex; }

    bool IsConfirmed() const { return m_Confirmed; }
    bool IsCancelled() const { return m_Cancelled; }
    bool IsActionSelected() const { return m_ActionSelected; } // NEW: detects 'Z' on a list item
    Mode GetMode() const { return m_Mode; }

private:
    void ClearSlots();
    void BuildSlots(const std::vector<std::shared_ptr<Pokemon>>& party);
    void BuildPreviewSlot(const Pokemon& pkmn);
    void UpdateCursorPosition();

    std::shared_ptr<Util::Renderer> m_Renderer;
    std::shared_ptr<Util::GameObject> m_BoxUI;
    std::shared_ptr<Util::GameObject> m_CursorUI;

    struct Slot {
        std::shared_ptr<Util::GameObject> sprite;
        std::shared_ptr<Util::GameObject> text;
    };
    std::vector<Slot> m_Slots;

    int m_CursorIndex   = 0;
    int m_PartySize     = 0;
    int m_InputCooldown = 0;

    Mode m_Mode       = Mode::PARTY_LIST;
    bool m_Confirmed  = false;
    bool m_Cancelled  = false;
    bool m_ActionSelected = false; // NEW

    // --- NEW Grid Layout Constants ---
    static constexpr float COL_1_X = -320.0f;
    static constexpr float COL_2_X =  320.0f;
    static constexpr float ROW_1_Y =  170.0f;
    static constexpr float ROW_2_Y =    0.0f;
    static constexpr float ROW_3_Y = -170.0f;
    
    static constexpr float SPRITE_OFFSET_X  = -140.0f;
    static constexpr float TEXT_OFFSET_X    =  -60.0f;
    static constexpr float CURSOR_OFFSET_X  = -210.0f;
    static constexpr float SPRITE_SCALE     = 2.0f;
    static constexpr int   INPUT_DELAY      = 10;
};