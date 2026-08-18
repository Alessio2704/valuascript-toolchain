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
        {.name = "and",       .type = TokenType::And},
        {.name = "case",      .type = TokenType::Case},
        {.name = "default",   .type = TokenType::Default},
        {.name = "else",      .type = TokenType::Else},
        {.name = "enum",      .type = TokenType::Enum},
        {.name = "extension", .type = TokenType::Extension},
        {.name = "false",     .type = TokenType::False},
        {.name = "func",      .type = TokenType::Func},
        {.name = "if",        .type = TokenType::If},
        {.name = "import",    .type = TokenType::Import},
        {.name = "let",       .type = TokenType::Let},
        {.name = "mod",       .type = TokenType::Mod},
        {.name = "not",       .type = TokenType::Not},
        {.name = "or",        .type = TokenType::Or},
        {.name = "return",    .type = TokenType::Return},
        {.name = "self",      .type = TokenType::Self},
        {.name = "struct",    .type = TokenType::Struct},
        {.name = "switch",    .type = TokenType::Switch},
        {.name = "then",      .type = TokenType::Then},
        {.name = "true",      .type = TokenType::True},
        {.name = "typealias", .type = TokenType::Typealias}
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
