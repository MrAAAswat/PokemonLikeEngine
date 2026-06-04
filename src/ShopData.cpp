#include "ShopData.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

ShopData LoadShop(const std::string& relativePath) {
    std::ifstream f(std::string(RESOURCE_DIR) + "/" + relativePath);
    if (!f.is_open()) return {};
    nlohmann::json j;
    f >> j;
    ShopData shop;
    shop.shopName = j.value("shopName", "Shop");
    for (auto& item : j["items"]) {
        shop.items.push_back({
            item["itemName"].get<std::string>(),
            item.value("quantity", -1)
        });
    }
    return shop;
}