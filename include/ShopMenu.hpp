#pragma once
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Renderer.hpp"
#include <memory>
#include <vector>
#include <string>
#include <map>
#include "ShopData.hpp"
#include "Map.hpp"

// Forward-declare the item properties (or include your ItemDatabase)
struct ItemProperties;  

class ShopMenu {
public:
    enum class Mode { BUY, SELL };
    enum class Result { NONE, BACK, BUY_ITEM, SELL_ITEM };

    explicit ShopMenu(std::shared_ptr<Util::Renderer> renderer);
    ~ShopMenu() = default;

    // Load items for BUY mode (from shop data)
    void LoadBuyItems(const std::vector<ShopItem>& shopItems,
                      const std::function<const ItemProperties&(const std::string&)>& getProps);
    // Load items for SELL mode (from player inventory)
    void LoadSellItems(const std::map<std::string, int>& playerInventory,
                       const std::function<const ItemProperties&(const std::string&)>& getProps);

    void Show(Mode mode);
    void Hide();
    Result Update();  // returns what happened this frame
    std::string GetSelectedItemName() const { return m_SelectedItemName; }
    Mode GetMode() const { return m_Mode; }
    void SetPlayerMoney(int money);   // NEW
    void Show(Mode mode, int playerMoney = 0);  

private:
    void RebuildDisplay();      // recreate text objects from m_DisplayList
    void UpdateCursorPosition();
    void ClearItems();

    std::shared_ptr<Util::Renderer> m_Renderer;

    // UI
    std::shared_ptr<Util::GameObject> m_BoxUI;
    std::vector<std::shared_ptr<Util::GameObject>> m_ItemTexts;
    std::shared_ptr<Util::GameObject> m_CursorUI;

    // Data
    Mode m_Mode = Mode::BUY;
    struct DisplayLine {
        std::string text;       // what’s drawn
        std::string itemName;   // internal reference
        std::string shopTexturePath;
    };
    std::vector<DisplayLine> m_DisplayList;
    int m_CursorIndex = 0;

    // Selection result
    std::string m_SelectedItemName;
    Result m_LastResult = Result::NONE;

    std::vector<std::shared_ptr<Util::GameObject>> m_ItemSprites;

    std::shared_ptr<Util::GameObject> m_MoneyTextObj;
    std::shared_ptr<Util::Text>       m_MoneyText;
    void UpdateMoneyDisplay();

    // Input cooldown
    int m_InputTimer = 0;
    static constexpr int INPUT_DELAY = 10;

    // Layout constants (tweak to fit your screen)
    static constexpr float TEXT_LEFT_MARGIN = -400.0f;
    static constexpr float START_Y = 200.0f;
    static constexpr float LINE_SPACING = 40.0f;
    static constexpr float CURSOR_OFFSET_X = -60.0f;
};