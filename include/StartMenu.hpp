#pragma once
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Renderer.hpp"
#include <memory>
#include <vector>
#include <string>

class StartMenu {
public:
    enum class Option {
        NONE = -1,
        POKEMON,
        BAG,
        SAVE,
        EXIT,
        CANCEL
    };

    struct Item {
        std::string label;
        Option      value;
        std::string iconPath;   // optional – path to small icon
    };

    explicit StartMenu(std::shared_ptr<Util::Renderer> renderer);
    ~StartMenu() = default;

    void SetVisible(bool visible);
    Option Update();

    // Call before SetVisible(true) to show player stats
    void SetPlayerInfo(int money, int partySize);

private:
    void BuildMenuGraphics();
    void UpdateCursorPosition();
    Option ProcessInput();

    std::shared_ptr<Util::Renderer> m_Renderer;

    // Layout constants
    static constexpr float BOX_SCALE_X      = 1.0f;
    static constexpr float BOX_POS_X        = 331.5f;
    static constexpr float BOX_POS_Y        = 0.0f;
    static constexpr float INFO_START_Y     = 150.0f;   // top info line
    static constexpr float INFO_SPACING     = 35.0f;
    static constexpr float OPTION_START_Y   = 70.0f;    // first menu option (shifted down)
    static constexpr float TEXT_LEFT_MARGIN = 170.0f;
    static constexpr float LINE_SPACING     = 40.0f;
    static constexpr float CURSOR_OFFSET_X  = -50.0f;
    static constexpr float SPRITE_SCALE     = 2.0f;
    static constexpr int   INPUT_COOLDOWN   = 10;

    // Data – edit this to change the menu
    const std::vector<Item> m_Items = {
        {"POKEMON", Option::POKEMON},
        {"BAG",     Option::BAG},
        {"SAVE",    Option::SAVE},
        {"EXIT",    Option::EXIT}
    };

    int m_CursorIndex = 0;
    int m_InputTimer  = 0;

    // UI elements
    std::shared_ptr<Util::GameObject> m_BoxUI;
    std::vector<std::shared_ptr<Util::GameObject>> m_ItemTexts;   // one per option
    std::vector<std::shared_ptr<Util::GameObject>> m_ItemSprites; // icons
    std::shared_ptr<Util::GameObject> m_CursorUI;

    // Player info (static text)
    std::shared_ptr<Util::GameObject> m_MoneyTextObj;
    std::shared_ptr<Util::Text>       m_MoneyText;
    std::shared_ptr<Util::GameObject> m_PartyTextObj;
    std::shared_ptr<Util::Text>       m_PartyText;

    void BuildInfoDisplay();
    void ClearAll();
};