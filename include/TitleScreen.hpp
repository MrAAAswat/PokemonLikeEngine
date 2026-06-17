#pragma once

#include <memory>
#include <vector>
#include <string>
#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"

// ============================================================
//  TitleScreen — shown once at startup for save-select / new game.
//
//  Does NOT include SaveSystem.hpp or Item.hpp.
//  It reads save-slot JSON files directly via nlohmann/json,
//  so it has no dependency on GameState or InventoryData.
//
//  Usage (in App):
//    m_TitleScreen = std::make_shared<TitleScreen>(m_Renderer);
//    m_TitleScreen->Load();
//    m_CurrentState = State::TITLE;
//
//    // Inside State::TITLE branch of Update():
//    m_TitleScreen->Update();
//    if (m_TitleScreen->IsConfirmed()) {
//        int slot = m_TitleScreen->GetSelectedSlot(); // -1 = New Game
//        m_TitleScreen->Hide();
//        InitGameLoad(slot);
//        m_CurrentState = State::UPDATE;
//    }
// ============================================================

class TitleScreen : public Util::GameObject {
public:
    explicit TitleScreen(std::shared_ptr<Util::Renderer> renderer);

    void Load();    // scan save files on disk, build card UI
    void Update();  // call every frame while State == TITLE
    void Hide();    // hide all scene objects when leaving state

    bool IsConfirmed()         const { return m_Confirmed; }
    int  GetSelectedSlot()     const { return m_SelectedSlot; }  // -1 = New Game
    bool IsDeletionRequested() const { return m_DeletionConfirmed; }

private:
    // Mirrors the constants in SaveSystem without including it
    static constexpr int MAX_SLOTS = 3;
    static const     std::string SLOT_PREFIX;   // "save_slot"

    struct SlotInfo {
        bool        exists      = false;
        std::string mapName;
        int         money       = 0;
        int         partySize   = 0;
        std::string leadPokemon;
        int         leadLevel   = 1;
        std::string savePath;
    };

    enum class SubState { ANIMATING_IN, IDLE, CONFIRM_DELETE, CONFIRMED };

    void ScanSlots();
    void BuildUI();
    void RebuildCards();
    void DeleteSlot(int index);
    void HandleInput();
    void UpdateAnimations(float dt);
    std::string PrettyMapName(const std::string& path) const;

    std::shared_ptr<Util::Renderer> m_Renderer;

    std::vector<SlotInfo> m_Slots;
    int m_SelectedIndex = 0;
    int m_TotalCards    = 0;

    SubState m_SubState          = SubState::ANIMATING_IN;
    bool     m_Confirmed         = false;
    bool     m_DeletionConfirmed = false;
    int      m_SelectedSlot      = -1;

    float m_CardOffsetY = 350.0f;

    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<Util::GameObject> m_TitleObj;
    std::shared_ptr<Util::Text>       m_TitleText;
    std::shared_ptr<Util::GameObject> m_HintObj;
    std::shared_ptr<Util::Text>       m_HintText;
    std::shared_ptr<Util::GameObject> m_DeletePromptObj;
    std::shared_ptr<Util::Text>       m_DeletePromptText;
    std::shared_ptr<Util::GameObject> m_PlayerCursorObj;

    struct CardUI {
        std::shared_ptr<Util::GameObject> panel;
        std::shared_ptr<Util::GameObject> nameObj;
        std::shared_ptr<Util::Text>       nameText;
        std::shared_ptr<Util::GameObject> detailObj;
        std::shared_ptr<Util::Text>       detailText;
        std::shared_ptr<Util::GameObject> highlightObj;
    };
    std::vector<CardUI> m_Cards;
};