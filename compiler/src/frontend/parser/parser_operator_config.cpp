#include "parser.h"
#include "token/reserved_keyword_lookup.h"
#include "token/operator_lookup.h"
#include <unordered_set>

namespace valuascript::compiler
{
    std::pair<Precedence, bool> TokenTraits::get_binary_op_info(TokenType type)
    {
        switch (type)
        {
        case TokenType::Or: return {Precedence::Or, false};
        case TokenType::And: return {Precedence::And, false};
        case TokenType::Equals:
        case TokenType::NotEquals:
        case TokenType::Less:
        case TokenType::LessEqual:
        case TokenType::Greater:
        case TokenType::GreaterEqual: return {Precedence::Comparison, false};
        case TokenType::Plus:
        case TokenType::Minus: return {Precedence::Term, false};
        case TokenType::Star:
        case TokenType::Slash:
        case TokenType::Mod: return {Precedence::Factor, false};
        case TokenType::Caret: return {Precedence::Power, true};
        default: return {Precedence::None, false};
        }
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
        return binary_ops.contains(type);
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
        return unary_ops.contains(type);
    }

    bool TokenTraits::is_postfix_operator(const TokenType type)
    {
        return type == TokenType::Dot || type == TokenType::LeftParen || type == TokenType::LeftBracket;
    }

    bool TokenTraits::is_dangling_operator(const TokenType type)
    {
        return is_binary_operator(type) || is_unary_operator(type) ||
            type == TokenType::Assign || type == TokenType::Return || type == TokenType::Comma ||
            type == TokenType::Colon || type == TokenType::Arrow || type == TokenType::Then ||
            type == TokenType::Else || type == TokenType::Dot;
    }
}
