#pragma once
#include <array>
#include <algorithm>
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

    struct KeywordEntry
    {
        std::string_view name;
        TokenType type;
    };

    inline constexpr std::array<KeywordEntry, 21> RESERVED_KEYWORDS = {{
        {"and", TokenType::And},
        {"case", TokenType::Case},
        {"default", TokenType::Default},
        {"else", TokenType::Else},
        {"enum", TokenType::Enum},
        {"extension", TokenType::Extension},
        {"false", TokenType::False},
        {"func", TokenType::Func},
        {"if", TokenType::If},
        {"import", TokenType::Import},
        {"let", TokenType::Let},
        {"mod", TokenType::Mod},
        {"not", TokenType::Not},
        {"or", TokenType::Or},
        {"return", TokenType::Return},
        {"self", TokenType::Self},
        {"struct", TokenType::Struct},
        {"switch", TokenType::Switch},
        {"then", TokenType::Then},
        {"true", TokenType::True},
        {"typealias", TokenType::Typealias}
    }};

    constexpr bool is_keywords_sorted() noexcept
    {
        for (size_t i = 1; i < RESERVED_KEYWORDS.size(); ++i)
        {
            if (RESERVED_KEYWORDS[i - 1].name >= RESERVED_KEYWORDS[i].name)
                return false;
        }
        return true;
    }

    static_assert(is_keywords_sorted(), "RESERVED_KEYWORDS must be strictly sorted alphabetically!");

    [[nodiscard]] constexpr std::optional<TokenType> get_keyword_type(std::string_view lexeme) noexcept
    {
        const auto it = std::lower_bound(
            RESERVED_KEYWORDS.begin(),
            RESERVED_KEYWORDS.end(),
            lexeme,
            [](const KeywordEntry& entry, std::string_view key) noexcept {
                return entry.name < key;
            }
        );

        if (it != RESERVED_KEYWORDS.end() && it->name == lexeme)
        {
            return it->type;
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr bool is_reserved_keyword(std::string_view lexeme) noexcept
    {
        return get_keyword_type(lexeme).has_value();
    }

    [[nodiscard]] constexpr bool is_reserved_keyword(const Token& token) noexcept
    {
        return get_keyword_type(token.lexeme).has_value();
    }

    inline std::vector<std::string> get_all_reserved_keyword_strings()
    {
        std::vector<std::string> keywords;
        keywords.reserve(RESERVED_KEYWORDS.size());
        for (const auto& entry : RESERVED_KEYWORDS)
        {
            keywords.emplace_back(entry.name);
        }
        return keywords;
    }
}
