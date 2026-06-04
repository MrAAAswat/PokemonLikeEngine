#pragma once

#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include "Util/Renderer.hpp"
#include "Item.hpp" 
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

class InventoryMenu {
public:
    InventoryMenu(std::shared_ptr<Util::Renderer> renderer);
    ~InventoryMenu() = default;

    void Show(const std::map<ItemCategory, std::vector<std::pair<std::string, int>>>& categorizedItems);
    void Hide();
    bool Update(); 

    std::string GetSelectedItem() const {
        if (m_CategorizedItems.count(m_CurrentTab) == 0) return "";
        const auto& list = m_CategorizedItems.at(m_CurrentTab);
        if (list.empty() || m_SelectedIndex >= static_cast<int>(list.size())) return "";
        return list[m_SelectedIndex].first;
    }

    ItemCategory GetCurrentTab() const { 
        return m_CurrentTab; 
    }

private:
    void LoadItemTextureRegistry();
    void RebuildDisplay();
    void ClearDisplayItems();

    std::shared_ptr<Util::Renderer> m_Renderer;

    // UI Graphic Components
    std::shared_ptr<Util::GameObject> m_BoxUI;              
    std::shared_ptr<Util::GameObject> m_HeaderTextObj;     
    std::shared_ptr<Util::Text>       m_HeaderText;
    std::shared_ptr<Util::GameObject> m_LargePreviewIcon;  

    // Render array list tracking
    std::vector<std::shared_ptr<Util::GameObject>> m_ItemTexts;

    // State Tracking
    std::map<ItemCategory, std::vector<std::pair<std::string, int>>> m_CategorizedItems;
    ItemCategory m_CurrentTab = ItemCategory::GENERAL;
    
    // Internal JSON item texture mapping cache 
    std::unordered_map<std::string, std::string> m_ItemShopTextures;
    
    int m_SelectedIndex = 0;
    int m_ScrollOffset = 0;
    
    // --- 1280x720 NATIVE CALIBRATED LAYOUT METRICS ---
    static constexpr int   MAX_VISIBLE_ITEMS = 7;        // Matches the 7 notepad slots
    static constexpr float START_Y           = 242.0f;   // Calibrated baseline for top dotted line
    static constexpr float LINE_SPACING       = 59.5f;    // Scaled distance matching 60px grid perfectly
    static constexpr float TEXT_OFFSET_X      = -340.0f;  // Locked left alignment boundary for item list
    static constexpr int   INPUT_DELAY        = 10;       
    int m_InputTimer = 0;
};