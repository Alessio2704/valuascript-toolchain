#pragma once
#include "parser_context.h"
#include "ast_factory.h"

namespace valuascript::compiler
{
    class Parser;

    class ExpressionParser
    {
    public:
        Parser& parser;
        ParserContext& ctx;
        TokenCursor& cursor;

        explicit ExpressionParser(Parser& p);

        using PrefixParseFn = std::unique_ptr<Expression> (ExpressionParser::*)();
        using InfixParseFn = std::unique_ptr<Expression> (ExpressionParser::*)(
            std::unique_ptr<Expression> left, const Token& op);

        struct ParseRule
        {
            PrefixParseFn prefix;
            InfixParseFn infix;
            Precedence precedence;
            bool is_right_associative;
        };

        static ParseRule get_rule(TokenType type);

        std::unique_ptr<Expression> parse_expression(Precedence min_precedence = Precedence::Or);
        std::unique_ptr<Expression> handle_invalid_expression_start();

        template <typename T>
        std::unique_ptr<Expression> parse_literal_prefix();

        std::unique_ptr<Expression> parse_prefix_unary();
        std::unique_ptr<Expression> parse_infix_binary(std::unique_ptr<Expression> left, const Token& op);
        std::unique_ptr<Expression> parse_tuple_or_grouping();
        std::unique_ptr<Expression> complete_tuple(std::unique_ptr<Expression> first_expr, const Token& start_token);
        std::unique_ptr<Expression> complete_grouping(std::unique_ptr<Expression> first_expr, bool first_expr_failed,
                                                      const Token& start_token);
        std::unique_ptr<Expression> parse_tensor_literal();
        std::unique_ptr<Expression> parse_dict_literal();
        std::unique_ptr<Expression> parse_function_call(std::unique_ptr<Expression> target, const Token& op);
        std::unique_ptr<Expression> parse_tensor_access(std::unique_ptr<Expression> target, const Token& op);
        std::unique_ptr<Expression> parse_dot_access(std::unique_ptr<Expression> target, const Token& op);
        std::unique_ptr<Expression> parse_conditional_expression();
        std::unique_ptr<Expression> parse_switch_expression();
        std::unique_ptr<Expression> parse_switch_target();
        void parse_switch_body(std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression>>>& cases,
                               std::unique_ptr<Expression>& default_case);
        std::pair<std::vector<std::string>, std::unique_ptr<Expression>> parse_switch_case();
        std::unique_ptr<Expression> parse_switch_default();
        std::unique_ptr<Expression> parse_switch_result();

        std::vector<std::unique_ptr<Expression>> parse_expression_list(TokenType closing_token,
                                                                       std::optional<ValuascriptErrorCode>
                                                                       trailing_comma_err = std::nullopt,
                                                                       const std::vector<TokenType>& recovery_boundaries
                                                                           = {});

    private:
        bool is_inside_expr_grouping() const;
        bool can_continue_expression(const Token& op_tok, const ParseRule& rule, Precedence min_prec,
                                     bool inside_grouping) const;
        bool is_dangling_binary_operator(const Token& op) const;
        std::unique_ptr<Expression> handle_dangling_binary_operator(std::unique_ptr<Expression> left, const Token& op);
        void check_comparison_chaining(const ParseRule& previous_rule) const;
    };

    template <typename T>
    std::unique_ptr<Expression> ExpressionParser::parse_literal_prefix()
    {
        const Token& t = cursor.advance();
        if constexpr (std::is_same_v<T, BooleanLiteral>)
        {
            return AstFactory::make_node<T>(cursor, t, t.type == TokenType::True);
        }
        else if constexpr (std::is_same_v<T, SelfExpression>)
        {
            return AstFactory::make_node<T>(cursor, t);
        }
        else
        {
            return AstFactory::make_node<T>(cursor, t, t.lexeme);
        }
    }
}
