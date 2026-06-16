#include "ItemDatabase.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include "Util/Logger.hpp"
#include "Map.hpp"              // for Map::StringToCategory
#include "GameConfig.hpp"       // if PROP_DIR is defined there; if not, define it yourself

using json = nlohmann::json;    // <-- allow bare 'json'

// If PROP_DIR is not defined anywhere, define it here.
// It should match the folder where item textures are stored.
#ifndef PROP_DIR
#define PROP_DIR std::string(RESOURCE_DIR) + "/items/"
#endif

void ItemDatabase::Load(const std::string& jsonPath) {
    std::ifstream file(jsonPath);
    if (!file.is_open()) {
        LOG_ERROR("ItemDatabase: could not open '{}'", jsonPath);
        return;
    }

    json root;
    file >> root;

    for (const auto& entry : root["items"]) {
        ItemProperties props;
        props.texturePath = PROP_DIR + entry["texture"].get<std::string>();
        props.shopTexturePath = PROP_DIR + entry.value("shopTexture", ""); 
        props.name        = entry["name"].get<std::string>();
        props.category    = Map::StringToCategory(entry.value("category", "GENERAL"));
        props.zIndex      = entry.value("zIndex", 0.5f);
        props.buyPrice    = entry.value("buyPrice", 0);
        props.sellPrice   = entry.value("sellPrice", 0);

        s_Database[props.name] = props;
    }
    LOG_INFO("ItemDatabase loaded {} items", s_Database.size());
}

const ItemProperties& ItemDatabase::GetProperties(const std::string& name) {
    static ItemProperties empty;
    auto it = s_Database.find(name);
    return (it != s_Database.end()) ? it->second : empty;
}