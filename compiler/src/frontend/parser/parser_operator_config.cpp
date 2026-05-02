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

    static const std::unordered_map<TokenType, OperatorConfig> kOperatorConfigs = {
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

    Precedence TokenTraits::get_operator_precedence(const TokenType op_type)
    {
        if (auto it = kOperatorConfigs.find(op_type); it != kOperatorConfigs.end())
        {
            return it->second.precedence;
        }
        return Precedence::None;
    }

    bool TokenTraits::is_operator_right_associative(TokenType op_type)
    {
        if (auto it = kOperatorConfigs.find(op_type); it != kOperatorConfigs.end())
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
            for (const auto& [lexeme, tok] : kBinaryOperators)
            {
                set.insert(tok);
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
            for (const auto& [lexeme, tok] : kUnaryOperators)
            {
                set.insert(tok);
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
