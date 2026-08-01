#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <utility>
#include "token.h"
#include "reserved_keyword_lookup.h"

namespace valuascript::shared
{
    using OperatorMap = std::unordered_map<std::string, TokenType, StringHash, std::equal_to<>>;

    const OperatorMap& get_binary_operators_map();
    const OperatorMap& get_unary_operators_map();

    std::vector<std::pair<TokenType, std::string>> get_all_binary_operators();
    std::vector<std::pair<TokenType, std::string>> get_all_unary_operators();
}
