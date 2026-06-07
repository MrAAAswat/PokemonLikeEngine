#pragma once
#include "pch.hpp"
#include "Pokemon.hpp"
#include "GameFlags.hpp"
#include "Player.hpp"
#include "BattleAnimator.hpp"
#include "BattleManager.hpp"
#include "InventoryMenu.hpp"
#include "PokemonMenu.hpp"
#include "Util/GameObject.hpp"
#include "PokeballAnimator.hpp"
#include "Util/Renderer.hpp"
#include "ResourceManager.hpp"
#include "Util/Text.hpp"
#include "Util/AssetStore.hpp"
#include "AnimationPlayer.hpp"
#include "BattleAnimation.hpp"
#include "MoveDatabase.hpp"

#include <memory>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <queue>
#include <sstream>

class BattleAnimator;

class BattleUI {
public:
    BattleUI(std::shared_ptr<Util::Renderer> renderer);

    void SetInventoryMenu(std::shared_ptr<InventoryMenu> invMenu) { m_InventoryMenu = invMenu; }
    void SetPlayer(std::shared_ptr<Player> player)                { m_Player = player; }
    void SetAnimationLibrary(std::shared_ptr<AnimationLibrary> animLib) { m_AnimLibrary = animLib; }

    void Show(std::vector<std::shared_ptr<Pokemon>> playerParty,
          std::shared_ptr<Pokemon> wildPokemon,
          const std::string& battleFlag = "");

    void StartTrainerBattle(std::vector<std::shared_ptr<Pokemon>> playerParty,
                        std::vector<std::shared_ptr<Pokemon>> enemyParty,
                        const std::string& battleFlag = "");
    void Hide();
    bool Update();

  

    // ── Battle state queries ───────────────────────────────────────────────
    bool IsBattleOver()   const { return m_BattleOver; }

    // Set at the moment Hide() is called — read these in ProcessBattleEnd()
    // to decide whether to award the flag and reward.
    bool PlayerWon()      const { return m_PlayerWon; }
    bool PlayerLost()     const { return m_PlayerLost; }
    bool PlayerEscaped()  const { return m_EscapeSuccessful && !m_PlayerWon && !m_PlayerLost; }

private:
    enum class UIState {
        ANIMATING,
        MAIN_MENU,
        MOVE_MENU,
        WAITING_TEXT,
        POKEMON_MENU,
        BAG_MENU,
        CATCH_ANIMATION
    };

    // ── Pokemon references ────────────────────────────────────────────────
    std::shared_ptr<Pokemon> m_PlayerPokemon;
    std::shared_ptr<Pokemon> m_EnemyPokemon;

    // ── Battle outcome flags ──────────────────────────────────────────────
    // Reset to false in Show(). Set immediately before Hide() at each exit.
    bool m_BattleOver    = false;
    bool m_PlayerWon     = false;   // true only on genuine player victory
    bool m_PlayerLost    = false;   // true only when entire party faints
    bool m_EscapeSuccessful = false;
    std::string m_BattleFlag;

    // ── UI state machine ──────────────────────────────────────────────────
    UIState m_UIState   = UIState::ANIMATING;
    int     m_CursorIndex = 0;

    void UpdateCursorPosition();
    void UpdateMenuVisibility();
    void SetDialogue(const std::string& text);
    void HandleEnemyFaint();
    void AttemptRun();
    void ProcessNextMessage();

    // ── External references ───────────────────────────────────────────────
    std::shared_ptr<Util::Renderer>   m_Renderer;
    std::unique_ptr<BattleManager>    m_BattleLogic;
    std::shared_ptr<InventoryMenu>    m_InventoryMenu;
    std::shared_ptr<Player>           m_Player;
    std::shared_ptr<PokemonMenu>      m_PokemonMenu;
    std::unique_ptr<BattleAnimator>   m_Animator;
    std::shared_ptr<AnimationLibrary> m_AnimLibrary;

    // ── Trainer battle ────────────────────────────────────────────────────
    std::vector<std::shared_ptr<Pokemon>> m_EnemyTeam;
    int  m_CurrentEnemyIndex = 0;
    bool m_IsTrainerBattle   = false;
    bool m_IsVisible         = false;

    // ── Scene objects ─────────────────────────────────────────────────────
    std::shared_ptr<Util::GameObject> m_Background;
    std::shared_ptr<Util::GameObject> m_PlayerBase;
    std::shared_ptr<Util::GameObject> m_EnemyBase;
    std::shared_ptr<Util::GameObject> m_PlayerSprite;
    std::shared_ptr<Util::GameObject> m_EnemySprite;

    // ── Panels & bars ─────────────────────────────────────────────────────
    std::shared_ptr<Util::GameObject> m_PlayerPanel;
    std::shared_ptr<Util::GameObject> m_EnemyPanel;
    std::shared_ptr<Util::GameObject> m_PlayerHPBar;
    std::shared_ptr<Util::GameObject> m_EnemyHPBar;
    std::shared_ptr<Util::GameObject> m_PlayerEXPBar;
    std::shared_ptr<Util::GameObject> m_PlayerHPTextObj;
    std::shared_ptr<Util::Text>       m_PlayerHPText;
    std::shared_ptr<Util::Text>       m_PlayerNameText;
    std::shared_ptr<Util::Text>       m_EnemyNameText;

    // ── Level text ────────────────────────────────────────────────────────
    std::shared_ptr<Util::GameObject> m_PlayerLevelText;
    std::shared_ptr<Util::GameObject> m_EnemyLevelText;
    std::shared_ptr<Util::Text>       m_PlayerLevelTextDrawable;
    std::shared_ptr<Util::Text>       m_EnemyLevelTextDrawable;

    // ── Dialogue & menus ──────────────────────────────────────────────────
    std::shared_ptr<Util::GameObject> m_DialogueBox;
    std::shared_ptr<Util::Text>       m_DialogueText;
    std::shared_ptr<Util::GameObject> m_DialogueTextObj;
    std::shared_ptr<Util::GameObject> m_CommandBox;
    std::shared_ptr<Util::GameObject> m_MenuCursor;
    std::shared_ptr<Util::GameObject> m_MoveBox;
    std::shared_ptr<Util::Text>       m_MoveTexts[4];
    std::shared_ptr<Util::GameObject> m_MoveTextObjs[4];

    // ── Intro animation ───────────────────────────────────────────────────
    std::shared_ptr<Util::Image> m_EnemyFrame1;
    std::shared_ptr<Util::Image> m_EnemyFrame2;
    std::shared_ptr<Util::Image> m_PlayerFrame1;
    std::shared_ptr<Util::Image> m_PlayerFrame2;

    int  m_IntroAnimTimer   = 0;
    bool m_IsIntroAnimating = false;
    bool m_IsSlidingIn      = false;
    int  m_SlideTimer       = 0;

    // ── Timers & state ────────────────────────────────────────────────────
    int  m_CommandCursorIndex = 0;
    int  m_TextWaitTimer      = 0;
    int  m_AnimTime           = 0;
    int  m_PlayerLungeTimer   = 0;
    int  m_EnemyShakeTimer    = 0;
    int  m_EnemyLungeTimer    = 0;
    int  m_PlayerShakeTimer   = 0;

    // ── HP display ────────────────────────────────────────────────────────
    float m_DisplayPlayerHPPercent  = 1.0f;
    float m_DisplayEnemyHPPercent   = 1.0f;
    float m_DisplayPlayerEXPPercent = 0.0f;
    bool  m_AllowEXPAnimation       = false;
    int   m_TargetPlayerHP          = 0;
    int   m_TargetEnemyHP           = 0;
    float m_VisualPlayerHP          = 0.0f;
    float m_VisualEnemyHP           = 0.0f;

    // ── Dialogue queue ────────────────────────────────────────────────────
    std::queue<std::string>     m_DialogueQueue;
    BattleManager::TurnResult   m_LastResult;

    // ── Animation system ──────────────────────────────────────────────────
    bool m_IsMoveAnimating     = false;
    int  m_MoveAnimatingTimeout = 0;

    std::shared_ptr<AnimationPlayer> m_AnimPlayer;

    Util::AssetStore<std::shared_ptr<Util::Image>> m_SheetCache{
        [](const std::string& path) {
            return std::make_shared<Util::Image>(path);
        }
    };

    // ── Pokeball catch ────────────────────────────────────────────────────
    std::shared_ptr<Util::GameObject> m_PokeballSprite;
    std::shared_ptr<PokeballAnimator> m_PokeballAnimator;
    int   m_CatchPhaseTimer  = 0;
    int   m_CatchShakes      = 0;
    int   m_TargetShakes     = 3;
    bool  m_CatchWillSucceed = false;
    glm::vec2 m_ThrowStart   = {-270.0f, -50.0f};
    glm::vec2 m_ThrowEnd     = { 400.0f, 150.0f};

    void UpdateBar(std::shared_ptr<Util::GameObject> bar,
                   float percent, float leftEdgeX, float fixedY,
                   float maxScale, bool isHPBar);
};