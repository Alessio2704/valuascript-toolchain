#include "parser.h"
#include "token/reserved_keyword_lookup.h"
#include <algorithm>

namespace valuascript::compiler
{
    std::unique_ptr<Expression> Parser::parse_function_call(std::unique_ptr<Expression> target, const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;
        CloserTracker tracker(*this, TokenType::RightParen);
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> arguments;

        try
        {
            if (!cursor_.check(TokenType::RightParen) && !cursor_.is_at_end())
            {
                const Token& p0 = cursor_.peek();
                const TokenType p1 = cursor_.peek(1).type;

                if (p0.type == TokenType::Identifier || TokenTraits::acts_like_identifier(p0, p1))
                {
                    if (p1 != TokenType::Colon && TokenTraits::is_binary_operator(p1))
                    {
                        if (!TokenTraits::is_newline_statement_boundary(cursor_.previous(), p0, p1))
                        {
                            cursor_.report_error(cursor_.previous(),
                                                 ValuascriptErrorCode::MissingOperatorOrArgumentName);
                        }
                    }
                }
                else
                {
                    if (TokenTraits::is_expression_start(p0.type))
                    {
                        if (!TokenTraits::is_newline_statement_boundary(cursor_.previous(), p0, p1))
                        {
                            cursor_.report_error(cursor_.previous(),
                                                 ValuascriptErrorCode::MissingOperatorOrArgumentName);
                        }
                    }
                    else if (!cursor_.check(TokenType::Colon) && !cursor_.check(TokenType::Comma))
                    {
                        cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ExpectedArgumentNameOrClosingParen);
                    }
                }
            }

            ParameterRuleSpec arg_spec{
                .allow_modifiers = false,
                .allow_type = false,
                .allow_value = true,
                .require_value = true,
                .value_separator = TokenType::Colon,
                .missing_name_err = ValuascriptErrorCode::MissingArgumentNameInFunctionCall,
                .missing_value_separator_err = ValuascriptErrorCode::MissingColonAfterArgument,
                .missing_value_err = ValuascriptErrorCode::InvalidExpression,
                .unexpected_modifier_err = ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration
            };

            auto args_gen = parse_list<GenericParameter>(
                TokenType::RightParen,
                std::make_optional(ValuascriptErrorCode::TrailingCommaInFunctionCall),
                std::make_optional(ValuascriptErrorCode::MissingCommaSeparatorForArgumentsInFunctionCall),
                std::vector<TokenType>{},
                [this]()
                {
                    const Token& tok = cursor_.peek();
                    bool is_id_like = tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(
                        tok, cursor_.peek(1).type);
                    return is_id_like && cursor_.peek(1).type == TokenType::Colon;
                },
                [&]() { return parse_generic_parameter(arg_spec); }
            );

            arguments.reserve(args_gen.size());
            for (auto& g : args_gen)
            {
                arguments.emplace_back(g.name.lexeme, std::move(g.value));
            }

            const Token& end_token = cursor_.consume(TokenType::RightParen,
                                                     ValuascriptErrorCode::ExpectedRightParenAfterArguments);

            return make_node_with_span<FunctionCall>(
                cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token)),
                std::move(target), std::move(arguments));
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightParen);

            return make_node_with_span<FunctionCall>(
                cursor_.combine_spans(target_span, cursor_.make_span(cursor_.previous(), cursor_.previous())),
                std::move(target), std::move(arguments));
        }
    }

    std::unique_ptr<Expression> Parser::parse_tensor_access(std::unique_ptr<Expression> target, const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;
        CloserTracker tracker(*this, TokenType::RightBracket);
        std::unique_ptr<Expression> index_expr = nullptr;

        try
        {
            auto parse_bound = [&]() -> std::unique_ptr<Expression>
            {
                if (cursor_.check(TokenType::Colon) || cursor_.check(TokenType::RightBracket))
                {
                    return nullptr;
                }

                auto expr = parse_expression();

                if (!cursor_.check(TokenType::Colon) && !cursor_.check(TokenType::RightBracket))
                {
                    if (!TokenTraits::is_newline_statement_boundary(cursor_.previous(), cursor_.peek(),
                                                                    cursor_.peek(1).type))
                    {
                        if (TokenTraits::is_expression_start(cursor_.peek().type))
                        {
                            cursor_.report_error(cursor_.peek(),
                                                 ValuascriptErrorCode::MissingOperatorOrExpectedColonOrBracketInTensor);
                        }
                        else if (cursor_.check(TokenType::Comma))
                        {
                            cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::UnexpectedCommaInBracketAccess);
                        }
                    }
                }

                return expr;
            };

            index_expr = parse_bound();

            if (cursor_.match({TokenType::Colon}))
            {
                std::unique_ptr<Expression> end_expr = parse_bound();
                const SourceSpan colon_span = index_expr ? index_expr->span : target_span;
                const SourceSpan slice_end_span = end_expr
                                                      ? end_expr->span
                                                      : cursor_.make_span(cursor_.previous(), cursor_.previous());

                index_expr = make_node_with_span<BinaryExpression>(
                    cursor_.combine_spans(colon_span, slice_end_span), std::move(index_expr), TokenType::Colon,
                    std::move(end_expr));
            }
            else if (!index_expr)
            {
                cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::EmptyBracketAccess);
            }

            const Token& end_token = cursor_.consume(TokenType::RightBracket,
                                                     ValuascriptErrorCode::UnmatchedBracketAfterTensorIndex);

            return make_node_with_span<BracketAccess>(
                cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token)), std::move(target),
                std::move(index_expr));
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightBracket);

            return make_node_with_span<BracketAccess>(
                cursor_.combine_spans(target_span, cursor_.make_span(cursor_.previous(), cursor_.previous())),
                std::move(target), std::move(index_expr));
        }
    }

    std::unique_ptr<Expression> Parser::parse_dot_access(std::unique_ptr<Expression> target, const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;

        auto property_token = attempt_parse<Token>(
            [&]() { return consume_identifier(ValuascriptErrorCode::ExpectedPropertyName, true, true); },
            {
                .stop_tokens = {
                    TokenType::Assign,
                    TokenType::Comma,
                    TokenType::RightParen,
                    TokenType::RightBracket,
                    TokenType::RightBrace
                },
                .force_stop_at_statement_boundary_ignoring_dangling_op = true
            },
            Token(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column)
        );

        const SourceSpan property_span = cursor_.make_span(property_token, property_token);
        return make_node_with_span<DotAccess>(cursor_.combine_spans(target_span, property_span), std::move(target),
                                              property_token.lexeme);
    }

    std::unique_ptr<Expression> Parser::parse_tuple_or_grouping()
    {
        const Token& start_token = cursor_.advance();
        CloserTracker tracker(*this, TokenType::RightParen);

        if (cursor_.match({TokenType::RightParen}))
        {
            return make_node<TupleLiteral>(start_token, std::vector<std::unique_ptr<Expression>>{});
        }

        bool first_expr_failed = false;
        auto first_expr = attempt_parse<std::unique_ptr<Expression>>(
            [&]() { return parse_expression(); },
            {
                .stop_tokens = {TokenType::Comma, TokenType::RightParen},
                .stop_at_currently_tracked_closers = false,
                .stop_at_currently_tracked_sync_tokens = false
            },
            nullptr,
            &first_expr_failed
        );

        if (cursor_.match({TokenType::Comma}))
        {
            return complete_tuple(std::move(first_expr), start_token);
        }

        return complete_grouping(std::move(first_expr), first_expr_failed, start_token);
    }

    std::unique_ptr<Expression>
    Parser::complete_tuple(std::unique_ptr<Expression> first_expr, const Token& start_token)
    {
        if (cursor_.check(TokenType::RightParen))
        {
            cursor_.report_error_no_panic(cursor_.previous(), ValuascriptErrorCode::SingleElementTuplesNotAllowed);
        }

        std::vector<std::unique_ptr<Expression>> elements;
        if (first_expr) elements.push_back(std::move(first_expr));

        auto remaining = parse_expression_list(TokenType::RightParen, ValuascriptErrorCode::TrailingCommaInTuple);
        elements.insert(elements.end(), std::make_move_iterator(remaining.begin()),
                        std::make_move_iterator(remaining.end()));

        try
        {
            const Token& end_token = cursor_.consume(TokenType::RightParen,
                                                     ValuascriptErrorCode::ExpectedRightParenAfterTupleElements);
            return make_node_with_span<TupleLiteral>(cursor_.make_span(start_token, end_token), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightParen);
            return make_node<TupleLiteral>(start_token, std::move(elements));
        }
    }

    std::unique_ptr<Expression> Parser::complete_grouping(std::unique_ptr<Expression> first_expr,
                                                          bool first_expr_failed, const Token& start_token)
    {
        try
        {
            if (!first_expr_failed && !cursor_.check(TokenType::RightParen) && TokenTraits::is_expression_start(
                cursor_.peek().type))
            {
                if (!TokenTraits::is_newline_statement_boundary(cursor_.previous(), cursor_.peek(),
                                                                cursor_.peek(1).type))
                {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperatorInsideGrouping);
                }
            }

            const Token& end_token = cursor_.consume(TokenType::RightParen,
                                                     ValuascriptErrorCode::ExpectedRightParenAfterExpression);
            return make_node_with_span<GroupingExpression>(cursor_.make_span(start_token, end_token),
                                                           std::move(first_expr));
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightParen);
            return make_node<GroupingExpression>(start_token, std::move(first_expr));
        }
    }

    std::unique_ptr<Expression> Parser::parse_tensor_literal()
    {
        const Token& start_token = cursor_.advance();
        CloserTracker tracker(*this, TokenType::RightBracket);
        auto elements = parse_expression_list(TokenType::RightBracket);

        try
        {
            const Token& end_token = cursor_.consume(TokenType::RightBracket,
                                                     ValuascriptErrorCode::UnmatchedBracketAfterTensorElements);
            return make_node_with_span<TensorLiteral>(cursor_.make_span(start_token, end_token), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightBracket);
            return make_node<TensorLiteral>(start_token, std::move(elements));
        }
    }

    std::unique_ptr<Expression> Parser::parse_dict_literal()
    {
        const Token& start_token = cursor_.advance();
        CloserTracker tracker(*this, TokenType::RightBrace);

        ParameterRuleSpec dict_spec{
            .allow_modifiers = true,
            .allow_type = false,
            .allow_value = true,
            .require_value = true,
            .value_separator = TokenType::Colon,
            .missing_name_err = ValuascriptErrorCode::ExpectedDictionaryKey,
            .missing_value_separator_err = ValuascriptErrorCode::ExpectedColonAfterDictionaryKey,
            .missing_value_err = ValuascriptErrorCode::InvalidExpression
        };

        auto items_gen = parse_list<GenericParameter>(
            TokenType::RightBrace,
            std::nullopt,
            ValuascriptErrorCode::ExpectedCommaSeparatorInDictionaryLiteral,
            std::vector<TokenType>{},
            [this]()
            {
                const Token& tok = cursor_.peek();
                const TokenType next = cursor_.peek(1).type;

                if (tok.type == TokenType::At)
                {
                    if (is_at_any_declaration()) return false;
                    return true;
                }

                if (tok.type == TokenType::At || tok.type == TokenType::Identifier) return true;

                return is_reserved_keyword(tok) && (next == TokenType::Colon);
            },
            [&]() { return parse_generic_parameter(dict_spec); }
        );

        std::vector<DictItem> elements;
        elements.reserve(items_gen.size());
        for (auto& g : items_gen)
        {
            elements.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.value)});
        }

        try
        {
            const Token& end_token = cursor_.consume(TokenType::RightBrace,
                                                     ValuascriptErrorCode::UnmatchedBraceInDictionaryLiteral);
            return make_node_with_span<DictLiteral>(cursor_.make_span(start_token, end_token), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightBrace);
            return make_node<DictLiteral>(start_token, std::move(elements));
        }
    }

    std::unique_ptr<Expression> Parser::parse_conditional_expression()
    {
        const Token& start_token = cursor_.advance();
        SyncSetTracker tracker(*this, {TokenType::Then, TokenType::Else});

        auto condition = attempt_parse<std::unique_ptr<Expression>>(
            [&]() { return parse_expression(); },
            {
                .stop_tokens = {
                    TokenType::Then, TokenType::Else,
                    TokenType::RightParen, TokenType::RightBracket, TokenType::RightBrace
                },
                .stop_at_currently_tracked_closers = false,
                .stop_at_currently_tracked_sync_tokens = false,
                .stop_early_if_unbalanced_blocks_detected = true
            },
            nullptr
        );

        if (!cursor_.match({TokenType::Then}))
        {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingThenToken);
        }

        auto then_branch = attempt_parse<std::unique_ptr<Expression>>(
            [&]() { return parse_expression(); },
            {
                .stop_tokens = {
                    TokenType::Then, TokenType::Else,
                    TokenType::RightParen, TokenType::RightBracket, TokenType::RightBrace
                },
                .stop_at_currently_tracked_closers = false,
                .stop_at_currently_tracked_sync_tokens = false,
                .stop_early_if_unbalanced_blocks_detected = true
            },
            nullptr
        );

        std::unique_ptr<Expression> else_branch = nullptr;
        if (!cursor_.match({TokenType::Else}))
        {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingElseToken);
        }

        else_branch = parse_expression();

        return make_node<ConditionalExpression>(start_token, std::move(condition), std::move(then_branch),
                                                std::move(else_branch));
    }

    std::unique_ptr<Expression> Parser::parse_switch_expression()
    {
        const Token& start_token = cursor_.advance();
        auto target = parse_switch_target();

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeSwitchBody);
        CloserTracker tracker(*this, TokenType::RightBrace);

        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression>>> cases;
        std::unique_ptr<Expression> default_case = nullptr;

        parse_switch_body(cases, default_case);

        try
        {
            const Token& end_token = cursor_.consume(TokenType::RightBrace,
                                                     ValuascriptErrorCode::ExpectedRightBraceAfterSwitchBody);
            return make_node_with_span<SwitchExpression>(cursor_.make_span(start_token, end_token), std::move(target),
                                                         std::move(cases), std::move(default_case));
        }
        catch (const ParseSyncException&)
        {
            synchronize_and_consume_closer(TokenType::RightBrace);
            return make_node<SwitchExpression>(start_token, std::move(target), std::move(cases),
                                               std::move(default_case));
        }
    }

    std::pair<std::vector<std::string>, std::unique_ptr<Expression>> Parser::parse_switch_case()
    {
        std::vector<std::string> identifiers;

        while (true)
        {
            const Token& tok = cursor_.peek();
            if (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, cursor_.peek(1).type))
            {
                auto id_token = attempt_parse<Token>(
                    [&]() { return consume_identifier(ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase); },
                    {
                        .stop_tokens = {TokenType::Comma, TokenType::Arrow, TokenType::RightBrace},
                        .stop_at_statement_boundary_respecting_dangling_op = true,
                        .stop_at_currently_tracked_closers = false,
                        .stop_at_currently_tracked_sync_tokens = false
                    },
                    Token(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column)
                );
                identifiers.push_back(id_token.lexeme);
            }
            else
            {
                cursor_.report_error_no_panic(tok, ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase, true);
                if (!cursor_.check(TokenType::Comma) && !cursor_.check(TokenType::Arrow) && !cursor_.check(
                    TokenType::RightBrace))
                {
                    synchronize_with({
                        .stop_tokens = {TokenType::Comma, TokenType::Arrow, TokenType::RightBrace},
                        .stop_at_statement_boundary_respecting_dangling_op = true,
                        .stop_at_currently_tracked_closers = false,
                        .stop_at_currently_tracked_sync_tokens = false
                    });
                }
            }

            if (cursor_.match({TokenType::Comma})) continue;
            if (cursor_.check(TokenType::Identifier))
            {
                cursor_.report_error_no_panic(cursor_.peek(),
                                              ValuascriptErrorCode::ExpectedCommaBetweenCaseIdentifiers);
                continue;
            }
            break;
        }

        auto result = attempt_parse<std::unique_ptr<Expression>>(
            [&]() { return parse_switch_result(); },
            {
                .stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace},
                .stop_at_currently_tracked_closers = false,
                .stop_at_currently_tracked_sync_tokens = false,
                .stop_early_if_unbalanced_blocks_detected = true
            },
            nullptr
        );

        return {std::move(identifiers), std::move(result)};
    }

    std::unique_ptr<Expression> Parser::parse_switch_default()
    {
        auto result = attempt_parse<std::unique_ptr<Expression>>(
            [&]() { return parse_switch_result(); },
            {
                .stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace},
                .stop_at_currently_tracked_closers = false,
                .stop_at_currently_tracked_sync_tokens = false,
                .stop_early_if_unbalanced_blocks_detected = true
            },
            nullptr
        );
        return result;
    }

    std::unique_ptr<Expression> Parser::parse_switch_result()
    {
        cursor_.consume(TokenType::Arrow, ValuascriptErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier);
        auto expr = parse_expression();

        bool should_break_out = is_missing_closing_brace() &&
        (is_at_top_level_declaration() || cursor_.peek().type == TokenType::Return ||
            TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
            (cursor_.peek().line > cursor_.previous().line &&
                TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type)));

        if (!should_break_out && !cursor_.check(TokenType::Case) && !cursor_.check(TokenType::Default) &&
            !cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end())
        {
            if (cursor_.peek().line == cursor_.previous().line)
            {
                if (TokenTraits::is_expression_start(cursor_.peek().type))
                {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::MissingOperatorInSwitchCaseResult, true);
                }
                else
                {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::CaseOrDefaultMissingInSwitchAfterResult, true);
                }
            }
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_switch_target()
    {
        bool failed = false;
        auto target = attempt_parse<std::unique_ptr<Expression>>(
            [&]()
            {
                cursor_.consume(TokenType::LeftParen, ValuascriptErrorCode::ExpectedLeftParenAfterSwitch);
                CloserTracker tracker(*this, TokenType::RightParen);
                auto t = parse_expression();
                cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::ExpectedRightParenAfterSwitchTarget);
                return t;
            },
            {
                .stop_tokens = {TokenType::RightParen, TokenType::LeftBrace},
                .stop_at_currently_tracked_closers = false,
                .stop_at_currently_tracked_sync_tokens = false
            },
            nullptr,
            &failed
        );

        if (failed)
        {
            if (cursor_.check(TokenType::RightParen))
            {
                cursor_.advance();
            }
            else if (TokenTraits::is_grouping_closer(cursor_.peek().type))
            {
                if (!is_active_closer(cursor_.peek().type)) cursor_.advance();
            }
        }
        return target;
    }

    void Parser::parse_switch_body(
        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression>>>& cases,
        std::unique_ptr<Expression>& default_case)
    {
        SyncSetTracker tracker(*this, {TokenType::Case, TokenType::Default});

        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end())
        {
            bool should_break_out = is_missing_closing_brace() &&
            (is_at_top_level_declaration() || cursor_.peek().type == TokenType::Return ||
                TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                (cursor_.peek().line > cursor_.previous().line &&
                    TokenTraits::is_expression_statement_start(
                        cursor_.peek(), cursor_.peek(1).type)));

            if (should_break_out) break;

            attempt_parse_void(
                [&]()
                {
                    if (cursor_.match({TokenType::Case}))
                    {
                        cases.push_back(parse_switch_case());
                    }
                    else if (cursor_.match({TokenType::Default}))
                    {
                        if (default_case != nullptr)
                        {
                            cursor_.report_error_no_panic(cursor_.previous(),
                                                          ValuascriptErrorCode::MultipleDefaultCasesInSwitch);
                        }
                        default_case = parse_switch_default();
                    }
                    else
                    {
                        if (TokenTraits::is_top_level_token(cursor_.peek().type))
                        {
                            const Token& start_tok = cursor_.peek();
                            consume_unexpected_statement_gracefully();
                            SourceSpan span = cursor_.make_span(start_tok, cursor_.previous());
                            cursor_.report_error_no_panic(
                                span, ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere);
                            throw ParseSyncException();
                        }
                        else
                        {
                            cursor_.report_error(cursor_.peek(),
                                                 ValuascriptErrorCode::ExpectedCaseOrDefaultInsideSwitchBody, true);
                        }
                    }
                },
                {
                    .stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace},
                    .stop_at_currently_tracked_closers = false,
                    .stop_at_currently_tracked_sync_tokens = false,
                    .stop_early_if_unbalanced_blocks_detected = true
                }
            );
        }
    }
}