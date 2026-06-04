#include "InventoryMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <fstream>
#include <cctype>

std::string ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){ return std::tolower(c); });
    return str;
}

InventoryMenu::InventoryMenu(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(renderer)
{
    // 1. Full Screen Background Canvas Configuration
    m_BoxUI = std::make_shared<Util::GameObject>();
    auto bgImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/itemstorage_bg.PNG");
    m_BoxUI->SetDrawable(bgImg);
    m_BoxUI->SetZIndex(80.0f);
    m_BoxUI->m_Transform.translation = {0.0f, 0.0f}; // Perfectly centered for 1280x720 canvas
    m_BoxUI->m_Transform.scale = {1.0f, 1.0f};       // Clean 1:1 layout mapping
    m_BoxUI->SetVisible(false);
    m_Renderer->AddChild(m_BoxUI);

    // 2. Folder Tab Title Text (High-contrast bold font config)
    m_HeaderText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 34, "ITEMS", Util::Color(255, 255, 255)
    );
    m_HeaderTextObj = std::make_shared<Util::GameObject>();
    m_HeaderTextObj->SetDrawable(m_HeaderText);
    m_HeaderTextObj->SetZIndex(82.0f);
    m_HeaderTextObj->SetVisible(false);
    m_Renderer->AddChild(m_HeaderTextObj);

    // 3. Lower Left Hand Selection Preview Box Setup
    m_LargePreviewIcon = std::make_shared<Util::GameObject>();
    m_LargePreviewIcon->SetZIndex(82.0f);
    m_LargePreviewIcon->m_Transform.scale = {3.0f, 3.0f}; // Crisp nearest-neighbor upscale for item sprites
    // Calibrated directly to center within the bottom-left white selection frame box
    m_LargePreviewIcon->m_Transform.translation = {-414.0f, -258.0f}; 
    m_LargePreviewIcon->SetVisible(false);
    m_Renderer->AddChild(m_LargePreviewIcon);

    LoadItemTextureRegistry();
}

void InventoryMenu::LoadItemTextureRegistry() {
    std::string jsonPath = RESOURCE_DIR "/JSON/items.json"; 
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        jsonPath = RESOURCE_DIR "/items.json";
        file.open(jsonPath);
    }

    if (file.is_open()) {
        try {
            nlohmann::json j;
            file >> j;
            if (j.contains("items") && j["items"].is_array()) {
                for (const auto& item : j["items"]) {
                    if (item.contains("name")) {
                        std::string name = item["name"].get<std::string>();
                        std::string texPath = "";
                        
                        // Resilient check matching your accurate schema keys
                        if (item.contains("shopTexturePath")) {
                            texPath = item["shopTexturePath"].get<std::string>();
                        } else if (item.contains("texturePath")) {
                            texPath = item["texturePath"].get<std::string>();
                        }
                        
                        if (!texPath.empty()) {
                            m_ItemShopTextures[ToLower(name)] = texPath;
                        }
                    }
                }
            }
        } catch (...) {}
    }
}

void InventoryMenu::Show(const std::map<ItemCategory, std::vector<std::pair<std::string, int>>>& categorizedItems) {
    m_CategorizedItems = categorizedItems;
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
    const auto& currentList = m_CategorizedItems[m_CurrentTab];

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

    if (!currentList.empty()) {
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

    // --- FIX: Dynamic Center Alignment Management for Folder Tab Title ---
    float headerW = m_HeaderText->GetSize().x;
    m_HeaderTextObj->m_Transform.translation = { -440.0f + (headerW / 2.0f), 282.0f };

    const auto& currentList = m_CategorizedItems[m_CurrentTab];

    // --- CASE: TAB IS EMPTY ---
    if (currentList.empty()) {
        m_LargePreviewIcon->SetVisible(false);

        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 36, "  (Empty)", Util::Color(140, 140, 140)
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

    // --- CASE: POPULATING THE ROW LINES ---
    int endIndex = std::min(static_cast<int>(currentList.size()), m_ScrollOffset + MAX_VISIBLE_ITEMS);
    int uiRowCounter = 0;

    for (int i = m_ScrollOffset; i < endIndex; ++i) {
        float yPos = START_Y - static_cast<float>(uiRowCounter) * LINE_SPACING;
        
        std::string prefix = (i == m_SelectedIndex) ? "> " : "  ";
        std::string lineText = prefix + currentList[i].first + "   x" + std::to_string(currentList[i].second);

        // Increased font size to 36 and applied standard dark charcoal gray text layout
        auto txt = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 36, lineText, Util::Color(70, 70, 70)
        );
        auto txtObj = std::make_shared<Util::GameObject>();
        txtObj->SetDrawable(txt);
        txtObj->SetZIndex(81.0f);

        // --- FIX: Forces left-edge alignment despite center-anchored engine defaults ---
        float textW = txt->GetSize().x;
        txtObj->m_Transform.translation = { TEXT_OFFSET_X + (textW / 2.0f), yPos };

        m_Renderer->AddChild(txtObj);
        m_ItemTexts.push_back(txtObj);

        // --- HOVER PREVIEW ICON RENDERING ---
        if (i == m_SelectedIndex) {
            std::string itemNameKey = ToLower(currentList[i].first);
            if (m_ItemShopTextures.count(itemNameKey) > 0) {
                std::string rawPath = m_ItemShopTextures[itemNameKey];
                std::string fullTexturePath = rawPath;

                // Built-in intelligent lookup sequence to handle any JSON path layout variant safely
                if (!std::ifstream(fullTexturePath).good()) {
                    fullTexturePath = RESOURCE_DIR "/" + rawPath;
                }
                if (!std::ifstream(fullTexturePath).good()) {
                    fullTexturePath = RESOURCE_DIR "/UI/" + rawPath;
                }
                if (!std::ifstream(fullTexturePath).good()) {
                    fullTexturePath = RESOURCE_DIR "/Items/" + rawPath;
                }

                auto iconImg = ResourceManager::GetImageStore().Get(fullTexturePath);
                if (iconImg) {
                    m_LargePreviewIcon->SetDrawable(iconImg);
                    m_LargePreviewIcon->SetVisible(true);
                } else {
                    m_LargePreviewIcon->SetVisible(false);
                }
            } else {
                m_LargePreviewIcon->SetVisible(false);
            }
        }
        uiRowCounter++;
    }
}

void InventoryMenu::ClearDisplayItems() {
    for (auto& textObj : m_ItemTexts) {
        m_Renderer->RemoveChild(textObj);
    }
    m_ItemTexts.clear();
}