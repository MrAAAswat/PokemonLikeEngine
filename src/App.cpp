#include "App.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "GameConfig.hpp"
#include "Prop.hpp"
#include "SaveSystem.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Text.hpp"
#include <iostream>
#include "StartMenu.hpp"
#include "InventoryMenu.hpp"
#include "PokemonMenu.hpp"
#include "RandomEncounters.hpp"
#include "ResourceManager.hpp"
#include "PokemonDatabase.hpp"
#include "MapGenerator.hpp"
#include "BattleAnimation.hpp"
#include "ItemDatabase.hpp"
#include "TitleScreen.hpp"

const std::string RES      = std::string(RESOURCE_DIR);
const std::string MAP_DIR  = RES + "/maps/";

// ==========================================
// CORE LIFECYCLE
// ==========================================

void App::Start() {
    LOG_INFO("Start Game");

    // 1. Initialise engine and UI subsystems FIRST
    InitSystems();
    InitUI();

    // 2. Create and show the title / load-game screen
    m_TitleScreen = std::make_shared<TitleScreen>(m_Renderer);
    m_TitleScreen->Load();
    m_Renderer->AddChild(m_TitleScreen);

    // 3. Start in TITLE state – InitGameLoad() will be called later after the player chooses a slot
    m_CurrentState = State::TITLE;
}

void App::Update() {
    if (Util::Input::IfExit()) {
        m_CurrentState = State::END;
        return;
    }

    HandleGlobalShortcuts();

    // Handle QuickSave trigger
    if (Util::Input::IsKeyDown(Util::Keycode::F) && m_CurrentState == State::UPDATE) {
        PerformQuickSave();
    }

    // Main State Machine routing
    switch (m_CurrentState) {
        case State::START:            break;
        case State::DIALOGUE:         ProcessDialogueState();       break;
        case State::START_MENU:       ProcessStartMenuState();      break;
        case State::TITLE:            ProcessTitleState();          break;
        case State::INVENTORY_MENU:   m_InventoryMenu->Update();    break;
        case State::POKEMON_MENU:     ProcessPokemonMenuState();    break;
        case State::UPDATE:           ProcessOverworldUpdateState(); break;
        case State::BATTLE:           ProcessBattleState();         break;
        case State::SHOP:             ProcessShopState();           break;
        case State::END:              break;
    }

    if (m_CurrentState != State::TITLE) {
        m_Map->Update();
    }
    m_Renderer->Update();
}

void App::End() {
    LOG_TRACE("End");
}

// ==========================================
// INITIALIZATION HELPERS
// ==========================================

void App::InitSystems() {
    MoveDatabase::Init();
    PokemonDatabase::Init();
    TrainerDatabase::Init();
    ItemDatabase::Load(RESOURCE_DIR "/data/items.json");
    srand(static_cast<unsigned int>(time(nullptr)));
    
    m_Renderer = std::make_shared<Util::Renderer>();
    m_Map = std::make_shared<Map>();
    m_Map->SetInteractionCallback([this](NPC* npc) {
        m_ActiveNPC = npc;
        m_Character->StopMoving();
        m_CurrentState = State::DIALOGUE;
        m_DialogueBoxUI->SetVisible(true);
        m_DialogueUI->SetVisible(true);

        // Face the NPC toward the player
        npc->FaceToward(m_Character->GetGridX(), m_Character->GetGridY());

        // Get the dialogue lines
        m_CurrentDialogueLines = npc->Interact(*m_Character);
        m_CurrentDialogueIndex = 0;

        if (!m_CurrentDialogueLines.empty()) {
            m_DialogueText->SetText(m_CurrentDialogueLines[0]);
            float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
            m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
        }
    });
    m_Map->SetRenderer(m_Renderer); 
    m_PokemonMenu = std::make_shared<PokemonMenu>(m_Renderer);
    m_Character = std::make_shared<Player>(0.0f, 0.0f);
    m_Map->SetPlayer(m_Character.get());
    m_Renderer->AddChild(m_Character); 

    m_BattleUI = std::make_shared<BattleUI>(m_Renderer);  
    
    try {
        AnimationLibrary::Get().LoadFromJson(RES + "/data/TEST.json"); 
    } catch (const std::exception& e) {
        LOG_ERROR("JSON Error: {}", e.what());
    }

    m_InventoryMenu = std::make_shared<InventoryMenu>(m_Renderer);
    m_BattleUI->SetInventoryMenu(m_InventoryMenu);
    m_BattleUI->SetPlayer(m_Character);
    m_Map->SetBattleCallback([this](NPC* npc) {
        m_PendingBattleFlag = npc->GetInteractFlag();
        m_PendingRewardItem = npc->GetRewardItemName();
        m_PendingRewardQty  = npc->GetRewardQuantity();
        m_PendingRewardCategory = npc->GetRewardItemCategory();
        m_PendingRewardMoney = npc->GetRewardMoney();
        m_PendingRewardCategory = m_ActiveNPC->GetRewardItemCategory();  // ← ADD THIS


        m_Character->SetVisible(false);
        m_Map->SetVisible(false);
        m_BattleUI->StartTrainerBattle(m_Character->GetParty(), npc->GetParty(), m_PendingBattleFlag);
        m_CurrentState = State::BATTLE;
    }
        );
    
}

// App.cpp — implementation
void App::InitGameLoad(int slot) {
    m_Map->LoadConnections(RESOURCE_DIR "/maps/connections.txt");

    // Build the save-file path for the selected slot.
    // slot == -1 means New Game (no file to load).
    std::string savePath = "";
    if (slot >= 0) {
        savePath = SaveSystem::SAVE_SLOT_PREFIX
                 + "_" + std::to_string(slot) + ".json";
    }

    SaveSystem::GameState loadedState;
    bool loaded = false;

    if (!savePath.empty()) {
        loaded = SaveSystem::LoadGame(loadedState, savePath);
    }

    if (loaded) {
        GameConfig::LootedItems    = loadedState.lootedItems;
        m_Character->SetMoney(loadedState.money);
        m_Map->LoadLevel(loadedState.mapPath);
        m_Character->SetGridPosition(loadedState.gridX, loadedState.gridY);
        m_Map->WarpTo(loadedState.gridX, loadedState.gridY);
        m_Character->SetDirection(
            static_cast<Character::Direction>(loadedState.direction));
        m_Character->SetInventory(loadedState.inventory);
        for (const auto& pkmn : loadedState.party) {
            m_Character->AddPokemon(pkmn);
        }
        m_LastHealMapPath = loadedState.lastHealMapPath;
        m_LastHealX       = loadedState.lastHealX;
        m_LastHealY       = loadedState.lastHealY;
        m_ActiveSaveSlot  = slot;   // remember which file to overwrite on save
    } else {
        // New Game defaults
        GameConfig::LootedItems.clear();
        m_Map->LoadLevel(MAP_DIR + "PlayerHouse2F");
        m_Character->SetGridPosition(3, 5);
        m_Map->WarpTo(3, 5);
        m_LastHealMapPath = "";
        m_LastHealX = m_LastHealY = -1;
        m_ActiveSaveSlot = slot; // could be -1 until the player first saves
    }
}

void App::InitUI() {
    m_DialogueBoxUI = std::make_shared<Util::GameObject>();
    auto boxImage = ResourceManager::GetImageStore().Get(RESOURCE_DIR "/UI/BWTextBox3.png");    
    m_DialogueBoxUI->SetDrawable(boxImage);
    m_DialogueBoxUI->SetZIndex(9.0f); 
    m_DialogueBoxUI->SetVisible(false);
    m_DialogueBoxUI->m_Transform.scale = {1.0f, 1.0f};          
    m_DialogueBoxUI->m_Transform.translation = {0.0f, -288.0f};

    m_DialogueUI = std::make_shared<Util::GameObject>();
    m_DialogueText = std::make_shared<Util::Text>(
        RESOURCE_DIR "/Fonts/power clear.ttf", 30, "...", Util::Color(50, 50, 50)
    );
    m_DialogueUI->SetDrawable(m_DialogueText);
    m_DialogueUI->SetZIndex(10.0f);  
    m_DialogueUI->SetVisible(false);
    m_DialogueUI->m_Transform.translation = {-600.0f, -260.0f};

    m_StartMenu = std::make_shared<StartMenu>(m_Renderer);
    
    m_Renderer->AddChild(m_DialogueBoxUI);
    m_Renderer->AddChild(m_DialogueUI);
    m_ShopMenu = std::make_shared<ShopMenu>(m_Renderer);
}

void App::PerformQuickSave() {
    // If we started a New Game and never saved, auto‑pick a free slot.
    if (m_ActiveSaveSlot < 0) {
        // Find the first empty slot by checking file existence.
        for (int i = 0; i < SaveSystem::MAX_SAVE_SLOTS; ++i) {
            std::string path = SaveSystem::SAVE_SLOT_PREFIX
                             + "_" + std::to_string(i) + ".json";
            if (!std::filesystem::exists(path)) {
                m_ActiveSaveSlot = i;
                break;
            }
        }
        // If all slots are full, just overwrite slot 0.
        if (m_ActiveSaveSlot < 0)
            m_ActiveSaveSlot = 0;
    }

    // Build the correct file path for the current slot.
    std::string savePath = SaveSystem::SAVE_SLOT_PREFIX
                         + "_" + std::to_string(m_ActiveSaveSlot) + ".json";

    SaveSystem::GameState current;
    current.money           = m_Character->GetMoney();
    current.mapPath         = m_Map->GetCurrentLevelPath();
    current.gridX           = m_Character->GetGridX();
    current.gridY           = m_Character->GetGridY();
    current.direction       = static_cast<int>(m_Character->GetFacingDirection());
    current.inventory       = m_Character->GetInventory();
    current.lootedItems     = GameConfig::LootedItems;
    current.party           = m_Character->GetParty();
    current.lastHealMapPath = m_LastHealMapPath;
    current.lastHealX       = m_LastHealX;
    current.lastHealY       = m_LastHealY;

    SaveSystem::SaveGame(current, savePath);   // <-- uses the slot path

    LOG_INFO("Game saved to slot {} → {}", m_ActiveSaveSlot, savePath);
}

// ==========================================
// STATE MACHINE EXECUTION FUNCTIONS
// ==========================================

void App::ProcessBattleState() {
    m_BattleUI->Update();

    if (!m_BattleUI->IsBattleOver() && !Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        return;   // Battle still ongoing – nothing to do
    }

    // If the player forced an exit via ESC, hide the battle UI
    if (Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        m_BattleUI->Hide();
    }

    if (m_BattleUI->PlayerWon() && !m_PendingBattleFlag.empty()) {
            std::string rewardFlag = m_PendingBattleFlag + "_rewarded";
            if (!GameFlags::Get(rewardFlag)) {
                if (!m_PendingRewardItem.empty()) {
                    m_Character->AddItem(m_PendingRewardItem, m_PendingRewardCategory, m_PendingRewardQty);
                    LOG_INFO("Received {} x{} (category {}) as battle reward.",
                            m_PendingRewardItem, m_PendingRewardQty, static_cast<int>(m_PendingRewardCategory));
                }
                if (m_PendingRewardMoney > 0) {
                    m_Character->AddMoney(m_PendingRewardMoney);
                    LOG_INFO("Received ${} as battle reward.", m_PendingRewardMoney);
                }
                GameFlags::Set(rewardFlag, true);
            }
        }

    // ----- 2. HANDLE WHITE‑OUT (all Pokémon fainted) -----
    if (!m_BattleUI->PlayerWon()) {
        bool allFainted = true;
        for (auto& p : m_Character->GetParty()) {
            if (p && !p->IsFainted()) {
                allFainted = false;
                break;
            }
        }

        if (allFainted) {
            // Heal the entire party
            for (auto& p : m_Character->GetParty()) {
                if (p) {
                    p->SetCurrentHP(p->GetMaxHP());
                    // Future: clear any status conditions here
                }
            }

            // Teleport to the last healing centre (or default starter house)
            if (!m_LastHealMapPath.empty()) {
                m_Map->LoadLevel(m_LastHealMapPath);
                m_Character->SetGridPosition(m_LastHealX, m_LastHealY);
                m_Map->WarpTo(m_LastHealX, m_LastHealY);
            } else {
                // No healing centre visited yet → default player’s house
                m_Map->LoadLevel(MAP_DIR + "PlayerHouse2F");
                m_Character->SetGridPosition(3, 5);
                m_Map->WarpTo(3, 5);
            }

            // Clear any leftover interaction state from before the battle
            m_ActiveNPC = nullptr;
            m_CurrentDialogueLines.clear();
            m_Map->SetPaused(false);

            // Reset the player’s facing direction
            m_Character->SetDirection(Character::Direction::DOWN);
        }
    }

    // ----- 3. CLEANUP AND RETURN TO OVERWORLD -----
    m_PendingBattleFlag.clear();
    m_PendingRewardItem.clear();
    m_PendingRewardQty = 0;
    m_PendingRewardMoney = 0;

    m_Map->SetVisible(true);
    m_Character->SetVisible(true);

    if (m_Map->IsNPCTrainerApproachActive()) {
        m_Map->EndNPCTrainerApproach();
    }

    m_CurrentState = State::UPDATE;
}

void App::ProcessStartMenuState() {
    StartMenu::Option selection = m_StartMenu->Update();

    switch (selection) {
        case StartMenu::Option::POKEMON: {
            LOG_TRACE("Selected: POKEMON");
            m_CurrentState = State::POKEMON_MENU;
            m_StartMenu->SetVisible(false);
            m_PokemonMenu->Show(m_Character->GetParty());
            break;
        }

        case StartMenu::Option::BAG: {
            LOG_TRACE("Selected: BAG");
            m_CurrentState = State::INVENTORY_MENU;
            m_StartMenu->SetVisible(false);

            std::map<ItemCategory, std::vector<std::pair<std::string, int>>> sorted;
            sorted[ItemCategory::GENERAL]    = {};
            sorted[ItemCategory::POKEBALLS]  = {};
            sorted[ItemCategory::KEY_ITEMS]  = {};

            for (const auto& [itemName, invData] : m_Character->GetInventory()) {
                sorted[invData.category].push_back({itemName, invData.quantity});
            }
            m_InventoryMenu->Show(sorted);
            break;
        }

        case StartMenu::Option::SAVE: {
            PerformQuickSave();
            CloseAllMenus();
            break;
        }

        case StartMenu::Option::EXIT: {
            m_CurrentState = State::END;
            break;
        }

        case StartMenu::Option::CANCEL: {
            CloseAllMenus();
            break;
        }

        case StartMenu::Option::NONE:
        default:
            break;
    }
}

void App::ProcessPokemonMenuState() {
    if (m_PokemonMenu->Update()) {
        
        // 1. If we are currently looking at a Preview, exiting returns us to the Grid List
        if (m_PokemonMenu->GetMode() == PokemonMenu::Mode::PREVIEW) {
            m_PokemonMenu->Show(m_Character->GetParty());
            return;
        }

        // 2. If we pressed 'Z' while hovering over a Pokemon in the Grid List, show the Preview
        if (m_PokemonMenu->IsActionSelected()) {
            int selectedIdx = m_PokemonMenu->GetSelectedIndex();
            auto party = m_Character->GetParty();
            if (selectedIdx >= 0 && selectedIdx < static_cast<int>(party.size()) && party[selectedIdx]){
                LOG_TRACE("Opening preview for index %d.", selectedIdx);
                m_PokemonMenu->ShowPreview(*party[selectedIdx]);
            }
            return;
        }

        // 3. Otherwise (Pressed 'X'), Exit the Pokemon menu and go back to Start Menu
        LOG_TRACE("Exited POKEMON menu");
        m_PokemonMenu->Hide();
        m_StartMenu->SetVisible(true);
        m_CurrentState = State::START_MENU;
        m_SwapIndex = -1; 
        return;
    }
}
void App::ProcessOverworldUpdateState() {
    // A. INTERACTION
    if (Util::Input::IsKeyDown(Util::Keycode::Z) && !m_Character->IsMoving()) {
        int checkX = m_Character->GetGridX();
        int checkY = m_Character->GetGridY();

        Character::Direction facing = m_Character->GetFacingDirection();
        if (facing == Character::Direction::UP)    checkY -= 1;
        if (facing == Character::Direction::DOWN)  checkY += 1;
        if (facing == Character::Direction::LEFT)  checkX -= 1;
        if (facing == Character::Direction::RIGHT) checkX += 1;

        HandleOverworldInteraction(checkX, checkY);
    }

    // B. PROCESSING MOVEMENT
    glm::vec2 movement = m_Character->Update(m_Map);
    m_Map->Move(-movement.x, -movement.y);

    // C. DOORS / WARPING
    if (m_Character->HasHitDoor()) {
        HandleOverworldWarping();
    }

    // D. RANDOM ENCOUNTERS
    HandleOverworldEncounters();
    m_Map->SetPlayerGridPosition(m_Character->GetGridX(), m_Character->GetGridY());
}

// ==========================================
// OVERWORLD UPDATE DELEGATES
// ==========================================

void App::HandleOverworldInteraction(int checkX, int checkY) {
    auto targetNPC = m_Map->GetNPCAt(checkX, checkY);
    
    if (!targetNPC) {
        int propID = m_Map->GetPropType(checkX, checkY); 
        if (propID == 24) {
            float extendedX = checkX;
            float extendedY = checkY;
            Character::Direction playerDir = m_Character->GetFacingDirection();
            
            if (playerDir == Character::Direction::UP)         extendedY -= 1.0f; 
            else if (playerDir == Character::Direction::DOWN)  extendedY += 1.0f;
            else if (playerDir == Character::Direction::LEFT)  extendedX -= 1.0f;
            else if (playerDir == Character::Direction::RIGHT) extendedX += 1.0f;

            targetNPC = m_Map->GetNPCAt(extendedX, extendedY);
        }
    }

    if (targetNPC) {
        m_ActiveNPC = targetNPC.get();;
        m_Character->StopMoving();
        m_CurrentState = State::DIALOGUE; 
        m_DialogueBoxUI->SetVisible(true);
        m_DialogueUI->SetVisible(true);

        Character::Direction playerDir = m_Character->GetFacingDirection();
        if (playerDir == Character::Direction::UP)         targetNPC->SetDirection(Character::Direction::DOWN);
        else if (playerDir == Character::Direction::DOWN)  targetNPC->SetDirection(Character::Direction::UP);
        else if (playerDir == Character::Direction::LEFT)  targetNPC->SetDirection(Character::Direction::RIGHT);
        else if (playerDir == Character::Direction::RIGHT) targetNPC->SetDirection(Character::Direction::LEFT);

        m_CurrentDialogueLines = targetNPC->Interact(*m_Character);
        m_CurrentDialogueIndex = 0; 

        if (!m_CurrentDialogueLines.empty()) {
            m_DialogueText->SetText(m_CurrentDialogueLines[m_CurrentDialogueIndex]);
            float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
            m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
        }
    }
    else {
        std::string collectedItem = m_Map->CollectItemAt(checkX, checkY, *m_Character);
        if (!collectedItem.empty()) {
            m_Character->StopMoving();
            m_CurrentState = State::DIALOGUE; 
            m_DialogueBoxUI->SetVisible(true);
            m_DialogueUI->SetVisible(true);
            
            m_CurrentDialogueLines = { "Obtained 1x " + collectedItem + "!" };
            m_CurrentDialogueIndex = 0;
            m_DialogueText->SetText(m_CurrentDialogueLines[m_CurrentDialogueIndex]);
            float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
            m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
        }
    }
}

void App::HandleOverworldWarping() {
    std::string doorKey = m_Map->GetCurrentLevelPath() + "_" + std::to_string(m_Character->GetGridX()) + "_" + std::to_string(m_Character->GetGridY());
    LOG_INFO("Door key: {}", doorKey);

    auto it = GameConfig::DoorRouting.find(doorKey);
    if (it == GameConfig::DoorRouting.end()) {
        LOG_WARN("No door routing for key: {}", doorKey);
        m_Character->ClearDoorFlag();
        return;
    }
    
    GameConfig::WarpDestination dest = it->second;
    
    if (dest.levelPath.find("GENERATED_CAVE") != std::string::npos) {
        auto generated = MapGenerator::GenerateCave(40, 40);
        std::string exitKey = "GENERATED_CAVE_" + std::to_string(generated.spawnX) + "_" + std::to_string(generated.spawnY);
        
        GameConfig::DoorRouting[exitKey] = { m_Map->GetCurrentLevelPath(), m_Character->GetGridX(), m_Character->GetGridY() };
        
        m_Map->LoadGeneratedLevel("GENERATED_CAVE", generated.ground, generated.props);
        m_Character->SetGridPosition(generated.spawnX, generated.spawnY);
        m_Map->WarpTo(generated.spawnX, generated.spawnY);
    } 
    else {
        m_Map->LoadLevel(dest.levelPath); 
        m_Character->SetGridPosition(dest.spawnX, dest.spawnY);
        m_Map->WarpTo(dest.spawnX, dest.spawnY);
        LOG_INFO("Warped to prop ID: {}", m_Map->GetPropType(dest.spawnX, dest.spawnY));
    }
    m_Character->StopMoving();
    m_Character->ClearDoorFlag(); 
}

void App::HandleOverworldEncounters() {
    static int lastGridX = -1;
    static int lastGridY = -1;
    int currentX = m_Character->GetGridX();
    int currentY = m_Character->GetGridY();

    if (!m_Character->IsMoving() && (currentX != lastGridX || currentY != lastGridY)) {
        lastGridX = currentX;
        lastGridY = currentY;

        int currentProp = m_Map->GetPropType(currentX, currentY);
        if (currentProp == GameConfig::PROP_TALLGRASS) {
            if (rand() % 100 < 10) { 
                m_CurrentState = State::BATTLE;
                m_Character->SetVisible(false);
                m_Map->SetVisible(false); 
                
                auto wildPokemon = GenerateWildPokemon(m_Map->GetCurrentLevelPath()); 
                m_BattleUI->Show(m_Character->GetParty(), wildPokemon);
            }
        } 
    }
}

// ==========================================
// SYSTEM SHORTCUT / MENU ASSISTANCE HELPERS
// ==========================================

void App::HandleGlobalShortcuts() {
    if (Util::Input::IsKeyDown(Util::Keycode::I)) {
        if (m_CurrentState == State::UPDATE) {
            OpenStartMenu();
        } else if (m_CurrentState == State::START_MENU) {
            CloseAllMenus();
        }
    }
    if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
        if (m_CurrentState == State::POKEMON_MENU || m_CurrentState == State::INVENTORY_MENU) {
            ReturnToStartMenu();
        }
    }
}

void App::OpenStartMenu() {
    m_StartMenu->SetPlayerInfo(m_Character->GetMoney(),
                               static_cast<int>(m_Character->GetParty().size()));
    m_StartMenu->SetVisible(true);
    m_CurrentState = State::START_MENU;
}

void App::CloseAllMenus() {
    m_StartMenu->SetVisible(false);
    if (m_PokemonMenu) m_PokemonMenu->Hide();
    if (m_InventoryMenu) m_InventoryMenu->Hide();
    m_CurrentState = State::UPDATE;
}

void App::ReturnToStartMenu() {
    if (m_PokemonMenu) m_PokemonMenu->Hide();
    if (m_InventoryMenu) m_InventoryMenu->Hide();
    m_StartMenu->SetVisible(true);
    m_CurrentState = State::START_MENU;
}

std::shared_ptr<Pokemon> App::GenerateWildPokemon(const std::string& mapPath) {
    std::string mapKey = mapPath;
    auto idx = mapPath.find("Resources/");
    if (idx != std::string::npos)
        mapKey = mapPath.substr(idx);

    auto it = RandomEncounters::GetMapEncounters().find(mapKey);
    if (it == RandomEncounters::GetMapEncounters().end()) {
        return PokemonDatabase::CreatePokemon("Rattata", 2);
    }

    const auto& encounterList = it->second;
    int totalWeight = 0;

    for (const auto& entry : encounterList) {
        totalWeight += entry.weight;
    }

    int roll = rand() % totalWeight;
    int currentWeight = 0;
    for (const auto& entry : encounterList) {
        currentWeight += entry.weight;
        if (roll < currentWeight) {
            int levelRange = (entry.maxLevel - entry.minLevel) + 1;
            int randomLevel = entry.minLevel + (rand() % levelRange);
            return PokemonDatabase::CreatePokemon(entry.speciesName, randomLevel);
        }
    }
    return PokemonDatabase::CreatePokemon("Rattata", 2);
}

void App::ProcessShopState() {
    ShopMenu::Result result = m_ShopMenu->Update();

    auto getProps = [](const std::string& name) -> const ItemProperties& {
        return ItemDatabase::GetProperties(name);
    };

    switch (result) {
        case ShopMenu::Result::BUY_ITEM: {
            std::string itemName = m_ShopMenu->GetSelectedItemName();
            const auto& props = getProps(itemName);

            auto it = std::find_if(m_CurrentShopData.items.begin(), m_CurrentShopData.items.end(),
                [&](const ShopItem& si) { return si.itemName == itemName; });
            int stock = (it != m_CurrentShopData.items.end()) ? it->quantity : -1;

            if (stock == 0) {
                LOG_INFO("Item out of stock!");
                break;
            }

            if (m_Character->SpendMoney(props.buyPrice)) {
                m_Character->AddItem(itemName, props.category, 1);

                if (it != m_CurrentShopData.items.end() && stock > 0)
                    it->quantity--;

                m_ShopMenu->SetBuyData(m_CurrentShopData.items, getProps);

                std::map<std::string, int> inv;
                for (const auto& [name, data] : m_Character->GetInventory())
                    inv[name] = data.quantity;
                m_ShopMenu->SetPlayerInventory(inv);

                m_ShopMenu->LoadBuyItems(m_CurrentShopData.items, getProps);
                m_ShopMenu->Show(ShopMenu::Mode::BUY, m_Character->GetMoney());
            } else {
                LOG_INFO("Not enough money!");
            }
            break;
        }

        case ShopMenu::Result::SELL_ITEM: {
            std::string itemName = m_ShopMenu->GetSelectedItemName();
            const auto& props = getProps(itemName);

            if (m_Character->GetItemCount(itemName) > 0) {
                m_Character->RemoveItem(itemName, 1);
                m_Character->AddMoney(props.sellPrice);

                std::map<std::string, int> inv;
                for (const auto& [name, data] : m_Character->GetInventory())
                    inv[name] = data.quantity;
                m_ShopMenu->SetPlayerInventory(inv);

                m_ShopMenu->LoadSellItems(inv, getProps);
                m_ShopMenu->Show(ShopMenu::Mode::SELL, m_Character->GetMoney());
            }
            break;
        }

        case ShopMenu::Result::BACK: {
            m_ShopMenu->Hide();
            m_CurrentState = State::UPDATE;
            break;
        }

        default:
            break;
    }
}

void App::ProcessDialogueState() {
    // --- NEW: Intercept for Starter Confirmation Overlay ---
    if (m_PendingStarterConfirm) {
        if (Util::Input::IsKeyDown(Util::Keycode::Z)) {
            // Player chose YES
            m_Character->AddPokemon(m_StarterToGive);
            GameFlags::Set("chosen_starter", true);
            LOG_INFO("Player selected starter: {}", m_StarterToGive->GetName());

            m_PokemonMenu->Hide(); // Hide the preview overlay
            m_PendingStarterConfirm = false;

            // Immediately show the success text
            m_CurrentDialogueLines = { "Received " + m_StarterToGive->GetName() + "! Take good care of it!" };
            m_CurrentDialogueIndex = 0;
            m_DialogueText->SetText(m_CurrentDialogueLines[0]);
            float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
            m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
            
            m_StarterToGive.reset();
        }
        else if (Util::Input::IsKeyDown(Util::Keycode::X) || Util::Input::IsKeyDown(Util::Keycode::ESCAPE)) {
            // Player chose NO
            m_PokemonMenu->Hide(); // Hide the preview overlay
            m_PendingStarterConfirm = false;

            // Immediately show the rejection text
            m_CurrentDialogueLines = { "You decided not to choose " + m_StarterToGive->GetName() + "." };
            m_CurrentDialogueIndex = 0;
            m_DialogueText->SetText(m_CurrentDialogueLines[0]);
            float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
            m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
            
            m_StarterToGive.reset();
        }
        return; // Block normal dialogue Z-advance while the prompt is active
    }

    if (!Util::Input::IsKeyDown(Util::Keycode::Z)) return;

    if (!m_CurrentDialogueLines.empty() &&
        m_CurrentDialogueIndex < m_CurrentDialogueLines.size() - 1) {
        m_CurrentDialogueIndex++;
        m_DialogueText->SetText(m_CurrentDialogueLines[m_CurrentDialogueIndex]);
        float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
        m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
        return;
    }

    m_DialogueBoxUI->SetVisible(false);
    m_DialogueUI->SetVisible(false);
    m_Map->SetPaused(false);

    if (!m_ActiveNPC) {
        m_CurrentState = State::UPDATE;
        return;
    }

    const NPCAction action      = m_ActiveNPC->GetActionType();
    const std::string data      = m_ActiveNPC->GetActionData();
    const std::string flag      = m_ActiveNPC->GetInteractFlag();
    const ItemCategory category = m_ActiveNPC->GetActionCategory();
    auto npcParty               = m_ActiveNPC->GetParty();

    std::string rewardItem;
    ItemCategory itemType;
    int rewardQty = 0;
    int rewardMoney = 0;
    if (action == NPCAction::BATTLE) {
        rewardItem = m_ActiveNPC->GetRewardItemName();
        rewardQty  = m_ActiveNPC->GetRewardQuantity();
        rewardMoney  = m_ActiveNPC->GetRewardMoney(); 
        itemType = m_ActiveNPC->GetRewardItemCategory(); 

    }

    std::vector<ShopItem> shopItems;
    if (action == NPCAction::SHOP)
        shopItems = m_ActiveNPC->GetShopItems();

    m_ActiveNPC->SetLocked(false);
    m_ActiveNPC = nullptr;

    switch (action) {

        case NPCAction::HEAL: {
            for (auto& pokemon : m_Character->GetParty())
                if (pokemon) pokemon->SetCurrentHP(pokemon->GetMaxHP());
            m_LastHealMapPath = m_Map->GetCurrentLevelPath();
            m_LastHealX       = m_Character->GetGridX();
            m_LastHealY       = m_Character->GetGridY();
            m_HasHealLocation = true;
            if (!flag.empty()) GameFlags::Set(flag, true);
            m_CurrentState = State::UPDATE;
            break;
        }

        case NPCAction::SHOP: {
            if (!shopItems.empty()) {
                m_CurrentShopData.items = shopItems;
                m_CurrentShopData.shopName = "Shop";
                auto getProps = [](const std::string& name) -> const ItemProperties& {
                    return ItemDatabase::GetProperties(name);
                };
                m_ShopMenu->LoadBuyItems(m_CurrentShopData.items, getProps);
                m_ShopMenu->Show(ShopMenu::Mode::BUY, m_Character->GetMoney());
                m_CurrentState = State::SHOP;
            } else {
                LOG_WARN("SHOP NPC had no shopItems – shop not opened.");
                m_CurrentState = State::UPDATE;
            }
            break;
        }

        case NPCAction::GIVE_ITEM: {
            if (!data.empty()) {
                m_Character->AddItem(data, category, 1);
                LOG_INFO("Player received: {} (qty: 1)", data);
            }
            if (!flag.empty()) GameFlags::Set(flag, true);
            m_CurrentState = State::UPDATE;
            break;
        }

        case NPCAction::BATTLE: {
            m_PendingRewardItem   = rewardItem;
            m_PendingRewardQty    = rewardQty;
            m_PendingRewardMoney = rewardMoney;
            m_PendingBattleFlag   = flag;
            m_PendingRewardCategory = itemType;

            m_Character->SetVisible(false);
            m_Map->SetVisible(false);
            m_BattleUI->StartTrainerBattle(m_Character->GetParty(), npcParty, flag);

            m_CurrentState = State::BATTLE;
            break;
        }

        case NPCAction::CHECK_ITEM: {
            if (!data.empty() && m_Character->GetItemCount(data) > 0) {
                if (!flag.empty()) GameFlags::Set(flag, true);
            }
            m_CurrentState = State::UPDATE;
            break;
        }
        case NPCAction::WARP: {
            std::istringstream iss(data);
            std::string mapFile;
            int wx, wy;
            if (iss >> mapFile >> wx >> wy) {
                std::string fullPath = MAP_DIR + mapFile;
                m_Map->LoadLevel(fullPath);
                m_Map->LoadConnections(RESOURCE_DIR "/maps/connections.txt");
                m_Character->SetGridPosition(wx, wy);
                m_Map->WarpTo(wx, wy);
            }
            m_Character->StopMoving();
            m_Character->ClearDoorFlag();
            m_CurrentState = State::UPDATE;
            break;
        }

        case NPCAction::SELECT_STARTER: {
            if (!data.empty() && !GameFlags::Get("chosen_starter")) {
                std::shared_ptr<Pokemon> preview = nullptr;
                if (data == "Bulbasaur") {
                    preview = std::make_shared<Pokemon>("Bulbasaur", 5, PokemonType::GRASS, PokemonType::POISON, 45, 49, 49, 65, 65, 45, 45);
                    preview->LearnMove("Tackle");
                    preview->LearnMove("Growl");
                } else if (data == "Charmander") {
                    preview = std::make_shared<Pokemon>("Charmander", 5, PokemonType::FIRE, PokemonType::NONE, 39, 52, 43, 60, 50, 65, 45);
                    preview->LearnMove("Scratch");
                    preview->LearnMove("Growl");
                } else if (data == "Squirtle") {
                    preview = std::make_shared<Pokemon>("Squirtle", 5, PokemonType::WATER, PokemonType::NONE, 44, 48, 65, 50, 64, 43, 43);
                    preview->LearnMove("Tackle");
                    preview->LearnMove("Tail Whip");
                }

                if (preview) {
                    m_StarterToGive = preview;
                    m_PendingStarterConfirm = true; // Flag tells ProcessDialogueState to wait for Z/X
                    
                    // Show the visual overlay
                    m_PokemonMenu->ShowPreview(*preview); 

                    // Alter the text box to prompt the user
                    m_CurrentDialogueLines = { "Do you want to choose " + preview->GetName() + "? [Z] Yes  [X] No" };
                    m_CurrentDialogueIndex = 0;
                    m_DialogueText->SetText(m_CurrentDialogueLines[0]);
                    float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
                    m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
                    
                    // Ensure dialogue UI stays fully visible
                    m_DialogueBoxUI->SetVisible(true);
                    m_DialogueUI->SetVisible(true);

                    // Force state back to DIALOGUE so the interception block catches the very next frame
                    m_CurrentState = State::DIALOGUE;
                } else {
                    LOG_ERROR("Unknown starter species: {}", data);
                    m_CurrentState = State::UPDATE;
                }
            } else {
                m_CurrentState = State::UPDATE;
            }
            break;
        }

        case NPCAction::BUY_POKEMON: {
            std::vector<std::string> feedback;
            std::string species = "Gengar";
            int price = 3000;
            std::string minBall = "Greatball";

            bool hasGengar = false;
            bool hasCharizard = false;
            for (auto& p : m_Character->GetParty()) {
                if (p) {
                    if (p->GetName() == "Gengar") hasGengar = true;
                    if (p->GetName() == "Charizard") hasCharizard = true;
                }
            }

            if (!hasGengar) {
                species = "Gengar";
                price = 3000;
                minBall = "Greatball";
            } else if (!hasCharizard) {
                species = "Charizard";
                price = 10000;
                minBall = "Masterball";
            } else {
                feedback = { "I have no more powerful Pokemon to sell." };
                goto show_feedback;
            }

            {
                auto getBallRank = [](const std::string& name) {
                    if (name == "Pokeball") return 1;
                    if (name == "Greatball") return 2;
                    if (name == "Ultraball") return 3;
                    if (name == "Masterball") return 4;
                    return 0;
                };

                int requiredRank = getBallRank(minBall);
                std::string bestBall = "";
                int bestRank = 100;

                for (const auto& pair : m_Character->GetInventory()) {
                    const std::string& name = pair.first;
                    const auto& inv = pair.second;
                    if (inv.category == ItemCategory::POKEBALLS) {
                        int r = getBallRank(name);
                        if (r >= requiredRank && r < bestRank) {
                            bestRank = r;
                            bestBall = name;
                        }
                    }
                }

                if (bestBall.empty()) {
                    feedback = { "Go get the right ball!", "That " + species + " won't fit in what you have." };
                } else if (m_Character->GetMoney() < price) {
                    feedback = { "You don't have enough money!", "That " + species + " costs " + std::to_string(price) + "." };
                } else {
                    m_Character->SpendMoney(price);
                    m_Character->RemoveItem(bestBall, 1);
                    auto pkmn = PokemonDatabase::CreatePokemon(species, 50);
                    if (m_Character->AddPokemon(pkmn)) {
                        feedback = { "Transaction complete!", "Received " + species + "! It was placed in your party." };
                    } else {
                        feedback = { "Your party is full! Come back later." };
                        m_Character->AddMoney(price);
                        m_Character->AddItem(bestBall, ItemCategory::POKEBALLS, 1);
                    }
                }
            }

        show_feedback:
            m_CurrentState = State::DIALOGUE;
            m_DialogueBoxUI->SetVisible(true);
            m_DialogueUI->SetVisible(true);
            m_CurrentDialogueLines = feedback;
            m_CurrentDialogueIndex = 0;
            m_DialogueText->SetText(m_CurrentDialogueLines[0]);
            float textHalfWidth = m_DialogueText->GetSize().x / 2.0f;
            m_DialogueUI->m_Transform.translation.x = -600.0f + textHalfWidth;
            break;
        }

        default: {
            m_CurrentState = State::UPDATE;
            break;
        }
    }
}


void App::ProcessTitleState() {
    m_TitleScreen->Update();

    if (!m_TitleScreen->IsConfirmed()) return;

    int slot = m_TitleScreen->GetSelectedSlot();
    // slot == -1  →  New Game
    // slot >= 0   →  Load slot N

    m_TitleScreen->Hide();
    InitGameLoad(slot);     // see section (c) below
    m_CurrentState = State::UPDATE;
}