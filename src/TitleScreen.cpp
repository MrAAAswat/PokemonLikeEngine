#include "TitleScreen.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Color.hpp"

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <cmath>
#include <cctype>

using json = nlohmann::json;
namespace fs = std::filesystem;

// ── static member definition ─────────────────────────────────────────────────
const std::string TitleScreen::SLOT_PREFIX = "save_slot";

// ── layout constants ──────────────────────────────────────────────────────────
static constexpr float CARD_SPACING = 340.0f;
static constexpr float CARD_Y       =  -20.0f;
static constexpr float ANIM_SPEED   = 900.0f;
static constexpr float TITLE_Y      =  240.0f;
static constexpr float HINT_Y       = -260.0f;

static constexpr float Z_BG     = 0.0f;
static constexpr float Z_CARD   = 2.0f;
static constexpr float Z_GLOW   = 3.0f;
static constexpr float Z_TEXT   = 4.0f;
static constexpr float Z_PROMPT = 8.0f;
static constexpr float Z_PLAYER = 5.0f; 
static constexpr float PLAYER_Y = -150.0f; 

// ── ctor ──────────────────────────────────────────────────────────────────────
TitleScreen::TitleScreen(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(std::move(renderer))
{
    m_Slots.resize(MAX_SLOTS);
    SetVisible(true); 
    SetZIndex(100.0f);
}

// ── Load ─────────────────────────────────────────────────────────────────────
void TitleScreen::Load() {
    ScanSlots();
    m_TotalCards  = static_cast<int>(m_Slots.size()) + 1;  // +1 for New Game
    m_CardOffsetY = 350.0f;
    m_SubState    = SubState::ANIMATING_IN;
    BuildUI();
}

// ── ScanSlots — reads raw JSON, no SaveSystem dependency ─────────────────────
void TitleScreen::ScanSlots() {
    for (int i = 0; i < MAX_SLOTS; ++i) {
        SlotInfo& slot = m_Slots[i];
        slot = {};
        slot.savePath = SLOT_PREFIX + "_" + std::to_string(i) + ".json";

        if (!fs::exists(slot.savePath)) continue;
        std::ifstream f(slot.savePath);
        if (!f.is_open()) continue;

        json j;
        try { f >> j; } catch (...) { continue; }

        slot.exists  = true;
        slot.money   = j.value("money", 0);
        slot.mapName = PrettyMapName(j.value("mapPath", ""));

        if (j.contains("party") && j["party"].is_array() && !j["party"].empty()) {
            slot.partySize   = static_cast<int>(j["party"].size());
            const auto& lead = j["party"][0];
            slot.leadPokemon = lead.value("name",  "?");
            slot.leadLevel   = lead.value("level", 1);
        }
    }
}

// ── BuildUI ──────────────────────────────────────────────────────────────────
void TitleScreen::BuildUI() {
    // Reuse the existing dialogue-box texture as a dark background panel
    auto bgImg = ResourceManager::GetImageStore().Get(
        RESOURCE_DIR "/UI/BWTextBox3.png");

    auto playerImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/player/character_000.png"); // Swap with your actual sprite path
    m_PlayerCursorObj = std::make_shared<Util::GameObject>();
    m_PlayerCursorObj->SetDrawable(playerImg);
    m_PlayerCursorObj->m_Transform.scale = {3.0f, 3.0f}; // Integer scaling for pixel art crispness
    m_PlayerCursorObj->SetZIndex(Z_PLAYER);
    m_PlayerCursorObj->SetVisible(true);
    m_Renderer->AddChild(m_PlayerCursorObj);

    m_Background = std::make_shared<Util::GameObject>();
    m_Background->SetDrawable(bgImg);
    m_Background->m_Transform.scale       = {10.0f, 10.0f};
    m_Background->m_Transform.translation = {0.0f, 0.0f};
    m_Background->SetZIndex(Z_BG);
    m_Background->SetVisible(true);
    m_Renderer->AddChild(m_Background);

    m_TitleText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 60,
        "POKEMON RIPOFF", Util::Color(255, 220, 40));
    m_TitleObj = std::make_shared<Util::GameObject>();
    m_TitleObj->SetDrawable(m_TitleText);
    m_TitleObj->SetZIndex(Z_TEXT);
    m_TitleObj->m_Transform.translation = {0.0f, TITLE_Y};
    m_TitleObj->SetVisible(true);
    m_Renderer->AddChild(m_TitleObj);

    m_HintText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 20,
        "Arrow Keys: Navigate    Z: Select    X: Delete Save",
        Util::Color(200, 200, 200));
    m_HintObj = std::make_shared<Util::GameObject>();
    m_HintObj->SetDrawable(m_HintText);
    m_HintObj->SetZIndex(Z_TEXT);
    m_HintObj->m_Transform.translation = {0.0f, HINT_Y};
    m_HintObj->SetVisible(true);
    m_Renderer->AddChild(m_HintObj);

    m_DeletePromptText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 26,
        "Delete this save?   Z: Yes    X: No",
        Util::Color(255, 70, 70));
    m_DeletePromptObj = std::make_shared<Util::GameObject>();
    m_DeletePromptObj->SetDrawable(m_DeletePromptText);
    m_DeletePromptObj->SetZIndex(Z_PROMPT);
    m_DeletePromptObj->m_Transform.translation = {0.0f, HINT_Y - 50.0f};
    m_DeletePromptObj->SetVisible(false);
    m_Renderer->AddChild(m_DeletePromptObj);

    RebuildCards();
}

// ── RebuildCards ─────────────────────────────────────────────────────────────
void TitleScreen::RebuildCards() {
    for (auto& c : m_Cards) {
        m_Renderer->RemoveChild(c.panel);
        m_Renderer->RemoveChild(c.nameObj);
        m_Renderer->RemoveChild(c.detailObj);
        m_Renderer->RemoveChild(c.highlightObj);
    }
    m_Cards.clear();

    m_TotalCards = static_cast<int>(m_Slots.size()) + 1;

    float totalWidth = CARD_SPACING * (m_TotalCards - 1);
    float startX     = -totalWidth / 2.0f;

    auto panelImg = ResourceManager::GetImageStore().Get(
        RESOURCE_DIR "/UI/BWTextBox3.png");

    for (int i = 0; i < m_TotalCards; ++i) {
        CardUI card;
        float cx = startX + i * CARD_SPACING;
        float cy = CARD_Y + m_CardOffsetY;

        card.panel = std::make_shared<Util::GameObject>();
        card.panel->SetDrawable(panelImg);
        card.panel->m_Transform.scale       = {1.8f, 1.0f};
        card.panel->m_Transform.translation = {cx, cy};
        card.panel->SetZIndex(Z_CARD);
        card.panel->SetVisible(true);
        m_Renderer->AddChild(card.panel);

        card.highlightObj = std::make_shared<Util::GameObject>();
        card.highlightObj->SetDrawable(panelImg);
        card.highlightObj->m_Transform.scale       = {1.88f, 1.08f};
        card.highlightObj->m_Transform.translation = {cx, cy};
        card.highlightObj->SetZIndex(Z_GLOW);
        card.highlightObj->SetVisible(false);
        m_Renderer->AddChild(card.highlightObj);

        bool isNewGame = (i == static_cast<int>(m_Slots.size()));
        std::string titleStr, detailStr;

        if (isNewGame) {
            titleStr  = "New Game";
            detailStr = "Start a fresh adventure!";
        } else {
            const SlotInfo& s = m_Slots[i];
            titleStr = "Save Slot " + std::to_string(i + 1);
            if (s.exists) {
                detailStr  = s.mapName + "\n";
                detailStr += s.leadPokemon + "  Lv." + std::to_string(s.leadLevel) + "\n";
                detailStr += "Party: " + std::to_string(s.partySize)
                           + "   $" + std::to_string(s.money);
            } else {
                detailStr = "(empty)";
            }
        }

        card.nameText = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 24,
            titleStr, Util::Color(40, 40, 40));
        card.nameObj = std::make_shared<Util::GameObject>();
        card.nameObj->SetDrawable(card.nameText);
        card.nameObj->m_Transform.translation = {cx - 90.0f, cy + 45.0f};
        card.nameObj->SetZIndex(Z_TEXT);
        card.nameObj->SetVisible(true);
        m_Renderer->AddChild(card.nameObj);

        card.detailText = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 17,
            detailStr, Util::Color(60, 60, 60));
        card.detailObj = std::make_shared<Util::GameObject>();
        card.detailObj->SetDrawable(card.detailText);
        card.detailObj->m_Transform.translation = {cx - 90.0f, cy + 5.0f};
        card.detailObj->SetZIndex(Z_TEXT);
        card.detailObj->SetVisible(true);
        m_Renderer->AddChild(card.detailObj);

        m_Cards.push_back(std::move(card));
    }

    if (m_SelectedIndex < static_cast<int>(m_Cards.size()))
        m_Cards[m_SelectedIndex].highlightObj->SetVisible(true);
}

// ── Update ───────────────────────────────────────────────────────────────────
void TitleScreen::Update() {
    m_Confirmed         = false;
    m_DeletionConfirmed = false;
    constexpr float dt  = 1.0f / 60.0f;
    UpdateAnimations(dt);
    HandleInput();
}

// ── UpdateAnimations ─────────────────────────────────────────────────────────
void TitleScreen::UpdateAnimations(float dt) {
    if (m_SubState != SubState::ANIMATING_IN) return;

    m_CardOffsetY -= ANIM_SPEED * dt;
    if (m_CardOffsetY <= 0.0f) {
        m_CardOffsetY = 0.0f;
        m_SubState = SubState::IDLE;
    }

    float totalWidth = CARD_SPACING * (m_TotalCards - 1);
    float startX     = -totalWidth / 2.0f;
    for (int i = 0; i < static_cast<int>(m_Cards.size()); ++i) {
        float cx = startX + i * CARD_SPACING;
        float cy = CARD_Y + m_CardOffsetY;
        m_Cards[i].panel->m_Transform.translation        = {cx, cy};
        m_Cards[i].highlightObj->m_Transform.translation = {cx, cy};
        m_Cards[i].nameObj->m_Transform.translation      = {cx - 90.0f, cy + 45.0f};
        m_Cards[i].detailObj->m_Transform.translation    = {cx - 90.0f, cy + 5.0f};
    }
    float currentCardX = startX + m_SelectedIndex * CARD_SPACING;
    m_PlayerCursorObj->m_Transform.translation = { currentCardX, PLAYER_Y + m_CardOffsetY };
}

// ── HandleInput ──────────────────────────────────────────────────────────────
void TitleScreen::HandleInput() {
    if (m_SubState == SubState::ANIMATING_IN) return;

    if (m_SubState == SubState::CONFIRM_DELETE) {
        if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
            DeleteSlot(m_SelectedIndex);
            m_DeletePromptObj->SetVisible(false);
            m_SubState = SubState::IDLE;
            RebuildCards();
        } else if (Util::Input::IsKeyDown(Util::Keycode::X) ||
                   Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
            m_DeletePromptObj->SetVisible(false);
            m_SubState = SubState::IDLE;
        }
        return;
    }

    bool left  = Util::Input::IsKeyDown(Util::Keycode::LEFT)  ||
                 Util::Input::IsKeyDown(Util::Keycode::A);
    bool right = Util::Input::IsKeyDown(Util::Keycode::RIGHT) ||
                 Util::Input::IsKeyDown(Util::Keycode::D);

    int prev = m_SelectedIndex;
    if (left  && m_SelectedIndex > 0)               --m_SelectedIndex;
    if (right && m_SelectedIndex < m_TotalCards - 1) ++m_SelectedIndex;
    if (prev != m_SelectedIndex) {
        m_Cards[prev].highlightObj->SetVisible(false);
        m_Cards[m_SelectedIndex].highlightObj->SetVisible(true);
    }
    float totalWidth = CARD_SPACING * (m_TotalCards - 1);
    float startX     = -totalWidth / 2.0f;
    m_PlayerCursorObj->m_Transform.translation = { startX + m_SelectedIndex * CARD_SPACING, PLAYER_Y };

    if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
        bool isNewGame = (m_SelectedIndex == static_cast<int>(m_Slots.size()));
        m_SelectedSlot = isNewGame ? -1 : m_SelectedIndex;
        m_Confirmed    = true;
        m_SubState     = SubState::CONFIRMED;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::X)) {
        bool isNewGame = (m_SelectedIndex == static_cast<int>(m_Slots.size()));
        if (!isNewGame && m_Slots[m_SelectedIndex].exists) {
            m_DeletePromptObj->SetVisible(true);
            m_SubState = SubState::CONFIRM_DELETE;
        }
    }
}

// ── DeleteSlot ───────────────────────────────────────────────────────────────
void TitleScreen::DeleteSlot(int index) {
    if (index < 0 || index >= static_cast<int>(m_Slots.size())) return;
    const std::string& p = m_Slots[index].savePath;
    if (fs::exists(p)) {
        fs::remove(p);
        LOG_INFO("Deleted save slot {}: {}", index, p);
    }
    std::string savedPath = m_Slots[index].savePath;
    m_Slots[index] = {};
    m_Slots[index].savePath = savedPath;
    m_DeletionConfirmed = true;
}

// ── Hide ─────────────────────────────────────────────────────────────────────
void TitleScreen::Hide() {
    if (m_Background)      m_Background->SetVisible(false);
    if (m_TitleObj)        m_TitleObj->SetVisible(false);
    if (m_HintObj)         m_HintObj->SetVisible(false);
    if (m_DeletePromptObj) m_DeletePromptObj->SetVisible(false);
    if (m_PlayerCursorObj) m_PlayerCursorObj->SetVisible(false);
    for (auto& c : m_Cards) {
        c.panel->SetVisible(false);
        c.nameObj->SetVisible(false);
        c.detailObj->SetVisible(false);
        c.highlightObj->SetVisible(false);
    }
}

// ── PrettyMapName ─────────────────────────────────────────────────────────────
std::string TitleScreen::PrettyMapName(const std::string& path) const {
    if (path.empty()) return "Unknown";
    fs::path p(path);
    std::string stem = p.stem().string();
    std::string pretty;
    for (size_t i = 0; i < stem.size(); ++i) {
        if (i > 0 &&
            std::isupper(static_cast<unsigned char>(stem[i])) &&
            std::islower(static_cast<unsigned char>(stem[i - 1])))
            pretty += ' ';
        pretty += stem[i];
    }
    return pretty;
}