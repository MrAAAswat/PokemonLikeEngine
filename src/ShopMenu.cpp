#include "ShopMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

ShopMenu::ShopMenu(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(renderer)
{
    // ── Full‑screen background (same as inventory) ──────────────────────
    m_BoxUI = std::make_shared<Util::GameObject>();
    auto boxImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/itemstorage_bg.PNG");
    m_BoxUI->SetDrawable(boxImg);
    m_BoxUI->SetZIndex(90.0f);
    m_BoxUI->m_Transform.translation = {0.0f, 0.0f};
    m_BoxUI->m_Transform.scale = {1.0f, 1.0f};
    m_BoxUI->SetVisible(false);
    m_Renderer->AddChild(m_BoxUI);

    // ── Cursor ───────────────────────────────────────────────────────────
    m_CursorUI = std::make_shared<Util::GameObject>();
    auto cursorImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/Cursor.png");
    m_CursorUI->SetDrawable(cursorImg);
    m_CursorUI->SetZIndex(92.0f);
    m_CursorUI->SetVisible(false);
    m_Renderer->AddChild(m_CursorUI);

    // ── Money display ────────────────────────────────────────────────────
    m_MoneyText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 30,
        "Money: $0", Util::Color(50, 50, 50));
    m_MoneyTextObj = std::make_shared<Util::GameObject>();
    m_MoneyTextObj->SetDrawable(m_MoneyText);
    m_MoneyTextObj->SetZIndex(93.0f);
    m_MoneyTextObj->SetVisible(false);
    m_MoneyTextObj->m_Transform.translation = {MONEY_TEXT_X, MONEY_TEXT_Y};
    m_Renderer->AddChild(m_MoneyTextObj);

    // ── Large preview icon (bottom‑left) ─────────────────────────────────
    m_LargePreviewIcon = std::make_shared<Util::GameObject>();
    m_LargePreviewIcon->SetZIndex(93.0f);
    m_LargePreviewIcon->m_Transform.scale = {3.0f, 3.0f};   // same upscale as inventory
    m_LargePreviewIcon->m_Transform.translation = {PREVIEW_POS_X, PREVIEW_POS_Y};
    m_LargePreviewIcon->SetVisible(false);
    m_Renderer->AddChild(m_LargePreviewIcon);
}

// -------------------------------------------------------------------
// Load items for BUY mode (unchanged except using shopTexturePath)
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
// Show / Hide
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
    UpdatePreviewImage();            // show the first item
}

void ShopMenu::Hide() {
    m_BoxUI->SetVisible(false);
    m_CursorUI->SetVisible(false);
    m_MoneyTextObj->SetVisible(false);
    m_LargePreviewIcon->SetVisible(false);
    ClearItems();
}

// -------------------------------------------------------------------
// Rebuild the list (sprites + text) inside notepad area
// -------------------------------------------------------------------
void ShopMenu::RebuildDisplay() {
    ClearItems();

    for (size_t i = 0; i < m_DisplayList.size(); ++i) {
        const auto& line = m_DisplayList[i];
        float y = START_Y - static_cast<float>(i) * LINE_SPACING;

        // Text only
        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 22,
            line.text, Util::Color(70, 70, 70));
        auto txtObj = std::make_shared<Util::GameObject>();
        txtObj->SetDrawable(txt);
        txtObj->SetZIndex(91.0f);
        float textW = txt->GetSize().x;
        txtObj->m_Transform.translation = { TEXT_OFFSET_X + textW / 2.0f, y };
        m_Renderer->AddChild(txtObj);
        m_ItemTexts.push_back(txtObj);
    }
}

// -------------------------------------------------------------------
// Cursor & preview image
// -------------------------------------------------------------------
void ShopMenu::UpdateCursorPosition() {
    if (static_cast<size_t>(m_CursorIndex) < m_ItemTexts.size()) {
        float y = m_ItemTexts[m_CursorIndex]->m_Transform.translation.y;
        m_CursorUI->m_Transform.translation = {
            CURSOR_OFFSET_X,               // to the left of the small icon
            y + 5.0f
        };
    }
    UpdatePreviewImage();   // refresh the big preview
}

void ShopMenu::UpdatePreviewImage() {
    if (m_CursorIndex < 0 || m_CursorIndex >= static_cast<int>(m_DisplayList.size())) {
        m_LargePreviewIcon->SetVisible(false);
        return;
    }

    const std::string& path = m_DisplayList[m_CursorIndex].shopTexturePath;
    if (path.empty()) {
        m_LargePreviewIcon->SetVisible(false);
        return;
    }

    auto img = ResourceManager::GetImageStore().Get(path);
    if (img) {
        m_LargePreviewIcon->SetDrawable(img);
        m_LargePreviewIcon->SetVisible(true);
    } else {
        m_LargePreviewIcon->SetVisible(false);
    }
}

// -------------------------------------------------------------------
// Input handling (TAB toggle included)
// -------------------------------------------------------------------
ShopMenu::Result ShopMenu::Update() {
    // Toggle buy/sell with TAB (immediate)
    if (Util::Input::IsKeyDown(Util::Keycode::TAB)) {
        ToggleMode();
        m_InputTimer = INPUT_DELAY;
        return Result::NONE;
    }

    // Cooldown
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
        UpdateCursorPosition();   // also updates preview
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
// Cleanup
// -------------------------------------------------------------------
void ShopMenu::ClearItems() {
    for (auto& go : m_ItemTexts)
        m_Renderer->RemoveChild(go);
    m_ItemTexts.clear();
}
// -------------------------------------------------------------------
// Money display
// -------------------------------------------------------------------
void ShopMenu::SetPlayerMoney(int money) {
    m_PlayerMoney = money;
    m_MoneyText->SetText("Money: $" + std::to_string(money));
}

void ShopMenu::UpdateMoneyDisplay() {
    m_MoneyTextObj->SetVisible(true);
}

// -------------------------------------------------------------------
// Data storage for TAB toggle
// -------------------------------------------------------------------
void ShopMenu::SetBuyData(const std::vector<ShopItem>& items,
                          const std::function<const ItemProperties&(const std::string&)>& getProps) {
    m_BuyItems = items;
    m_GetProps = getProps;
}

void ShopMenu::SetPlayerInventory(const std::map<std::string, int>& inventory) {
    m_PlayerInventory = inventory;
}

void ShopMenu::ToggleMode() {
    if (m_Mode == Mode::BUY) {
        if (!m_GetProps) return;
        LoadSellItems(m_PlayerInventory, m_GetProps);
        Show(Mode::SELL, m_PlayerMoney);
    } else {
        LoadBuyItems(m_BuyItems, m_GetProps);
        Show(Mode::BUY, m_PlayerMoney);
    }
}