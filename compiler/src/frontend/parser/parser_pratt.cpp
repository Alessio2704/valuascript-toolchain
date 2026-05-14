#include "parser.h"
#include "token/reserved_keyword_lookup.h"
#include "token/operator_lookup.h"
#include <algorithm>
#include <array>

namespace valuascript::compiler
{
    Parser::ParseRule Parser::get_rule(TokenType type)
    {
        static const std::array<ParseRule, 256> rules = []
        {
            std::array<ParseRule, 256> r{};

            auto set_prefix = [&r](TokenType t, PrefixParseFn pre)
            {
                r[static_cast<size_t>(t)].prefix = pre;
            };

            auto set_infix = [&r](TokenType t, InfixParseFn in, Precedence prec, bool is_right_assoc = false)
            {
                auto idx = static_cast<size_t>(t);
                r[idx].infix = in;
                r[idx].precedence = prec;
                r[idx].is_right_associative = is_right_assoc;
            };

            for (const auto& [token, lexeme] : get_all_unary_operators())
            {
                set_prefix(token, &Parser::parse_prefix_unary);
            }

            for (const auto& [token, lexeme] : get_all_binary_operators())
            {
                auto [prec, is_right] = TokenTraits::get_binary_op_info(token);
                if (prec != Precedence::None)
                {
                    set_infix(token, &Parser::parse_infix_binary, prec, is_right);
                }
            }

            set_infix(TokenType::LeftParen, &Parser::parse_function_call, Precedence::Postfix);
            set_infix(TokenType::LeftBracket, &Parser::parse_tensor_access, Precedence::Postfix);
            set_infix(TokenType::Dot, &Parser::parse_dot_access, Precedence::Postfix);

            set_prefix(TokenType::LeftParen, &Parser::parse_tuple_or_grouping);
            set_prefix(TokenType::LeftBracket, &Parser::parse_tensor_literal);
            set_prefix(TokenType::LeftBrace, &Parser::parse_dict_literal);

            set_prefix(TokenType::Number, &Parser::parse_literal_prefix<NumberLiteral>);
            set_prefix(TokenType::PercentageLiteral, &Parser::parse_literal_prefix<PercentageLiteral>);
            set_prefix(TokenType::String, &Parser::parse_literal_prefix<StringLiteral>);
            set_prefix(TokenType::True, &Parser::parse_literal_prefix<BooleanLiteral>);
            set_prefix(TokenType::False, &Parser::parse_literal_prefix<BooleanLiteral>);
            set_prefix(TokenType::Identifier, &Parser::parse_literal_prefix<IdentifierAccess>);
            set_prefix(TokenType::Self, &Parser::parse_literal_prefix<SelfExpression>);

            set_prefix(TokenType::Switch, &Parser::parse_switch_expression);
            set_prefix(TokenType::If, &Parser::parse_conditional_expression);

            return r;
        }();

        auto idx = static_cast<size_t>(type);
        if (idx < rules.size()) return rules[idx];
        return {nullptr, nullptr, Precedence::None, false};
    }

    std::unique_ptr<Expression> Parser::parse_expression(const Precedence min_precedence)
    {
        const Token& start_tok = cursor_.peek();
        ParseRule rule = get_rule(start_tok.type);

        if (rule.prefix == nullptr)
        {
            return handle_invalid_expression_start();
        }

        auto left = (this->*(rule.prefix))();

        while (true)
        {
            const Token& op_tok = cursor_.peek();

            bool inside_expr_grouping = std::any_of(active_closers_.begin(), active_closers_.end(), [](TokenType t)
            {
                return t == TokenType::RightParen || t == TokenType::RightBracket;
            });

            if (op_tok.line > cursor_.previous().line && !inside_expr_grouping)
            {
                if (!TokenTraits::is_postfix_operator(op_tok.type))
                {
                    break;
                }
            }

            ParseRule infix_rule = get_rule(op_tok.type);

            if (infix_rule.precedence < min_precedence || infix_rule.precedence == Precedence::None)
            {
                break;
            }

            if (infix_rule.infix == nullptr)
            {
                break;
            }

            Token op = cursor_.advance();

            if (!inside_expr_grouping && infix_rule.infix == &Parser::parse_infix_binary)
            {
                const Token& next = cursor_.peek();
                if (next.type == TokenType::EndOfFile || (next.line > op.line && (
                    TokenTraits::is_statement_start(next, cursor_.peek(1).type) ||
                    TokenTraits::is_expression_statement_start(next, cursor_.peek(1).type))))
                {
                    cursor_.report_error_no_panic(op, ValuascriptErrorCode::InvalidExpression);
                    return make_node_with_span<BinaryExpression>(
                        cursor_.combine_spans(left->span, cursor_.make_span(op, op)), std::move(left), op.type,
                        nullptr);
                }
            }

            left = (this->*(infix_rule.infix))(std::move(left), op);

            if (infix_rule.precedence == Precedence::Comparison &&
                get_rule(cursor_.peek().type).precedence == Precedence::Comparison)
            {
                cursor_.report_error_no_panic(cursor_.peek(),
                                              ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations);
            }
        }

        return left;
    }

    std::unique_ptr<Expression> Parser::parse_infix_binary(std::unique_ptr<Expression> left, const Token& op)
    {
        ParseRule infix_rule = get_rule(op.type);
        Precedence next_precedence = infix_rule.is_right_associative
                                         ? infix_rule.precedence
                                         : static_cast<Precedence>(static_cast<int>(infix_rule.precedence) + 1);

        auto right = attempt_parse<std::unique_ptr<Expression>>(
            [&]() { return parse_expression(next_precedence); },
            {
                .stop_tokens = {
                    TokenType::Comma, TokenType::RightParen, TokenType::RightBracket, TokenType::RightBrace
                },
                .force_stop_at_statement_boundary_ignoring_dangling_op = true
            },
            nullptr
        );

        const SourceSpan right_span = right
                                          ? right->span
                                          : cursor_.make_span(cursor_.previous(), cursor_.previous());
        const SourceSpan combined = cursor_.combine_spans(left->span, right_span);

        return make_node_with_span<BinaryExpression>(combined, std::move(left), op.type, std::move(right));
    }

    std::unique_ptr<Expression> Parser::parse_prefix_unary()
    {
        Token op = cursor_.advance();

        bool inside_expr_grouping = std::any_of(active_closers_.begin(), active_closers_.end(), [](TokenType t)
        {
            return t == TokenType::RightParen || t == TokenType::RightBracket;
        });

        if (!inside_expr_grouping)
        {
            const Token& next = cursor_.peek();
            if (next.type == TokenType::EndOfFile || (next.line > op.line && (
                TokenTraits::is_statement_start(next, cursor_.peek(1).type) ||
                TokenTraits::is_expression_statement_start(next, cursor_.peek(1).type))))
            {
                cursor_.report_error_no_panic(op, ValuascriptErrorCode::InvalidExpression);
                return make_node_with_span<UnaryExpression>(cursor_.make_span(op, op), op.type, nullptr);
            }
        }

        auto right = attempt_parse<std::unique_ptr<Expression>>(
            [&] { return parse_expression(Precedence::Unary); },
            {
                .stop_tokens = {
                    TokenType::Comma, TokenType::RightParen, TokenType::RightBracket, TokenType::RightBrace
                },
                .force_stop_at_statement_boundary_ignoring_dangling_op = true
            },
            nullptr
        );

        return make_node<UnaryExpression>(op, op.type, std::move(right));
    }

    std::unique_ptr<Expression> Parser::handle_invalid_expression_start()
    {
        const Token& tok = cursor_.peek();
        const Token& next = cursor_.peek(1);
        const Token& prev = cursor_.previous();

        bool is_stmt_start = TokenTraits::is_statement_start(tok, next.type);
        bool force_location = (tok.type != TokenType::EndOfFile && !is_stmt_start);

        if (is_stmt_start)
        {
            if (tok.line > prev.line)
            {
                if (TokenTraits::is_dangling_operator(prev.type) ||
                    TokenTraits::is_grouping_opener(prev.type))
                {
                    cursor_.report_error(prev, ValuascriptErrorCode::InvalidExpression);
                }
                else
                {
                    cursor_.report_error(tok, ValuascriptErrorCode::InvalidExpression, force_location);
                }
            }

            const Token& start_tok = cursor_.peek();

            if (tok.type == TokenType::At && !is_at_any_declaration())
            {
                parse_modifiers();
                SourceSpan span = cursor_.make_span(start_tok, cursor_.previous());
                cursor_.report_error_no_panic(span, ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere);
                cursor_.report_error_no_panic(
                    span, ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
            }
            else
            {
                consume_unexpected_statement_gracefully();
                SourceSpan span = cursor_.make_span(start_tok, cursor_.previous());
                cursor_.report_error_no_panic(span, ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere);
            }

            if (TokenTraits::is_expression_start(cursor_.peek().type))
            {
                return parse_expression();
            }

            throw ParseSyncException();
        }

        if (tok.type == TokenType::Case || tok.type == TokenType::Default ||
            tok.type == TokenType::RightBrace || tok.type == TokenType::RightParen ||
            tok.type == TokenType::RightBracket || tok.type == TokenType::Return ||
            tok.type == TokenType::Then || tok.type == TokenType::Else)
        {
            if (tok.line > prev.line)
            {
                if (TokenTraits::is_dangling_operator(prev.type) ||
                    TokenTraits::is_grouping_opener(prev.type))
                {
                    cursor_.report_error(prev, ValuascriptErrorCode::InvalidExpression);
                }
            }

            cursor_.report_error(tok, ValuascriptErrorCode::InvalidExpression, force_location);
        }

        if (is_reserved_keyword(tok))
        {
            cursor_.report_error_no_panic(tok, ValuascriptErrorCode::ReservedKeywordAsIdentifier, true);
            cursor_.advance();
            return make_node_with_span<IdentifierAccess>(cursor_.make_span(tok, tok), tok.lexeme);
        }

        cursor_.report_error(tok, ValuascriptErrorCode::InvalidExpression, force_location);
    }
}
