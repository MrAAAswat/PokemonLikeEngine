#pragma once
#include "Pokemon.hpp"
#include "Character.hpp"
#include <memory>
#include <string>
#include <vector>
#include <queue>

class BattleManager {
public:
    enum class BattleState {
        SELECTING_ACTION,
        SELECTING_MOVE,
        EXECUTING_PLAYER_TURN,
        EXECUTING_ENEMY_TURN,
        SHOWING_MESSAGE,
        BATTLE_WON,
        BATTLE_LOST,
        CATCHING,
        BATTLE_ESCAPED
    };

    enum class Action { FIGHT, BAG, POKEMON, RUN };

    struct TurnResult {
        std::string message;
        bool playerFainted;
        bool enemyFainted;
        int expGained;
    };

    BattleManager(std::shared_ptr<Pokemon> playerPokemon,
                  std::shared_ptr<Pokemon> enemyPokemon,
                  bool isWildBattle);

    // State queries
    BattleState GetState() const { return m_State; }
    void SetState(BattleState state) { m_State = state; }
    void SetPlayerPokemon(std::shared_ptr<Pokemon> newPokemon) {
        m_PlayerPokemon = newPokemon;
    }

    std::shared_ptr<Pokemon> GetPlayerPokemon() { return m_PlayerPokemon; }
    std::shared_ptr<Pokemon> GetEnemyPokemon()  { return m_EnemyPokemon; }

    // Player input handlers
    TurnResult SelectAction(Action action);
    TurnResult SelectMove(int moveIndex);
    TurnResult ThrowBall();

    // Item and catch logic
    void UseItem(std::shared_ptr<Character> player,
                 std::shared_ptr<Pokemon> target,
                 const std::string& itemName);
    int CalculateCatchRate();
    bool TryCatchPokemon(std::shared_ptr<Pokemon> target, float ballMultiplier);

    // Enemy turn
    TurnResult ExecuteEnemyMove();
    TurnResult ProcessEnemyTurn();
    std::string GetLastEnemyMove() const { return m_LastEnemyMove; }

    // Battle end checks
    bool IsFinished() const {
        return m_State == BattleState::BATTLE_WON ||
               m_State == BattleState::BATTLE_LOST;
    }
    bool PlayerWon() const {
        return m_State == BattleState::BATTLE_WON;
    }

    // Message queue for UI
    std::queue<std::string> m_MessageQueue;
    void ClearMessageQueue() {
        while (!m_MessageQueue.empty()) m_MessageQueue.pop();
    }

    // Switch Pokémon
    void SwitchPlayerPokemon(int partyIndex, std::shared_ptr<Character> player);

private:
    bool m_IsWildBattle;
    std::string m_LastEnemyMove;
    std::shared_ptr<Pokemon> m_PlayerPokemon;
    std::shared_ptr<Pokemon> m_EnemyPokemon;
    BattleState m_State = BattleState::SELECTING_ACTION;

    TurnResult ExecutePlayerMove(int moveIndex);
    int CalculateDamage(Pokemon* attacker, Pokemon* defender,
                        const std::string& moveName);
};