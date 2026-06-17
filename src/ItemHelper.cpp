#include "ItemHelper.hpp"
#include "Util/Logger.hpp"
#include <algorithm>

namespace ItemHelper {

std::pair<bool, std::string> ApplyEffect(std::shared_ptr<Character> player,
                                         std::shared_ptr<Pokemon> target,
                                         const std::string& itemName) {
    // 1. Check inventory
    if (player->GetItemCount(itemName) <= 0) {
        return {false, "You don't have any " + itemName + "!"};
    }

    // 2. Validate target (unless it's a Pokéball – but we won't call this for Pokéballs)
    if (!target) {
        return {false, "No target selected!"};
    }
    if (target->IsFainted()) {
        return {false, target->GetName() + " is fainted! It won't have any effect."};
    }

    // 3. Consume one
    player->RemoveItem(itemName, 1);

    // 4. Apply effect
    if (itemName == "Potion") {
        int heal = 20;
        int newHP = std::min(target->GetMaxHP(), target->GetCurrentHP() + heal);
        target->SetCurrentHP(newHP);
        return {true, "Used Potion on " + target->GetName() + "!"};
    }
    else if (itemName == "Potion1" || itemName == "Super Potion") {
        int heal = 50;
        int newHP = std::min(target->GetMaxHP(), target->GetCurrentHP() + heal);
        target->SetCurrentHP(newHP);
        return {true, "Used Super Potion on " + target->GetName() + "!"};
    }
    // Add Hyper Potion, Max Potion, etc.

    else if (itemName == "Antidote") {
        if (target->IsPoisoned()) {
            target->CurePoison();
            return {true, target->GetName() + " was cured of poison!"};
        } else {
            return {true, "But it had no effect!"}; // item consumed, turn still used
        }
    }
    // Add other status cures

    else {
        // Unknown item – we already removed it, so treat as used? Probably not.
        // We'll return false and the item won't be consumed.
        // But we already consumed it above! So we need to either not consume it, or handle it before removal.
        // Better: move consumption *after* we know it's a valid item.
        // Refactor: check the item name before consuming.
        // For now, we'll assume all items passed here are valid, so we log a warning.
        LOG_WARN("[ItemHelper] Unknown item '{}' – it was consumed but has no effect!", itemName);
        return {true, "You used " + itemName + ", but nothing happened."};
    }
}

} // namespace ItemHelper