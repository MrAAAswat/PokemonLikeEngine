#ifndef INVENTORY_MENU_HPP
#define INVENTORY_MENU_HPP

#include "pch.hpp"
#include "Item.hpp"
#include "Map.hpp" // For ItemProperties definition
#include "Util/Renderer.hpp"
#include "Util/GameObject.hpp"
#include "Util/Text.hpp"
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <functional>

class InventoryMenu {
public:
    explicit InventoryMenu(std::shared_ptr<Util::Renderer> renderer);

    // Default getProps to nullptr to prevent breaking existing single-argument calls
    void Show(const std::map<ItemCategory, std::vector<std::pair<std::string, int>>>& categorizedItems,
              const std::function<const ItemProperties&(const std::string&)>& getProps = nullptr);
    void Hide();
    bool Update(); 

    // Public getters requested by BattleUI.cpp
    std::string GetSelectedItem() const;
    ItemCategory GetCurrentTab() const;

private:
    void RebuildDisplay();
    void ClearDisplayItems();
    void UpdatePreviewImage();

    std::shared_ptr<Util::Renderer> m_Renderer;

    // UI Graphic Components
    std::shared_ptr<Util::GameObject> m_BoxUI;              
    std::shared_ptr<Util::GameObject> m_HeaderTextObj;     
    std::shared_ptr<Util::Text>       m_HeaderText;
    std::shared_ptr<Util::GameObject> m_LargePreviewIcon;  

    // Text tracking
    std::vector<std::shared_ptr<Util::GameObject>> m_ItemTexts;

    // State Tracking (Completely Unchanged)
    std::map<ItemCategory, std::vector<std::pair<std::string, int>>> m_CategorizedItems;
    ItemCategory m_CurrentTab = ItemCategory::GENERAL;
    
    // Property lookup function hook
    std::function<const ItemProperties&(const std::string&)> m_GetProps;
    
    int m_SelectedIndex = 0;
    int m_ScrollOffset = 0;
    
    // --- 1280x720 METRICS ---
    static constexpr int   MAX_VISIBLE_ITEMS = 7;        
    static constexpr float START_Y           = 262.0f;   
    static constexpr float LINE_SPACING       = 59.5f;    
    static constexpr float TEXT_OFFSET_X      = -250.0f;  
    static constexpr int   INPUT_DELAY        = 10;       
    int m_InputTimer = 0;
};

#endif