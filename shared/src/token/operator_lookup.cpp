#include "operator_lookup.h"

namespace valuascript::shared
{
    const OperatorMap& get_binary_operators_map()
    {
        static const OperatorMap map = {
            {"+", TokenType::Plus}, {"-", TokenType::Minus},
            {"*", TokenType::Star}, {"/", TokenType::Slash},
            {"^", TokenType::Caret}, {"mod", TokenType::Mod},
            {"==", TokenType::Equals}, {"!=", TokenType::NotEquals},
            {">", TokenType::Greater}, {">=", TokenType::GreaterEqual},
            {"<", TokenType::Less}, {"<=", TokenType::LessEqual},
            {"and", TokenType::And}, {"or", TokenType::Or}
        };
        return map;
    }

    const OperatorMap& get_unary_operators_map()
    {
        static const OperatorMap map = {
            {"+", TokenType::Plus},
            {"-", TokenType::Minus},
            {"not", TokenType::Not}
        };
        return map;
    }

    std::vector<std::pair<TokenType, std::string>> get_all_binary_operators()
    {
        auto& map = get_binary_operators_map();
        std::vector<std::pair<TokenType, std::string>> ops;
        ops.reserve(map.size());
        for (const auto& [text, type] : map)
        {
            ops.emplace_back(type, text);
        }
        return ops;
    }

    std::vector<std::pair<TokenType, std::string>> get_all_unary_operators()
    {
        auto& map = get_unary_operators_map();
        std::vector<std::pair<TokenType, std::string>> ops;
        ops.reserve(map.size());
        for (const auto& [text, type] : map)
        {
            ops.emplace_back(type, text);
        }
        return ops;
    }
}
