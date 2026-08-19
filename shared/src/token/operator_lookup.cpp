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

    const std::vector<std::pair<TokenType, std::string>>& get_all_binary_operators()
    {
        static const std::vector<std::pair<TokenType, std::string>> ops = [] {
            std::vector<std::pair<TokenType, std::string>> list;
            list.reserve(BINARY_OPERATORS.size());
            for (const auto& op : BINARY_OPERATORS)
            {
                list.emplace_back(op.type, std::string(op.text));
            }
            return list;
        }();
        return ops;
    }

    const std::vector<std::pair<TokenType, std::string>>& get_all_unary_operators()
    {
        static const std::vector<std::pair<TokenType, std::string>> ops = [] {
            std::vector<std::pair<TokenType, std::string>> list;
            list.reserve(UNARY_OPERATORS.size());
            for (const auto& op : UNARY_OPERATORS)
            {
                list.emplace_back(op.type, std::string(op.text));
            }
            return list;
        }();
        return ops;
    }
}
