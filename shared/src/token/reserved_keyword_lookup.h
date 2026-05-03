#pragma once
#include <unordered_map>
#include <optional>
#include <vector>
#include <string>
#include "token.h"

namespace valuascript::shared
{
    const std::unordered_map<std::string, TokenType>& get_reserved_keywords();

    std::optional<TokenType> get_keyword_type(const std::string& lexeme);

    bool is_reserved_keyword(const Token& token);

    std::vector<std::string> get_all_reserved_keyword_strings();
}
