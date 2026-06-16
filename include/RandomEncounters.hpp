#ifndef RandomEncounters_HPP
#define RandomEncounters_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include "GameConfig.hpp"            // for RESOURCE_DIR (and DATA_DIR)
#include "nlohmann/json.hpp"        // JSON library

namespace RandomEncounters {

    struct WildEncounterEntry {
        std::string speciesName;
        int minLevel;
        int maxLevel;
        int weight;   // higher = more common
    };

    // Loads the encounter table from a JSON file once, returns a static reference.
    inline const std::unordered_map<std::string, std::vector<WildEncounterEntry>>&
    GetMapEncounters() {
        static std::unordered_map<std::string, std::vector<WildEncounterEntry>> map;
        static bool loaded = false;
        if (!loaded) {
            loaded = true;

            // Build path to the JSON file (same location as tiles.json, props.json, etc.)
            const std::string path = std::string(RESOURCE_DIR) + "/data/encounters.json";

            std::ifstream file(path);
            if (!file.is_open()) {
                std::cerr << "ERROR: Could not open encounter file: " << path << "\n";
                return map;
            }

            nlohmann::json root;
            try {
                file >> root;
            } catch (const nlohmann::json::parse_error& e) {
                std::cerr << "ERROR: JSON parse error in " << path << ": " << e.what() << "\n";
                return map;
            }

            if (!root.contains("encounters") || !root["encounters"].is_object()) {
                std::cerr << "ERROR: JSON missing 'encounters' object in " << path << "\n";
                return map;
            }

            
            for (const auto& [mapName, encounterList] : root["encounters"].items()) {
                if (!encounterList.is_array()) {
                    std::cerr << "WARNING: Encounters for map '" << mapName
                              << "' is not an array – skipping\n";
                    continue;
                }

                std::vector<WildEncounterEntry> entries;
                for (const auto& entry : encounterList) {
                    if (!entry.contains("species") || !entry.contains("weight")) {
                        std::cerr << "WARNING: Skipping invalid encounter entry\n";
                        continue;
                    }

                    WildEncounterEntry e;
                    e.speciesName = entry["species"].get<std::string>();
                    e.minLevel    = entry.value("minLevel", 2);
                    e.maxLevel    = entry.value("maxLevel", 5);
                    e.weight      = entry["weight"].get<int>();

                    entries.push_back(std::move(e));
                }
                map[mapName] = std::move(entries);
            }

            std::cout << "Loaded " << map.size() << " encounter tables from " << path << "\n";
        }
        return map;
    }

} // namespace RandomEncounters

#endif