#include "InventoryMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include <algorithm>

InventoryMenu::InventoryMenu(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(renderer)
{
    // Background Frame
    m_BoxUI = std::make_shared<Util::GameObject>();
    auto bgImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/itemstorage_bg.PNG");
    m_BoxUI->SetDrawable(bgImg);
    m_BoxUI->SetZIndex(80.0f);
    m_BoxUI->m_Transform.translation = {0.0f, 0.0f};
    m_BoxUI->m_Transform.scale = {1.0f, 1.0f};
    m_BoxUI->SetVisible(false);
    m_Renderer->AddChild(m_BoxUI);

    // Tab Header Title
    m_HeaderText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 34, "ITEMS", Util::Color(255, 255, 255)
    );
    m_HeaderTextObj = std::make_shared<Util::GameObject>();
    m_HeaderTextObj->SetDrawable(m_HeaderText);
    m_HeaderTextObj->SetZIndex(82.0f);
    m_HeaderTextObj->SetVisible(false);
    m_Renderer->AddChild(m_HeaderTextObj);

    // Bottom-Left Selection Preview Window
    m_LargePreviewIcon = std::make_shared<Util::GameObject>();
    m_LargePreviewIcon->SetZIndex(82.0f);
    m_LargePreviewIcon->m_Transform.scale = {3.0f, 3.0f}; 
    m_LargePreviewIcon->m_Transform.translation = {-414.0f, -258.0f}; 
    m_LargePreviewIcon->SetVisible(false);
    m_Renderer->AddChild(m_LargePreviewIcon);
}

void InventoryMenu::Show(const std::map<ItemCategory, std::vector<std::pair<std::string, int>>>& categorizedItems,
                         const std::function<const ItemProperties&(const std::string&)>& getProps) {
    m_CategorizedItems = categorizedItems;
    m_GetProps = getProps;
    m_CurrentTab = ItemCategory::GENERAL;
    m_SelectedIndex = 0;
    m_ScrollOffset = 0;
    m_InputTimer = 0;

    m_BoxUI->SetVisible(true);
    m_HeaderTextObj->SetVisible(true);
    
    RebuildDisplay();
}

void InventoryMenu::Hide() {
    m_BoxUI->SetVisible(false);
    m_HeaderTextObj->SetVisible(false);
    m_LargePreviewIcon->SetVisible(false);
    ClearDisplayItems();
}

std::string InventoryMenu::GetSelectedItem() const {
    auto it = m_CategorizedItems.find(m_CurrentTab);
    if (it != m_CategorizedItems.end() && !it->second.empty()) {
        if (m_SelectedIndex >= 0 && m_SelectedIndex < static_cast<int>(it->second.size())) {
            return it->second[m_SelectedIndex].first;
        }
    }
    return "";
}

ItemCategory InventoryMenu::GetCurrentTab() const {
    return m_CurrentTab;
}

bool InventoryMenu::Update() {
    if (m_InputTimer > 0) {
        --m_InputTimer;
        if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE))
            return true;
        return false;
    }

    if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        return true; 
    }

    bool needsRedraw = false;
    auto it = m_CategorizedItems.find(m_CurrentTab);
    bool hasItems = (it != m_CategorizedItems.end() && !it->second.empty());

    if (Util::Input::IsKeyDown(Util::Keycode::RIGHT) || Util::Input::IsKeyDown(Util::Keycode::D)) {
        int nextTab = (static_cast<int>(m_CurrentTab) + 1) % static_cast<int>(ItemCategory::COUNT);
        m_CurrentTab = static_cast<ItemCategory>(nextTab);
        m_SelectedIndex = 0; 
        m_ScrollOffset = 0;
        needsRedraw = true;
    }
    else if (Util::Input::IsKeyDown(Util::Keycode::LEFT) || Util::Input::IsKeyDown(Util::Keycode::A)) {
        int prevTab = (static_cast<int>(m_CurrentTab) - 1);
        if (prevTab < 0) prevTab = static_cast<int>(ItemCategory::COUNT) - 1;
        m_CurrentTab = static_cast<ItemCategory>(prevTab);
        m_SelectedIndex = 0; 
        m_ScrollOffset = 0;
        needsRedraw = true;
    }

    if (hasItems) {
        const auto& currentList = it->second;
        if (Util::Input::IsKeyDown(Util::Keycode::UP) || Util::Input::IsKeyDown(Util::Keycode::W)) {
            if (m_SelectedIndex > 0) {
                m_SelectedIndex--;
                if (m_SelectedIndex < m_ScrollOffset) m_ScrollOffset--;
                needsRedraw = true;
            }
        }
        else if (Util::Input::IsKeyDown(Util::Keycode::DOWN) || Util::Input::IsKeyDown(Util::Keycode::S)) {
            if (m_SelectedIndex < static_cast<int>(currentList.size()) - 1) {
                m_SelectedIndex++;
                if (m_SelectedIndex >= m_ScrollOffset + MAX_VISIBLE_ITEMS) m_ScrollOffset++;
                needsRedraw = true;
            }
        }
    }

    if (needsRedraw) {
        RebuildDisplay();
        m_InputTimer = INPUT_DELAY;
    }
    return false;
}

void InventoryMenu::RebuildDisplay() {
    ClearDisplayItems();

    if (m_CurrentTab == ItemCategory::GENERAL)         m_HeaderText->SetText("ITEMS");
    else if (m_CurrentTab == ItemCategory::POKEBALLS) m_HeaderText->SetText("POKEBALLS");
    else if (m_CurrentTab == ItemCategory::KEY_ITEMS) m_HeaderText->SetText("KEY ITEMS");

    float headerW = m_HeaderText->GetSize().x;
    m_HeaderTextObj->m_Transform.translation = { -440.0f + (headerW / 2.0f), 282.0f };

    const auto& currentList = m_CategorizedItems[m_CurrentTab];

    if (currentList.empty()) {
        m_LargePreviewIcon->SetVisible(false);

        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 34, "  (Empty)", Util::Color(140, 140, 140)
        );
        auto txtObj = std::make_shared<Util::GameObject>();
        txtObj->SetDrawable(txt);
        txtObj->SetZIndex(81.0f);
        
        float textW = txt->GetSize().x;
        txtObj->m_Transform.translation = { TEXT_OFFSET_X + (textW / 2.0f), START_Y };
        
        m_Renderer->AddChild(txtObj);
        m_ItemTexts.push_back(txtObj);
        return;
    }

    int endIndex = std::min(static_cast<int>(currentList.size()), m_ScrollOffset + MAX_VISIBLE_ITEMS);
    int uiRowCounter = 0;

    for (int i = m_ScrollOffset; i < endIndex; ++i) {
        float yPos = START_Y - static_cast<float>(uiRowCounter) * LINE_SPACING;
        
        std::string prefix = (i == m_SelectedIndex) ? "> " : "  ";
        std::string lineText = prefix + currentList[i].first + "   x" + std::to_string(currentList[i].second);

        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 34, lineText, Util::Color(70, 70, 70)
        );
        auto txtObj = std::make_shared<Util::GameObject>();
        txtObj->SetDrawable(txt);
        txtObj->SetZIndex(81.0f);

        float textW = txt->GetSize().x;
        txtObj->m_Transform.translation = { TEXT_OFFSET_X + (textW / 2.0f), yPos };

        m_Renderer->AddChild(txtObj);
        m_ItemTexts.push_back(txtObj);
        uiRowCounter++;
    }

    UpdatePreviewImage();
}

void InventoryMenu::UpdatePreviewImage() {
    const auto& currentList = m_CategorizedItems[m_CurrentTab];
    if (m_SelectedIndex < 0 || m_SelectedIndex >= static_cast<int>(currentList.size())) {
        m_LargePreviewIcon->SetVisible(false);
        return;
    }

    std::string itemName = currentList[m_SelectedIndex].first;
    std::string path = "";

    // 1. Try pulling the registry path if the lambda was provided
    if (m_GetProps) {
        path = m_GetProps(itemName).shopTexturePath;
    }

    std::shared_ptr<Util::Image> img = nullptr;
    if (!path.empty()) {
        img = ResourceManager::GetImageStore().Get(path);
    }

    // 2. Resilient Fallback (If getProps is null or path is empty)
    if (!img) {
        std::vector<std::string> fallbacks = {
            RESOURCE_DIR "/items/" + itemName + ".png",
            RESOURCE_DIR "/UI/" + itemName + ".png",
            RESOURCE_DIR "/items/" + itemName + ".PNG",
            RESOURCE_DIR "/UI/" + itemName + ".PNG"
        };
        for (const auto& p : fallbacks) {
            img = ResourceManager::GetImageStore().Get(p);
            if (img) break;
        }
    }

    if (img) {
        m_LargePreviewIcon->SetDrawable(img);
        m_LargePreviewIcon->SetVisible(true);
    } else {
        m_LargePreviewIcon->SetVisible(false);
    }
}

void InventoryMenu::ClearDisplayItems() {
    for (auto& textObj : m_ItemTexts) {
        m_Renderer->RemoveChild(textObj);
    }
    m_ItemTexts.clear();
}