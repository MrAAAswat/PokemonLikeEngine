#pragma once
#include <string>
#include <vector>

struct ShopItem {
    std::string itemName;
    int quantity = -1;   // -1 = unlimited
};

struct ShopData {
    std::string shopName;
    std::vector<ShopItem> items;
};

ShopData LoadShop(const std::string& relativePath);