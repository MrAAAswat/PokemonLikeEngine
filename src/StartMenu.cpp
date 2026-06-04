#include "StartMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

StartMenu::StartMenu(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(renderer)
{
    // Background box
    m_BoxUI = std::make_shared<Util::GameObject>();
    auto boxImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/MenuBoxUI2.png");
    m_BoxUI->SetDrawable(boxImg);
    m_BoxUI->SetZIndex(90.0f);
    m_BoxUI->m_Transform.scale = {BOX_SCALE_X, BOX_SCALE_X};
    m_BoxUI->m_Transform.translation = {BOX_POS_X, BOX_POS_Y};
    m_Renderer->AddChild(m_BoxUI);

    // Player info texts (invisible by default)
    m_MoneyText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/micross.ttf", 24,
        "Money: $0", Util::Color(50, 50, 50));
    m_MoneyTextObj = std::make_shared<Util::GameObject>();
    m_MoneyTextObj->SetDrawable(m_MoneyText);
    m_MoneyTextObj->SetZIndex(91.0f);
    m_MoneyTextObj->SetVisible(false);
    m_Renderer->AddChild(m_MoneyTextObj);

    m_PartyText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/micross.ttf", 24,
        "Party: 0/6", Util::Color(50, 50, 50));
    m_PartyTextObj = std::make_shared<Util::GameObject>();
    m_PartyTextObj->SetDrawable(m_PartyText);
    m_PartyTextObj->SetZIndex(91.0f);
    m_PartyTextObj->SetVisible(false);
    m_Renderer->AddChild(m_PartyTextObj);

    // Build menu options (text + icons)
    BuildMenuGraphics();

    // Cursor
    m_CursorUI = std::make_shared<Util::GameObject>();
    auto cursorImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/Cursor.png");
    m_CursorUI->SetDrawable(cursorImg);
    m_CursorUI->SetZIndex(92.0f);
    m_CursorUI->m_Transform.scale = {2.5f, 2.5f};
    m_Renderer->AddChild(m_CursorUI);

    SetVisible(false);
}

// -------------------------------------------------------------------
// Create / recreate the menu options (icons + text)
// -------------------------------------------------------------------
void StartMenu::BuildMenuGraphics() {
    // Remove old items
    for (auto& go : m_ItemTexts)   m_Renderer->RemoveChild(go);
    for (auto& go : m_ItemSprites) m_Renderer->RemoveChild(go);
    m_ItemTexts.clear();
    m_ItemSprites.clear();

    const float spriteX   = TEXT_LEFT_MARGIN - 30.0f;   // icon sits to the left of text
    const float textBaseX = TEXT_LEFT_MARGIN;           // text starts here

    for (size_t i = 0; i < m_Items.size(); ++i) {
        const auto& item = m_Items[i];
        float y = OPTION_START_Y - static_cast<float>(i) * LINE_SPACING;

        // Icon (if a path is given)
        if (!item.iconPath.empty()) {
            auto spriteObj = std::make_shared<Util::GameObject>();
            auto img = ResourceManager::GetImageStore().Get(item.iconPath);
            if (img) {
                spriteObj->SetDrawable(img);
                spriteObj->m_Transform.scale = {SPRITE_SCALE, SPRITE_SCALE};
            }
            spriteObj->SetZIndex(91.0f);
            spriteObj->m_Transform.translation = {spriteX, y};
            m_Renderer->AddChild(spriteObj);
            m_ItemSprites.push_back(spriteObj);
        }

        // Text
        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/micross.ttf", 32,
            item.label, Util::Color(50, 50, 50));
        auto go = std::make_shared<Util::GameObject>();
        go->SetDrawable(txt);
        go->SetZIndex(91.0f);
        float textW = txt->GetSize().x;
        go->m_Transform.translation = { textBaseX + textW / 2.0f, y };
        m_Renderer->AddChild(go);
        m_ItemTexts.push_back(go);
    }
}

// -------------------------------------------------------------------
// Update player info text positions & content
// -------------------------------------------------------------------
void StartMenu::BuildInfoDisplay() {
    // Position the info lines at the top of the menu
    float yMoney = INFO_START_Y;
    float yParty = yMoney - INFO_SPACING;

    m_MoneyTextObj->m_Transform.translation = {TEXT_LEFT_MARGIN + 100.0f, yMoney};
    m_PartyTextObj->m_Transform.translation = {TEXT_LEFT_MARGIN + 100.0f, yParty};

    m_MoneyTextObj->SetVisible(true);
    m_PartyTextObj->SetVisible(true);
}

void StartMenu::SetPlayerInfo(int money, int partySize) {
    m_MoneyText->SetText("Money: $" + std::to_string(money));
    m_PartyText->SetText("Party: " + std::to_string(partySize) + "/6");
    BuildInfoDisplay();  // reposition them when shown
}

// -------------------------------------------------------------------
// Show / hide
// -------------------------------------------------------------------
void StartMenu::SetVisible(bool visible) {
    m_BoxUI->SetVisible(visible);
    for (auto& go : m_ItemTexts)   go->SetVisible(visible);
    for (auto& go : m_ItemSprites) go->SetVisible(visible);
    m_CursorUI->SetVisible(visible);
    m_MoneyTextObj->SetVisible(visible);
    m_PartyTextObj->SetVisible(visible);

    if (visible) {
        m_CursorIndex = 0;
        UpdateCursorPosition();
    }
}

// -------------------------------------------------------------------
// Input handling
// -------------------------------------------------------------------
StartMenu::Option StartMenu::Update() {
    return ProcessInput();
}

StartMenu::Option StartMenu::ProcessInput() {
    if (m_InputTimer > 0) {
        --m_InputTimer;
        return Option::NONE;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::UP) || Util::Input::IsKeyDown(Util::Keycode::W)) {
        m_CursorIndex = (m_CursorIndex - 1 + m_Items.size()) % m_Items.size();
        UpdateCursorPosition();
        m_InputTimer = INPUT_COOLDOWN;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::DOWN) || Util::Input::IsKeyDown(Util::Keycode::S)) {
        m_CursorIndex = (m_CursorIndex + 1) % m_Items.size();
        UpdateCursorPosition();
        m_InputTimer = INPUT_COOLDOWN;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::RETURN) || Util::Input::IsKeyDown(Util::Keycode::Z)) {
        return m_Items[m_CursorIndex].value;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE) || Util::Input::IsKeyDown(Util::Keycode::X)) {
        return Option::CANCEL;
    }

    return Option::NONE;
}

// -------------------------------------------------------------------
// Cursor movement
// -------------------------------------------------------------------
void StartMenu::UpdateCursorPosition() {
    if (m_CursorIndex < 0 || m_CursorIndex >= static_cast<int>(m_ItemTexts.size()))
        return;

    float textY = m_ItemTexts[m_CursorIndex]->m_Transform.translation.y;
    m_CursorUI->m_Transform.translation = {
        TEXT_LEFT_MARGIN + CURSOR_OFFSET_X,
        textY + 5.0f
    };
}

// -------------------------------------------------------------------
// Cleanup (if needed, e.g. when rebuilding)
// -------------------------------------------------------------------
void StartMenu::ClearAll() {
    for (auto& go : m_ItemTexts)   m_Renderer->RemoveChild(go);
    for (auto& go : m_ItemSprites) m_Renderer->RemoveChild(go);
    m_ItemTexts.clear();
    m_ItemSprites.clear();
}