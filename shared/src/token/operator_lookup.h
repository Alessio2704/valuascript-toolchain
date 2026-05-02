#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <utility>
#include "token.h"

namespace valuascript::shared
{
    extern const std::unordered_map<std::string, TokenType> kBinaryOperators;
    extern const std::unordered_map<std::string, TokenType> kUnaryOperators;

    std::vector<std::pair<TokenType, std::string>> get_all_binary_operators();
    std::vector<std::pair<TokenType, std::string>> get_all_unary_operators();
}
