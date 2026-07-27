#pragma once
#include <unordered_map>
#include <optional>
#include <vector>
#include <string>
#include <string_view>
#include "token.h"

namespace valuascript::shared
{
    struct StringHash
    {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const noexcept
        {
            return std::hash<std::string_view>{}(sv);
        }
    };

    using KeywordMap = std::unordered_map<std::string, TokenType, StringHash, std::equal_to<>>;

    const KeywordMap& get_reserved_keywords();

    std::optional<TokenType> get_keyword_type(std::string_view lexeme);

    bool is_reserved_keyword(const Token& token);

    std::vector<std::string> get_all_reserved_keyword_strings();
}
