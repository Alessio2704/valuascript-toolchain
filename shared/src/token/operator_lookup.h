#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <utility>
#include "token.h"

namespace valuascript::shared
{
    const std::unordered_map<std::string, TokenType>& get_binary_operators_map();
    const std::unordered_map<std::string, TokenType>& get_unary_operators_map();

    std::vector<std::pair<TokenType, std::string>> get_all_binary_operators();
    std::vector<std::pair<TokenType, std::string>> get_all_unary_operators();
}
