#pragma once
#include "GameFlags.hpp"
#include "Item.hpp"
#include "Pokemon.hpp" 
#include "Util/Logger.hpp"
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace SaveSystem {

    const std::string SAVE_PATH = "savegame.json";

    struct GameState {
        std::string mapPath;
        int gridX;
        int gridY;
        int direction;
        int money = 0;  
        std::unordered_map<std::string, InventoryData> inventory;
        std::unordered_set<std::string> lootedItems;
        std::vector<std::shared_ptr<Pokemon>> party;

        // Last healing location (Pokémon Center etc.)
        std::string lastHealMapPath;   // empty = never healed
        int lastHealX = -1;
        int lastHealY = -1; 
    };

    // ------------------------------------------------------------------
    // Helper: convert an absolute map path to one relative to RESOURCE_DIR
    // ------------------------------------------------------------------
    inline std::string ToRelativeMapPath(const std::string& fullPath) {
        // Empty path stays empty
        if (fullPath.empty()) return fullPath;

        static const std::string resDir(RESOURCE_DIR);
        if (fullPath.rfind(resDir, 0) == 0) {
            std::string rel = fullPath.substr(resDir.length());
            while (!rel.empty() && (rel[0] == '/' || rel[0] == '\\')) {
                rel.erase(0, 1);
            }
            return rel;
        }
        return fullPath;
    }

    // ------------------------------------------------------------------
    // Helper: convert a stored relative path back to an absolute one
    // ------------------------------------------------------------------
    inline std::string ToAbsoluteMapPath(const std::string& storedPath) {
        // Empty path stays empty (never healed)
        if (storedPath.empty()) return storedPath;

        // Already absolute?
        if (!storedPath.empty() &&
            (storedPath[0] == '/' || storedPath[0] == '\\' ||
             (storedPath.size() >= 2 && storedPath[1] == ':'))) {
            return storedPath;
        }
        // Generated cave names – no physical file, keep unchanged
        if (storedPath.find("GENERATED_CAVE") == 0) {
            return storedPath;
        }
        // Prepend the current machine’s resource directory
        return std::string(RESOURCE_DIR) + "/" + storedPath;
    }

    // ------------------------------------------------------------------
    // SaveGame – stores all fields, including last heal location
    // ------------------------------------------------------------------
    inline void SaveGame(const GameState& state) {
        json j;

        // 1. Core Player Data
        j["mapPath"]  = ToRelativeMapPath(state.mapPath);
        j["gridX"]    = state.gridX;
        j["gridY"]    = state.gridY;
        j["direction"]= state.direction;
        j["money"]    = state.money;

        // 2. Last Healing Location (persist relative map path)
        j["lastHealMapPath"] = ToRelativeMapPath(state.lastHealMapPath);
        j["lastHealX"]       = state.lastHealX;
        j["lastHealY"]       = state.lastHealY;

        // 3. Flags
        j["flags"] = json::object();
        for (const auto& [name, value] : GameFlags::Flags) {
            j["flags"][name] = value;
        }

        // 4. Looted Items
        j["lootedItems"] = state.lootedItems;

        // 5. Inventory
        j["inventory"] = json::object();
        for (const auto& [itemName, data] : state.inventory) {
            j["inventory"][itemName] = {
                {"quantity", data.quantity},
                {"category", static_cast<int>(data.category)}
            };
        }

        // 6. Pokémon Party
        j["party"] = json::array();
        for (const auto& p : state.party) {
            json pkmnJson;
            pkmnJson["name"]        = p->GetName();
            pkmnJson["level"]       = p->GetLevel();
            pkmnJson["type1"]       = static_cast<int>(p->GetType1());
            pkmnJson["type2"]       = static_cast<int>(p->GetType2());
            pkmnJson["maxHp"]       = p->GetMaxHP();
            pkmnJson["currentHp"]   = p->GetCurrentHP();
            pkmnJson["atk"]         = p->GetAttack();
            pkmnJson["def"]         = p->GetDefense();
            pkmnJson["spa"]         = p->GetSpecialAttack();
            pkmnJson["spd"]         = p->GetSpecialDefense();
            pkmnJson["spe"]         = p->GetSpeed();
            pkmnJson["exp"]         = p->GetCurrentExp();
            pkmnJson["catchRate"]   = p->GetCatchRate();
            pkmnJson["moves"]       = p->GetMoves();
            j["party"].push_back(pkmnJson);
        }

        // Write to disk
        std::ofstream outFile(SAVE_PATH);
        if (outFile.is_open()) {
            outFile << j.dump(4);
            outFile.close();
            LOG_INFO("Game Saved as JSON: Map={}, Pos={},{}  HealSpot={}",
                     ToRelativeMapPath(state.mapPath), state.gridX, state.gridY,
                     ToRelativeMapPath(state.lastHealMapPath));
        }
    }

    // ------------------------------------------------------------------
    // LoadGame – restores all fields, including last heal location
    // ------------------------------------------------------------------
    inline bool LoadGame(GameState& outState) {
        if (!std::filesystem::exists(SAVE_PATH)) return false;

        std::ifstream inFile(SAVE_PATH);
        if (!inFile.is_open()) return false;

        json j;
        inFile >> j;

        // 1. Core Player Data
        outState.mapPath   = ToAbsoluteMapPath(j.value("mapPath", ""));
        outState.gridX     = j.value("gridX", 0);
        outState.gridY     = j.value("gridY", 0);
        outState.direction = j.value("direction", 0);
        outState.money     = j.value("money", 0);

        // 2. Last Healing Location
        outState.lastHealMapPath = ToAbsoluteMapPath(
            j.value("lastHealMapPath", "")
        );
        outState.lastHealX = j.value("lastHealX", -1);
        outState.lastHealY = j.value("lastHealY", -1);

        // 3. Flags
        if (j.contains("flags")) {
            for (auto& [key, value] : j["flags"].items()) {
                GameFlags::Set(key, value.get<bool>());
            }
        }

        // 4. Looted Items
        if (j.contains("lootedItems")) {
            outState.lootedItems = j["lootedItems"]
                .get<std::unordered_set<std::string>>();
        }

        // 5. Inventory
        if (j.contains("inventory")) {
            for (auto& [key, value] : j["inventory"].items()) {
                outState.inventory[key].quantity = value["quantity"].get<int>();
                outState.inventory[key].category = static_cast<ItemCategory>(
                    value["category"].get<int>()
                );
            }
        }

        // 6. Pokémon Party
        if (j.contains("party")) {
            for (const auto& pkmnJson : j["party"]) {
                std::string name = pkmnJson.value("name", "Unknown");
                int lvl = pkmnJson.value("level", 1);
                auto t1 = static_cast<PokemonType>(pkmnJson.value("type1", 0));
                auto t2 = static_cast<PokemonType>(pkmnJson.value("type2", 0));
                int mhp = pkmnJson.value("maxHp", 10);
                int atk = pkmnJson.value("atk", 5);
                int def = pkmnJson.value("def", 5);
                int spa = pkmnJson.value("spa", 5);
                int spd = pkmnJson.value("spd", 5);
                int spe = pkmnJson.value("spe", 5);
                int catchRate = pkmnJson.value("catchRate", 45);

                auto pkmn = std::make_shared<Pokemon>(
                    name, lvl, t1, t2, mhp, atk, def, spa, spd, spe, catchRate
                );
                pkmn->SetCurrentHP(pkmnJson.value("currentHp", mhp));
                pkmn->SetCurrentExp(pkmnJson.value("exp", 0));

                if (pkmnJson.contains("moves")) {
                    for (const auto& move : pkmnJson["moves"]) {
                        pkmn->LearnMove(move.get<std::string>());
                    }
                }
                outState.party.push_back(pkmn);
            }
        }

        return true;
    }

} // namespace SaveSystem