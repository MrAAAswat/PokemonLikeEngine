#include "PokemonMenu.hpp"
#include "ResourceManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include <string>

const std::string POKEMON_RES = std::string(RESOURCE_DIR) + "/Pokemon/";

// Helper to get a type name string (you may already have this elsewhere)
static std::string TypeToString(PokemonType type) {
    switch (type) {
        case PokemonType::NORMAL:   return "Normal";
        case PokemonType::FIRE:     return "Fire";
        case PokemonType::WATER:    return "Water";
        case PokemonType::GRASS:    return "Grass";
        case PokemonType::ELECTRIC: return "Electric";
        case PokemonType::ICE:      return "Ice";
        case PokemonType::FIGHTING: return "Fighting";
        case PokemonType::POISON:   return "Poison";
        case PokemonType::GROUND:   return "Ground";
        case PokemonType::FLYING:   return "Flying";
        case PokemonType::PSYCHIC:  return "Psychic";
        case PokemonType::BUG:      return "Bug";
        case PokemonType::ROCK:     return "Rock";
        case PokemonType::GHOST:    return "Ghost";
        case PokemonType::DRAGON:   return "Dragon";
        default:                    return "???";
    }
}

PokemonMenu::PokemonMenu(std::shared_ptr<Util::Renderer> renderer)
    : m_Renderer(renderer)
{
        // Load background for party list
    m_PartyBgImage = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/Party_bg.png");
    // Load background for stats preview – create or download a suitable image!
    m_StatsBgImage = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/pokedex_menu.png");

    m_BoxUI = std::make_shared<Util::GameObject>();
    // Start with party list background
    m_BoxUI->SetDrawable(m_PartyBgImage);
    m_BoxUI->m_Transform.scale = {1.0f, 1.0f};
    m_BoxUI->SetZIndex(90.0f);
    m_BoxUI->m_Transform.translation = {0.0f, 0.0f};
    m_BoxUI->SetVisible(false);
    m_Renderer->AddChild(m_BoxUI);

    m_CursorUI = std::make_shared<Util::GameObject>();
    auto cursorImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/Cursor.png");
    m_CursorUI->SetDrawable(cursorImg);
    m_CursorUI->SetZIndex(92.0f);
    m_CursorUI->SetVisible(false);
    m_Renderer->AddChild(m_CursorUI);
}

void PokemonMenu::ClearSlots() {
    for (auto& slot : m_Slots) {
        if (slot.sprite) m_Renderer->RemoveChild(slot.sprite);
        if (slot.text)   m_Renderer->RemoveChild(slot.text);
    }
    m_Slots.clear();
}
// ──────────────────────────────────────
//  Party list slots (unchanged)
// ──────────────────────────────────────
void PokemonMenu::BuildSlots(const std::vector<std::shared_ptr<Pokemon>>& party) {
    ClearSlots();

    if (party.empty()) {
        // ... (Keep your empty party logic here) ...
        return;
    }

    // Coordinates for the 6 boxes (Top L/R, Mid L/R, Bot L/R)
    std::vector<glm::vec2> gridPositions = {
        {COL_1_X, ROW_1_Y}, {COL_2_X, ROW_1_Y},
        {COL_1_X, ROW_2_Y}, {COL_2_X, ROW_2_Y},
        {COL_1_X, ROW_3_Y}, {COL_2_X, ROW_3_Y}
    };

    // Build max 6 slots to match your UI
    for (size_t i = 0; i < party.size() && i < 6; ++i) {
        const auto& p = party[i];
        Slot slot;

        // Sprite
        std::string spritePath = POKEMON_RES + p->GetName() + "_front_1.png";
        auto spriteObj = std::make_shared<Util::GameObject>();
        auto spriteImg = ResourceManager::GetImageStore().Get(spritePath);
        if (!spriteImg) {
            spriteImg = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/Pokemon/placeholder.png");
        }
        spriteObj->SetDrawable(spriteImg);
        spriteObj->m_Transform.scale = {SPRITE_SCALE, SPRITE_SCALE};
        spriteObj->SetZIndex(91.0f);
        spriteObj->m_Transform.translation = { gridPositions[i].x + SPRITE_OFFSET_X, gridPositions[i].y };

        // Info text - using \n to stack Name and HP so it fits nicely in the box
        std::string info = p->GetName() + "  Lv." + std::to_string(p->GetLevel()) + "\n" + "\n"
                         + "HP: " + std::to_string(p->GetCurrentHP()) + "/" + std::to_string(p->GetMaxHP());
        
        auto textDraw = std::make_shared<Util::Text>(
            RESOURCE_DIR "/Fonts/power clear.ttf", 24, info, Util::Color(255, 255, 255));
        auto textObj = std::make_shared<Util::GameObject>();
        textObj->SetDrawable(textDraw);
        textObj->SetZIndex(91.0f);
        
        // Align text to the right of the sprite
        float textHalfW = textDraw->GetSize().x / 2.0f;
        textObj->m_Transform.translation = { gridPositions[i].x + TEXT_OFFSET_X + textHalfW, gridPositions[i].y };

        m_Renderer->AddChild(spriteObj);
        m_Renderer->AddChild(textObj);

        slot.sprite = spriteObj;
        slot.text   = textObj;
        m_Slots.push_back(slot);
    }
}

// ──────────────────────────────────────
//  Single‑Pokémon preview slot
// ──────────────────────────────────────
void PokemonMenu::BuildPreviewSlot(const Pokemon& pkmn) {
    ClearSlots();

    Slot slot;

    // Sprite (larger, centred near top)
    std::string spritePath = POKEMON_RES + pkmn.GetName() + "_front_1.png";
    auto spriteImg = ResourceManager::GetImageStore().Get(spritePath);
    if (!spriteImg) {
        spriteImg = ResourceManager::GetImageStore().Get(
            RESOURCE_DIR "/Pokemon/placeholder.png");
    }
    slot.sprite = std::make_shared<Util::GameObject>();
    slot.sprite->SetDrawable(spriteImg);
    slot.sprite->m_Transform.scale = {3.0f, 3.0f};
    slot.sprite->m_Transform.translation = {0.0f, 120.0f};
    slot.sprite->SetZIndex(91.0f);
    m_Renderer->AddChild(slot.sprite);

    // Build the info string using the Pokemon’s own methods
    std::string info =
        pkmn.GetName() + "  Lv." + std::to_string(pkmn.GetLevel()) + "\n"
        + "Type: " + pkmn.GetTypeString() + "\n"                // e.g. "Fire/Flying"
        + "HP: " + std::to_string(pkmn.GetCurrentHP()) + "/" + std::to_string(pkmn.GetMaxHP()) + "\n"
        + "ATK:"  + std::to_string(pkmn.GetAttack())
        + "  DEF:" + std::to_string(pkmn.GetDefense()) + "\n"
        + "SP.ATK:" + std::to_string(pkmn.GetSpecialAttack())
        + "  SP.DEF:" + std::to_string(pkmn.GetSpecialDefense()) + "\n"
        + "SPEED:" + std::to_string(pkmn.GetSpeed()) + "\n\n"
        + "Moves:\n";

    // Append the actual moves (max 4) from GetMoves()
    const auto& moves = pkmn.GetMoves();
    for (size_t i = 0; i < moves.size() && i < 4; ++i) {
        info += "  " + moves[i] + "\n";
    }

    info += "\n[Z] Choose this Pokemon   [X] Cancel";

    auto textDraw = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 20, info,
        Util::Color(255, 255, 255));
    slot.text = std::make_shared<Util::GameObject>();
    slot.text->SetDrawable(textDraw);
    slot.text->SetZIndex(91.0f);
    // Centre the text block horizontally, below the sprite
    float textW = textDraw->GetSize().x;
    slot.text->m_Transform.translation = { -textW / 2.0f, -140.0f };
    m_Renderer->AddChild(slot.text);

    m_Slots.push_back(slot);
}

// ──────────────────────────────────────
//  Show / Hide
// ──────────────────────────────────────
void PokemonMenu::Show(const std::vector<std::shared_ptr<Pokemon>>& party) {
    m_BoxUI->SetDrawable(m_PartyBgImage);
    m_Mode          = Mode::PARTY_LIST;
    m_Confirmed     = false;
    m_Cancelled     = false;
    m_ActionSelected= false;
    m_PartySize     = static_cast<int>(party.size());
    m_CursorIndex   = 0;
    m_InputCooldown = 0;

    BuildSlots(party);

    m_BoxUI->SetVisible(true);
    m_CursorUI->SetVisible(!party.empty());
    UpdateCursorPosition();
}

void PokemonMenu::ShowPreview(const Pokemon& pkmn) {
    m_BoxUI->SetDrawable(m_StatsBgImage); 
    m_Mode      = Mode::PREVIEW;
    m_Confirmed = false;
    m_Cancelled = false;
    m_PartySize = 0;   // no cursor navigation
    m_InputCooldown = 0;

    BuildPreviewSlot(pkmn);

    m_BoxUI->SetVisible(true);
    m_CursorUI->SetVisible(false);
}

void PokemonMenu::Hide() {
    m_BoxUI->SetVisible(false);
    m_CursorUI->SetVisible(false);
    ClearSlots();
    m_Confirmed = false;
    m_Cancelled = false;
    m_ActionSelected = false;
}

// ──────────────────────────────────────
//  Update (mode‑aware)
// ──────────────────────────────────────
bool PokemonMenu::Update() {
    if (m_Mode == Mode::PARTY_LIST) {
        if (m_InputCooldown > 0) {
            --m_InputCooldown;
            return false;
        }

        if (m_PartySize > 0) {
            // 2D Navigation mapping
            if (Util::Input::IsKeyDown(Util::Keycode::RIGHT) || Util::Input::IsKeyDown(Util::Keycode::D)) {
                if (m_CursorIndex % 2 == 0 && m_CursorIndex + 1 < m_PartySize) {
                    m_CursorIndex++; m_InputCooldown = INPUT_DELAY; UpdateCursorPosition();
                }
            }
            else if (Util::Input::IsKeyDown(Util::Keycode::LEFT) || Util::Input::IsKeyDown(Util::Keycode::A)) {
                if (m_CursorIndex % 2 != 0) {
                    m_CursorIndex--; m_InputCooldown = INPUT_DELAY; UpdateCursorPosition();
                }
            }
            else if (Util::Input::IsKeyDown(Util::Keycode::DOWN) || Util::Input::IsKeyDown(Util::Keycode::S)) {
                if (m_CursorIndex + 2 < m_PartySize) {
                    m_CursorIndex += 2; m_InputCooldown = INPUT_DELAY; UpdateCursorPosition();
                }
            }
            else if (Util::Input::IsKeyDown(Util::Keycode::UP) || Util::Input::IsKeyDown(Util::Keycode::W)) {
                if (m_CursorIndex - 2 >= 0) {
                    m_CursorIndex -= 2; m_InputCooldown = INPUT_DELAY; UpdateCursorPosition();
                }
            }

            // Select to view Preview
            if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
                m_ActionSelected = true;
                return true;
            }
        }

        if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
            return true;
        }
        return false;
    }

    if (m_Mode == Mode::PREVIEW) {
    if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
        m_ActionSelected = true;
        m_Confirmed = true;
        return true;
    }
    if (Util::Input::IsKeyDown(Util::Keycode::X) || 
        Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_Cancelled = true;
        return true;
        }
    }
    return false;
}

void PokemonMenu::UpdateCursorPosition() {
    if (m_PartySize == 0 || m_CursorIndex >= m_PartySize) return;

    std::vector<glm::vec2> gridPositions = {
        {COL_1_X, ROW_1_Y}, {COL_2_X, ROW_1_Y},
        {COL_1_X, ROW_2_Y}, {COL_2_X, ROW_2_Y},
        {COL_1_X, ROW_3_Y}, {COL_2_X, ROW_3_Y}
    };

    m_CursorUI->m_Transform.translation = {
        gridPositions[m_CursorIndex].x + CURSOR_OFFSET_X,
        gridPositions[m_CursorIndex].y
    };
}