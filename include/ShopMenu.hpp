#pragma once
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Renderer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <functional>
#include "ShopData.hpp"          
#include "ItemDatabase.hpp" 
#include "Map.hpp" 

class ShopMenu {
public:
    enum class Mode { BUY, SELL };
    enum class Result { NONE, BACK, BUY_ITEM, SELL_ITEM };

    explicit ShopMenu(std::shared_ptr<Util::Renderer> renderer);
    ~ShopMenu() = default;

    // Load items for BUY mode
    void LoadBuyItems(const std::vector<ShopItem>& shopItems,
                      const std::function<const ItemProperties&(const std::string&)>& getProps);
    // Load items for SELL mode
    void LoadSellItems(const std::map<std::string, int>& playerInventory,
                       const std::function<const ItemProperties&(const std::string&)>& getProps);

    void Show(Mode mode, int playerMoney = 0);
    void Hide();
    Result Update();                       // returns action
    std::string GetSelectedItemName() const { return m_SelectedItemName; }
    Mode GetMode() const { return m_Mode; }

    // To support TAB toggle
    void SetBuyData(const std::vector<ShopItem>& items,
                    const std::function<const ItemProperties&(const std::string&)>& getProps);
    void SetPlayerInventory(const std::map<std::string, int>& inventory);
    void ToggleMode();
    void SetPlayerMoney(int money);          // <-- ADD THIS
    void UpdateMoneyDisplay();               // <-- ADD THIS

private:
    struct DisplayLine {
        std::string text;
        std::string itemName;
        std::string shopTexturePath;
    };

    void RebuildDisplay();
    void UpdateCursorPosition();
    void UpdatePreviewImage();              // NEW
    void ClearItems();

    std::shared_ptr<Util::Renderer> m_Renderer;

    // --- UI Elements ---
    std::shared_ptr<Util::GameObject> m_BoxUI;
    std::vector<std::shared_ptr<Util::GameObject>> m_ItemTexts;
    std::vector<std::shared_ptr<Util::GameObject>> m_ItemSprites;
    std::shared_ptr<Util::GameObject> m_CursorUI;

    // Money display
    std::shared_ptr<Util::GameObject> m_MoneyTextObj;
    std::shared_ptr<Util::Text>       m_MoneyText;

    // Large preview icon (bottom‑left)
    std::shared_ptr<Util::GameObject> m_LargePreviewIcon;   // NEW

    // --- Data ---
    Mode m_Mode = Mode::BUY;
    std::vector<DisplayLine> m_DisplayList;
    int m_CursorIndex = 0;

    std::string m_SelectedItemName;
    Result m_LastResult = Result::NONE;

    // For toggling
    std::vector<ShopItem> m_BuyItems;
    std::function<const ItemProperties&(const std::string&)> m_GetProps;
    std::map<std::string, int> m_PlayerInventory;
    int m_PlayerMoney = 0;

    // Input cooldown
    int m_InputTimer = 0;
    static constexpr int INPUT_DELAY = 10;

    // Layout – calibrated to itemstorage_bg.PNG notepad area
    static constexpr float START_Y          = 242.0f;   // top slot
    static constexpr float LINE_SPACING     = 59.5f;
    static constexpr float TEXT_OFFSET_X    = -220.0f;  // left edge of notepad
    static constexpr float CURSOR_OFFSET_X  = -260.0f;  // to the left of text
    static constexpr float SPRITE_SCALE     = 2.0f;
    static constexpr float SPRITE_X_OFFSET  = -300.0f;  // small icon next to text
    static constexpr float MONEY_TEXT_X     = 300.0f;   // top‑right of screen
    static constexpr float MONEY_TEXT_Y     = 290.0f;
    static constexpr float PREVIEW_POS_X    = -388.0f;  // bottom‑left box
    static constexpr float PREVIEW_POS_Y    = -250.0f;
};
