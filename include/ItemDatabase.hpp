#pragma once
#include <string>
#include <unordered_map>
#include "Map.hpp"   // for ItemProperties (or move the struct here)

class ItemDatabase {
public:
    static void Load(const std::string& jsonPath);
    static const ItemProperties& GetProperties(const std::string& name);
private:
    inline static std::unordered_map<std::string, ItemProperties> s_Database;
};