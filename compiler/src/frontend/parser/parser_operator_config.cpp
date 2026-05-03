#include "parser.h"
#include "token/reserved_keyword_lookup.h"
#include "token/operator_lookup.h"
#include <unordered_set>

namespace valuascript::compiler
{
    struct OperatorConfig
    {
        Precedence precedence;
        bool is_right_associative;
    };

    static const std::unordered_map<TokenType, OperatorConfig>& get_operator_configs()
    {
        static const std::unordered_map<TokenType, OperatorConfig> configs = {
            {TokenType::Or, {Precedence::Or, false}},
            {TokenType::And, {Precedence::And, false}},
            {TokenType::Equals, {Precedence::Comparison, false}},
            {TokenType::NotEquals, {Precedence::Comparison, false}},
            {TokenType::Less, {Precedence::Comparison, false}},
            {TokenType::LessEqual, {Precedence::Comparison, false}},
            {TokenType::Greater, {Precedence::Comparison, false}},
            {TokenType::GreaterEqual, {Precedence::Comparison, false}},
            {TokenType::Plus, {Precedence::Term, false}},
            {TokenType::Minus, {Precedence::Term, false}},
            {TokenType::Star, {Precedence::Factor, false}},
            {TokenType::Slash, {Precedence::Factor, false}},
            {TokenType::Mod, {Precedence::Factor, false}},
            {TokenType::Caret, {Precedence::Power, true}}
        };
        return configs;
    }

    Precedence TokenTraits::get_operator_precedence(const TokenType op_type)
    {
        auto& configs = get_operator_configs();
        if (auto it = configs.find(op_type); it != configs.end())
        {
            return it->second.precedence;
        }
        return Precedence::None;
    }

    bool TokenTraits::is_operator_right_associative(TokenType op_type)
    {
        auto& configs = get_operator_configs();
        if (auto it = configs.find(op_type); it != configs.end())
        {
            return it->second.is_right_associative;
        }
        return false;
    }

    bool TokenTraits::is_binary_operator(const TokenType type)
    {
        static const std::unordered_set<TokenType> binary_ops = []
        {
            std::unordered_set<TokenType> set;
            for (const auto& [token, lexeme] : get_all_binary_operators())
            {
                set.insert(token);
            }
            return set;
        }();
        return binary_ops.find(type) != binary_ops.end();
    }

    bool TokenTraits::is_unary_operator(const TokenType type)
    {
        static const std::unordered_set<TokenType> unary_ops = []
        {
            std::unordered_set<TokenType> set;
            for (const auto& [token, lexeme] : get_all_unary_operators())
            {
                set.insert(token);
            }
            return set;
        }();
        return unary_ops.find(type) != unary_ops.end();
    }

    bool TokenTraits::is_dangling_operator(const TokenType type)
    {
        return is_binary_operator(type) || is_unary_operator(type) ||
            type == TokenType::Assign || type == TokenType::Return || type == TokenType::Comma ||
            type == TokenType::Colon || type == TokenType::Arrow || type == TokenType::Then ||
            type == TokenType::Else || type == TokenType::Dot;
    }
}
