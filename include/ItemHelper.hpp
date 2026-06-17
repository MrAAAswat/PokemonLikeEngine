#pragma once
#include <string>
#include <memory>
#include "Pokemon.hpp"
#include "Character.hpp"

namespace ItemHelper {
    // Returns a message describing what happened
    std::pair<bool, std::string> ApplyEffect(std::shared_ptr<Character> player,
                                         std::shared_ptr<Pokemon> target,
                                         const std::string& itemName);
}