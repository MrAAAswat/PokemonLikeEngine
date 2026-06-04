#include "ShopMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

ShopMenu::ShopMenu(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(renderer)
{
    // Box background
    m_BoxUI = std::make_shared<Util::GameObject>();
    auto boxImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/InventoryBoxUI.png");
    m_BoxUI->SetDrawable(boxImg);
    m_BoxUI->SetZIndex(90.0f);
    m_BoxUI->m_Transform.translation = {0.0f, 0.0f};
    m_BoxUI->SetVisible(false);
    m_Renderer->AddChild(m_BoxUI);

    // Cursor
    m_CursorUI = std::make_shared<Util::GameObject>();
    auto cursorImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/Cursor.png");
    m_CursorUI->SetDrawable(cursorImg);
    m_CursorUI->SetZIndex(92.0f);
    m_CursorUI->SetVisible(false);
    m_Renderer->AddChild(m_CursorUI);

    // Money display
    m_MoneyText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 30,
        "Money: $0", Util::Color(50, 50, 50));
    m_MoneyTextObj = std::make_shared<Util::GameObject>();
    m_MoneyTextObj->SetDrawable(m_MoneyText);
    m_MoneyTextObj->SetZIndex(93.0f);
    m_MoneyTextObj->SetVisible(false);
    m_MoneyTextObj->m_Transform.translation = {TEXT_LEFT_MARGIN + 300.0f, START_Y + 60.0f};
    m_Renderer->AddChild(m_MoneyTextObj);
}

// -------------------------------------------------------------------
// Load items for BUY mode
// -------------------------------------------------------------------
void ShopMenu::LoadBuyItems(const std::vector<ShopItem>& shopItems,
                            const std::function<const ItemProperties&(const std::string&)>& getProps) {
    m_DisplayList.clear();
    for (const auto& entry : shopItems) {
        const auto& props = getProps(entry.itemName);
        if (props.buyPrice <= 0) continue;

        std::string line;
        if (entry.quantity == -1)
            line = props.name + "   $" + std::to_string(props.buyPrice);
        else
            line = props.name + "   $" + std::to_string(props.buyPrice) + "  x" + std::to_string(entry.quantity);

        // Use shopTexturePath (separate from overworld texture)
        m_DisplayList.push_back({line, entry.itemName, props.shopTexturePath});
    }
}

// -------------------------------------------------------------------
// Load items for SELL mode
// -------------------------------------------------------------------
void ShopMenu::LoadSellItems(const std::map<std::string, int>& playerInventory,
                             const std::function<const ItemProperties&(const std::string&)>& getProps) {
    m_DisplayList.clear();
    for (const auto& [name, qty] : playerInventory) {
        const auto& props = getProps(name);
        if (props.sellPrice <= 0) continue;
        std::string line = props.name + "   $" + std::to_string(props.sellPrice) + "  x" + std::to_string(qty);
        m_DisplayList.push_back({line, name, props.shopTexturePath});
    }
}

// -------------------------------------------------------------------
// Show the menu (with money)
// -------------------------------------------------------------------
void ShopMenu::Show(Mode mode, int playerMoney) {
    m_Mode = mode;
    m_CursorIndex = 0;
    m_InputTimer = 0;
    m_SelectedItemName.clear();
    m_LastResult = Result::NONE;
    RebuildDisplay();
    SetPlayerMoney(playerMoney);
    UpdateMoneyDisplay();
    m_BoxUI->SetVisible(true);
    m_CursorUI->SetVisible(!m_DisplayList.empty());
    UpdateCursorPosition();
}

// -------------------------------------------------------------------
// Rebuild all text and sprite objects
// -------------------------------------------------------------------
void ShopMenu::RebuildDisplay() {
    ClearItems();

    constexpr float SPRITE_SCALE = 2.0f;              // fixed scale for icons
    constexpr float SPRITE_X_OFFSET = TEXT_LEFT_MARGIN + 10.0f;  // left edge of sprite
    constexpr float TEXT_OFFSET_X   = TEXT_LEFT_MARGIN + 50.0f;  // start of text (to the right of sprite)
    constexpr float VERT_ADJUST     = 4.0f;           // fine‑tune vertical alignment

    for (size_t i = 0; i < m_DisplayList.size(); ++i) {
        const auto& line = m_DisplayList[i];
        float y = START_Y - static_cast<float>(i) * LINE_SPACING;

        // --- Sprite ---
        auto spriteObj = std::make_shared<Util::GameObject>();
        if (!line.shopTexturePath.empty()) {
            auto img = ResourceManager::GetImageStore().Get(line.shopTexturePath);
            if (img) {
                spriteObj->SetDrawable(img);
                spriteObj->m_Transform.scale = {SPRITE_SCALE, SPRITE_SCALE};
            }
        }
        spriteObj->SetZIndex(91.0f);
        spriteObj->m_Transform.translation = {SPRITE_X_OFFSET, y + VERT_ADJUST};
        m_Renderer->AddChild(spriteObj);
        m_ItemSprites.push_back(spriteObj);

        // --- Text ---
        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 22,
            line.text, Util::Color(50, 50, 50));
        auto txtObj = std::make_shared<Util::GameObject>();
        txtObj->SetDrawable(txt);
        txtObj->SetZIndex(91.0f);
        float textW = txt->GetSize().x;
        // Left‑align the text by offsetting half its width
        txtObj->m_Transform.translation = { TEXT_OFFSET_X + textW / 2.0f, y };
        m_Renderer->AddChild(txtObj);
        m_ItemTexts.push_back(txtObj);
    }
}

// -------------------------------------------------------------------
// Move cursor to the selected item
// -------------------------------------------------------------------
void ShopMenu::UpdateCursorPosition() {
    if (static_cast<size_t>(m_CursorIndex) < m_ItemTexts.size()) {
        float y = m_ItemTexts[m_CursorIndex]->m_Transform.translation.y;
        m_CursorUI->m_Transform.translation = {
            TEXT_LEFT_MARGIN + CURSOR_OFFSET_X,
            y + 5.0f
        };
    }
}

// -------------------------------------------------------------------
// Input handling
// -------------------------------------------------------------------
ShopMenu::Result ShopMenu::Update() {
    // Cooldown (prevents flying cursor)
    if (m_InputTimer > 0) {
        --m_InputTimer;
        if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE))
            return Result::BACK;
        return Result::NONE;
    }

    if (m_DisplayList.empty()) return Result::NONE;

    bool moved = false;
    if (Util::Input::IsKeyDown(Util::Keycode::UP) || Util::Input::IsKeyDown(Util::Keycode::W)) {
        m_CursorIndex = (m_CursorIndex - 1 + m_DisplayList.size()) % m_DisplayList.size();
        moved = true;
    } else if (Util::Input::IsKeyDown(Util::Keycode::DOWN) || Util::Input::IsKeyDown(Util::Keycode::S)) {
        m_CursorIndex = (m_CursorIndex + 1) % m_DisplayList.size();
        moved = true;
    }

    if (moved) {
        UpdateCursorPosition();
        m_InputTimer = INPUT_DELAY;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::Z) || Util::Input::IsKeyDown(Util::Keycode::RETURN)) {
        m_SelectedItemName = m_DisplayList[m_CursorIndex].itemName;
        return (m_Mode == Mode::BUY) ? Result::BUY_ITEM : Result::SELL_ITEM;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE))
        return Result::BACK;

    return Result::NONE;
}

// -------------------------------------------------------------------
// Clean up all dynamic UI elements
// -------------------------------------------------------------------
void ShopMenu::ClearItems() {
    for (auto& go : m_ItemTexts)
        m_Renderer->RemoveChild(go);
    for (auto& go : m_ItemSprites)
        m_Renderer->RemoveChild(go);
    m_ItemTexts.clear();
    m_ItemSprites.clear();
}

// -------------------------------------------------------------------
// Hide everything
// -------------------------------------------------------------------
void ShopMenu::Hide() {
    m_BoxUI->SetVisible(false);
    m_CursorUI->SetVisible(false);
    m_MoneyTextObj->SetVisible(false);
    ClearItems();
}

// -------------------------------------------------------------------
// Update the money text
// -------------------------------------------------------------------
void ShopMenu::SetPlayerMoney(int money) {
    m_MoneyText->SetText("Money: $" + std::to_string(money));
}

void ShopMenu::UpdateMoneyDisplay() {
    m_MoneyTextObj->SetVisible(true);
}