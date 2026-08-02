#pragma once
#include <array>
#include <unordered_map>
#include <vector>
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include "token.h"
#include "reserved_keyword_lookup.h"

namespace valuascript::shared
{
    enum class Precedence
    {
        None = 0, Or = 1, And = 2, Comparison = 3, Term = 4, Factor = 5, Power = 6, Unary = 7, Postfix = 8
    };

    struct BinaryOperatorEntry {
        std::string_view text;
        TokenType type;
        Precedence precedence;
        bool is_right_associative;
    };

    struct UnaryOperatorEntry {
        std::string_view text;
        TokenType type;
    };

    inline constexpr std::array<BinaryOperatorEntry, 14> BINARY_OPERATORS = {{
        {"+",   TokenType::Plus,         Precedence::Term,       false},
        {"-",   TokenType::Minus,        Precedence::Term,       false},
        {"*",   TokenType::Star,         Precedence::Factor,     false},
        {"/",   TokenType::Slash,        Precedence::Factor,     false},
        {"^",   TokenType::Caret,        Precedence::Power,      true },
        {"mod", TokenType::Mod,          Precedence::Factor,     false},
        {"==",  TokenType::Equals,       Precedence::Comparison, false},
        {"!=",  TokenType::NotEquals,    Precedence::Comparison, false},
        {">",   TokenType::Greater,      Precedence::Comparison, false},
        {">=",  TokenType::GreaterEqual, Precedence::Comparison, false},
        {"<",   TokenType::Less,         Precedence::Comparison, false},
        {"<=",  TokenType::LessEqual,    Precedence::Comparison, false},
        {"and", TokenType::And,          Precedence::And,        false},
        {"or",  TokenType::Or,           Precedence::Or,         false}
    }};

    inline constexpr std::array<UnaryOperatorEntry, 3> UNARY_OPERATORS = {{
        {"+", TokenType::Plus},
        {"-", TokenType::Minus},
        {"not", TokenType::Not}
    }};

    [[nodiscard]] constexpr std::optional<TokenType> lookup_binary_operator(std::string_view text) noexcept
    {
        for (const auto& op : BINARY_OPERATORS)
        {
            if (op.text == text) return op.type;
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr std::optional<TokenType> lookup_unary_operator(std::string_view text) noexcept
    {
        for (const auto& op : UNARY_OPERATORS)
        {
            if (op.text == text) return op.type;
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr std::pair<Precedence, bool> get_binary_op_info(TokenType type) noexcept
    {
        for (const auto& op : BINARY_OPERATORS)
        {
            if (op.type == type) return {op.precedence, op.is_right_associative};
        }
        return {Precedence::None, false};
    }

    [[nodiscard]] constexpr bool is_binary_operator(TokenType type) noexcept
    {
        for (const auto& op : BINARY_OPERATORS)
        {
            if (op.type == type) return true;
        }
        return false;
    }

    [[nodiscard]] constexpr bool is_unary_operator(TokenType type) noexcept
    {
        for (const auto& op : UNARY_OPERATORS)
        {
            if (op.type == type) return true;
        }
        return false;
    }

    using OperatorMap = std::unordered_map<std::string, TokenType, StringHash, std::equal_to<>>;

    const OperatorMap& get_binary_operators_map();
    const OperatorMap& get_unary_operators_map();

    std::vector<std::pair<TokenType, std::string>> get_all_binary_operators();
    std::vector<std::pair<TokenType, std::string>> get_all_unary_operators();
}
