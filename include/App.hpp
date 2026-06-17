#ifndef APP_HPP
#define APP_HPP

#include "pch.hpp"
#include "Player.hpp"
#include "StartMenu.hpp"
#include "BattleUI.hpp"
#include "InventoryMenu.hpp"
#include "PokemonMenu.hpp"
#include "Map.hpp"
#include "GameConfig.hpp"
#include "util/Text.hpp"
#include "ShopMenu.hpp"
#include "ShopData.hpp"
#include "TrainerDatabase.hpp"
#include "MoveDatabase.hpp"
#include "TitleScreen.hpp"
#include "Item.hpp"
#include "NPC.hpp"

struct InteractionResult {
    std::vector<std::string> dialogueLines; 
    std::string specialAction;              
};

class App {
public:
    enum class State {
        START,
        UPDATE,
        TITLE,
        DIALOGUE,         
        START_MENU,       
        POKEMON_MENU,     
        INVENTORY_MENU,
        BATTLE,
        SHOP,
        END
    };

    State GetCurrentState() const { return m_CurrentState; }
    
    std::shared_ptr<Util::GameObject> m_StartMenuBoxUI;
    std::shared_ptr<Util::GameObject> m_StartMenuCursorUI;
    std::shared_ptr<Util::GameObject> m_StartMenuTextUI;

    void Start();
    void Update();
    void ProcessTitleState();
    void End();

    std::vector<std::string> m_CurrentDialogueLines;
    size_t m_CurrentDialogueIndex = 0;
    std::shared_ptr<StartMenu> m_StartMenu;
    bool JustFinishedMoving() const { return m_JustFinishedMoving; }
    bool IsUsableOverworld(const std::string& itemName) {
    static const std::unordered_set<std::string> usable = {
        "Potion", "Potion1", "Super Potion", "Hyper Potion", "Max Potion",
        "Antidote", "Paralyze Heal", "Burn Heal", "Ice Heal", "Awakening"
    };
    return usable.count(itemName) > 0;
    }

private:
    void ValidTask();
    int m_SwapIndex = -1;
    
    std::shared_ptr<ShopMenu> m_ShopMenu;
    ShopData m_CurrentShopData;   
    std::shared_ptr<Util::Renderer> m_Renderer;
    std::shared_ptr<Map> m_Map;
    std::shared_ptr<Util::GameObject> m_DialogueBoxUI;
    
    State m_CurrentState = State::START;
    bool m_JustFinishedMoving = false;
    std::shared_ptr<Player> m_Character;
    //std::shared_ptr<NPC> m_ActiveNPC = nullptr;
    NPC* m_ActiveNPC = nullptr;
    // --- Your UI components ---
    std::shared_ptr<Util::GameObject> m_DialogueUI;
    std::shared_ptr<Util::Text> m_DialogueText;
    
    // Submenus
    std::shared_ptr<InventoryMenu> m_InventoryMenu;
    std::shared_ptr<PokemonMenu> m_PokemonMenu;
    std::shared_ptr<BattleUI> m_BattleUI;
    
    std::string m_PendingRewardItem;
    int         m_PendingRewardQty   = 0;
    int         m_PendingRewardMoney = 0;
    std::string m_PendingBattleFlag;
    ItemCategory m_PendingRewardCategory = ItemCategory::GENERAL;
    // NEW: Handles the in-dialogue starter selection
    bool m_PendingStarterConfirm = false;
    std::shared_ptr<Pokemon> m_StarterToGive;
    std::string m_PendingStarterSpecies;

    std::string m_LastHealMapPath;
    int         m_LastHealX = -1;
    int         m_LastHealY = -1;
    bool        m_HasHealLocation = false;
    bool m_IsItemTargeting = false;
    std::string m_TargetItemName;
    std::shared_ptr<TitleScreen> m_TitleScreen;
    int m_ActiveSaveSlot = -1;


    // ==========================================
    // RESTRUCTURING HELPER FUNCTIONS
    // ==========================================
    void InitSystems();
    void InitGameLoad(int slot = -1);
    void InitUI();
    void PerformQuickSave();

    // State Processing Delegates
    void ProcessDialogueState();
    void ProcessStartMenuState();
    void ProcessPokemonMenuState();
    void ProcessOverworldUpdateState();
    void ProcessBattleState();
    void ProcessShopState();

    // Overworld sub-helpers
    void HandleOverworldInteraction(int checkX, int checkY);
    void HandleOverworldWarping();
    void HandleOverworldEncounters();

    std::shared_ptr<Pokemon> GenerateWildPokemon(const std::string& mapPath);
    
    void HandleGlobalShortcuts();
    void OpenStartMenu();
    void CloseAllMenus();
    void ReturnToStartMenu();
};

#endif