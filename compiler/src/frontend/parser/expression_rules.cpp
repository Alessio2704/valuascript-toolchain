#include "expression_parser.h"
#include "token/operator_lookup.h"
#include <array>

namespace valuascript::compiler
{
    using EP = ExpressionParser;

    static std::array<EP::ParseRule, 256> build_expression_rules()
    {
        std::array<EP::ParseRule, 256> rules{};

        auto set_prefix = [&rules](TokenType t, EP::PrefixParseFn pre)
        {
            rules[static_cast<size_t>(t)].prefix = pre;
        };

        auto set_infix = [&rules](TokenType t, EP::InfixParseFn in, Precedence prec, bool is_right_assoc = false)
        {
            auto idx = static_cast<size_t>(t);
            rules[idx].infix = in;
            rules[idx].precedence = prec;
            rules[idx].is_right_associative = is_right_assoc;
        };

        for (const auto& [token, lexeme] : get_all_unary_operators())
        {
            set_prefix(token, &EP::parse_prefix_unary);
        }

        for (const auto& [token, lexeme] : get_all_binary_operators())
        {
            auto [prec, is_right] = TokenTraits::get_binary_op_info(token);
            if (prec != Precedence::None)
            {
                set_infix(token, &EP::parse_infix_binary, prec, is_right);
            }
        }

        set_infix(TokenType::LeftParen, &EP::parse_function_call, Precedence::Postfix);
        set_infix(TokenType::LeftBracket, &EP::parse_tensor_access, Precedence::Postfix);
        set_infix(TokenType::Dot, &EP::parse_dot_access, Precedence::Postfix);

        set_prefix(TokenType::LeftParen, &EP::parse_tuple_or_grouping);
        set_prefix(TokenType::LeftBracket, &EP::parse_tensor_literal);
        set_prefix(TokenType::LeftBrace, &EP::parse_dict_literal);

        set_prefix(TokenType::Number, &EP::parse_literal_prefix<NumberLiteral>);
        set_prefix(TokenType::PercentageLiteral, &EP::parse_literal_prefix<PercentageLiteral>);
        set_prefix(TokenType::String, &EP::parse_literal_prefix<StringLiteral>);
        set_prefix(TokenType::True, &EP::parse_literal_prefix<BooleanLiteral>);
        set_prefix(TokenType::False, &EP::parse_literal_prefix<BooleanLiteral>);
        set_prefix(TokenType::Identifier, &EP::parse_literal_prefix<IdentifierAccess>);
        set_prefix(TokenType::Self, &EP::parse_literal_prefix<SelfExpression>);

        set_prefix(TokenType::Switch, &EP::parse_switch_expression);
        set_prefix(TokenType::If, &EP::parse_conditional_expression);

        return rules;
    }

    ExpressionParser::ParseRule ExpressionParser::get_rule(TokenType type)
    {
        static const std::array<EP::ParseRule, 256> rules = build_expression_rules();

        auto idx = static_cast<size_t>(type);
        if (idx < rules.size())
        {
            return rules[idx];
        }

        return {nullptr, nullptr, Precedence::None, false};
    }
}
