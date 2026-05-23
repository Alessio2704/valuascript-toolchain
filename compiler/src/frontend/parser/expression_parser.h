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

        using PrefixParseFn = ExprPtr (ExpressionParser::*)();
        using InfixParseFn = ExprPtr (ExpressionParser::*)(ExprPtr left, const Token& op);

        struct ParseRule
        {
            PrefixParseFn prefix;
            InfixParseFn infix;
            Precedence precedence;
            bool is_right_associative;
        };

        static ParseRule get_rule(TokenType type);

        ExprPtr parse_expression(Precedence min_precedence = Precedence::Or);
        ExprPtr handle_invalid_expression_start();

        template <typename T>
        ExprPtr parse_literal_prefix();

        ExprPtr parse_prefix_unary();
        ExprPtr parse_infix_binary(ExprPtr left, const Token& op);
        ExprPtr parse_tuple_or_grouping();
        ExprPtr complete_tuple(ExprPtr first_expr, const Token& start_token);
        ExprPtr complete_grouping(ExprPtr first_expr, bool first_expr_failed, const Token& start_token);
        ExprPtr parse_tensor_literal();
        ExprPtr parse_dict_literal();
        ExprPtr parse_function_call(ExprPtr target, const Token& op);
        ExprPtr parse_tensor_access(ExprPtr target, const Token& op);
        ExprPtr parse_dot_access(ExprPtr target, const Token& op);
        ExprPtr parse_conditional_expression();
        ExprPtr parse_switch_expression();
        ExprPtr parse_switch_target();
        void parse_switch_body(std::vector<std::pair<std::vector<std::string>, ExprPtr>>& cases,
                               ExprPtr& default_case);
        std::pair<std::vector<std::string>, ExprPtr> parse_switch_case();
        ExprPtr parse_switch_default();
        ExprPtr parse_switch_result();

        std::vector<ExprPtr> parse_expression_list(TokenType closing_token,
                                                   std::optional<ParserErrorCode> trailing_comma_err =
                                                       std::nullopt,
                                                   const std::vector<TokenType>& recovery_boundaries = {});

    private:
        bool is_inside_expr_grouping() const;
        bool can_continue_expression(const Token& op_tok, const ParseRule& rule, Precedence min_prec,
                                     bool inside_grouping) const;
        bool is_dangling_binary_operator(const Token& op) const;
        ExprPtr handle_dangling_binary_operator(ExprPtr left, const Token& op);
        void check_comparison_chaining(const ParseRule& previous_rule) const;
    };

    template <typename T>
    ExprPtr ExpressionParser::parse_literal_prefix()
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
