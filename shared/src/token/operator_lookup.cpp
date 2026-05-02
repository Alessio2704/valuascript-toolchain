#include "operator_lookup.h"

namespace valuascript::shared
{
    const std::unordered_map<std::string, TokenType> kBinaryOperators = {
        {"+", TokenType::Plus}, {"-", TokenType::Minus},
        {"*", TokenType::Star}, {"/", TokenType::Slash},
        {"^", TokenType::Caret}, {"mod", TokenType::Mod},
        {"==", TokenType::Equals}, {"!=", TokenType::NotEquals},
        {">", TokenType::Greater}, {">=", TokenType::GreaterEqual},
        {"<", TokenType::Less}, {"<=", TokenType::LessEqual},
        {"and", TokenType::And}, {"or", TokenType::Or}
    };

    const std::unordered_map<std::string, TokenType> kUnaryOperators = {
        {"+", TokenType::Plus},
        {"-", TokenType::Minus},
        {"not", TokenType::Not}
    };

    std::vector<std::pair<TokenType, std::string>> get_all_binary_operators()
    {
        std::vector<std::pair<TokenType, std::string>> ops;
        ops.reserve(kBinaryOperators.size());
        for (const auto& [text, type] : kBinaryOperators)
        {
            ops.emplace_back(type, text);
        }
        return ops;
    }

    std::vector<std::pair<TokenType, std::string>> get_all_unary_operators()
    {
        std::vector<std::pair<TokenType, std::string>> ops;
        ops.reserve(kUnaryOperators.size());
        for (const auto& [text, type] : kUnaryOperators)
        {
            ops.emplace_back(type, text);
        }
        return ops;
    }
}
