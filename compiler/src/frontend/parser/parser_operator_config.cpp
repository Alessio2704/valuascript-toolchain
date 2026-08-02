#include "parser.h"
#include "token/reserved_keyword_lookup.h"
#include "token/operator_lookup.h"
#include <unordered_set>

namespace valuascript::compiler
{
    std::pair<Precedence, bool> TokenTraits::get_binary_op_info(TokenType type)
    {
        return shared::get_binary_op_info(type);
    }

    bool TokenTraits::is_binary_operator(const TokenType type)
    {
        return shared::is_binary_operator(type);
    }

    bool TokenTraits::is_unary_operator(const TokenType type)
    {
        return shared::is_unary_operator(type);
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
