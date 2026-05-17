#include "expression_parser.h"
#include "parser.h"
#include "declaration_parser.h"
#include "token/reserved_keyword_lookup.h"
#include "token/operator_lookup.h"
#include <algorithm>
#include <array>

namespace valuascript::compiler
{
    using E = ValuascriptErrorCode;

    ExpressionParser::ExpressionParser(Parser& p) : parser(p), ctx(p.ctx), cursor(p.ctx.cursor)
    {
    }

    ExpressionParser::ParseRule ExpressionParser::get_rule(TokenType type)
    {
        static const std::array<ParseRule, 256> rules = []
        {
            std::array<ParseRule, 256> r{};
            auto set_prefix = [&r](TokenType t, PrefixParseFn pre) { r[static_cast<size_t>(t)].prefix = pre; };
            auto set_infix = [&r](TokenType t, InfixParseFn in, Precedence prec, bool is_right_assoc = false)
            {
                auto idx = static_cast<size_t>(t);
                r[idx].infix = in;
                r[idx].precedence = prec;
                r[idx].is_right_associative = is_right_assoc;
            };

            for (const auto& [token, lexeme] : get_all_unary_operators())
            {
                set_prefix(token, &ExpressionParser::parse_prefix_unary);
            }

            for (const auto& [token, lexeme] : get_all_binary_operators())
            {
                auto [prec, is_right] = TokenTraits::get_binary_op_info(token);
                if (prec != Precedence::None) set_infix(token, &ExpressionParser::parse_infix_binary, prec, is_right);
            }

            set_infix(TokenType::LeftParen, &ExpressionParser::parse_function_call, Precedence::Postfix);
            set_infix(TokenType::LeftBracket, &ExpressionParser::parse_tensor_access, Precedence::Postfix);
            set_infix(TokenType::Dot, &ExpressionParser::parse_dot_access, Precedence::Postfix);

            set_prefix(TokenType::LeftParen, &ExpressionParser::parse_tuple_or_grouping);
            set_prefix(TokenType::LeftBracket, &ExpressionParser::parse_tensor_literal);
            set_prefix(TokenType::LeftBrace, &ExpressionParser::parse_dict_literal);

            set_prefix(TokenType::Number, &ExpressionParser::parse_literal_prefix<NumberLiteral>);
            set_prefix(TokenType::PercentageLiteral, &ExpressionParser::parse_literal_prefix<PercentageLiteral>);
            set_prefix(TokenType::String, &ExpressionParser::parse_literal_prefix<StringLiteral>);
            set_prefix(TokenType::True, &ExpressionParser::parse_literal_prefix<BooleanLiteral>);
            set_prefix(TokenType::False, &ExpressionParser::parse_literal_prefix<BooleanLiteral>);
            set_prefix(TokenType::Identifier, &ExpressionParser::parse_literal_prefix<IdentifierAccess>);
            set_prefix(TokenType::Self, &ExpressionParser::parse_literal_prefix<SelfExpression>);

            set_prefix(TokenType::Switch, &ExpressionParser::parse_switch_expression);
            set_prefix(TokenType::If, &ExpressionParser::parse_conditional_expression);

            return r;
        }();

        auto idx = static_cast<size_t>(type);
        if (idx < rules.size()) return rules[idx];
        return {nullptr, nullptr, Precedence::None, false};
    }

    std::unique_ptr<Expression> ExpressionParser::parse_expression(const Precedence min_precedence)
    {
        const Token& start_tok = cursor.peek();
        ParseRule rule = get_rule(start_tok.type);

        if (rule.prefix == nullptr) return handle_invalid_expression_start();

        auto left = (this->*(rule.prefix))();

        while (true)
        {
            const Token& op_tok = cursor.peek();
            bool inside_expr_grouping = std::any_of(
                ctx.active_closers.begin(),
                ctx.active_closers.end(),
                [](TokenType t) { return t == TokenType::RightParen || t == TokenType::RightBracket; }
            );

            if (op_tok.line > cursor.previous().line && !inside_expr_grouping)
            {
                if (!TokenTraits::is_postfix_operator(op_tok.type)) break;
            }

            ParseRule infix_rule = get_rule(op_tok.type);
            if (infix_rule.precedence < min_precedence || infix_rule.precedence == Precedence::None) break;
            if (infix_rule.infix == nullptr) break;

            Token op = cursor.advance();

            if (!inside_expr_grouping && infix_rule.infix == &ExpressionParser::parse_infix_binary)
            {
                const Token& next = cursor.peek();
                if (next.type == TokenType::EndOfFile || (next.line > op.line && (
                    TokenTraits::is_statement_start(next, cursor.peek(1).type) ||
                    TokenTraits::is_expression_statement_start(next, cursor.peek(1).type))))
                {
                    cursor.report_error_no_panic(op, E::InvalidExpression);
                    return ctx.make_node_with_span<BinaryExpression>(
                        cursor.combine_spans(left->span, cursor.make_span(op, op)), std::move(left), op.type, nullptr);
                }
            }

            left = (this->*(infix_rule.infix))(std::move(left), op);

            if (infix_rule.precedence == Precedence::Comparison && get_rule(cursor.peek().type).precedence ==
                Precedence::Comparison)
            {
                cursor.report_error_no_panic(cursor.peek(),
                                             E::ChainingNotAllowedForComparisonOperations);
            }
        }
        return left;
    }

    std::unique_ptr<Expression> ExpressionParser::parse_infix_binary(std::unique_ptr<Expression> left, const Token& op)
    {
        ParseRule infix_rule = get_rule(op.type);
        Precedence next_precedence = infix_rule.is_right_associative
                                         ? infix_rule.precedence
                                         : static_cast<Precedence>(static_cast<int>(infix_rule.precedence) + 1);

        auto right = ctx.try_parse<std::unique_ptr<Expression>>(
            [&]() { return parse_expression(next_precedence); },
            RecoveryConfig::ForceStopAtBoundary({
                TokenType::Comma, TokenType::RightParen,
                TokenType::RightBracket, TokenType::RightBrace
            })
        );

        const SourceSpan right_span = right ? right->span : cursor.make_span(cursor.previous(), cursor.previous());
        return ctx.make_node_with_span<BinaryExpression>(cursor.combine_spans(left->span, right_span), std::move(left),
                                                         op.type, std::move(right));
    }

    std::unique_ptr<Expression> ExpressionParser::parse_prefix_unary()
    {
        Token op = cursor.advance();
        bool inside_expr_grouping = std::any_of(
            ctx.active_closers.begin(),
            ctx.active_closers.end(),
            [](TokenType t) { return t == TokenType::RightParen || t == TokenType::RightBracket; }
        );

        if (!inside_expr_grouping)
        {
            const Token& next = cursor.peek();
            if (next.type == TokenType::EndOfFile || (next.line > op.line && (
                TokenTraits::is_statement_start(next, cursor.peek(1).type) ||
                TokenTraits::is_expression_statement_start(next, cursor.peek(1).type))))
            {
                cursor.report_error_no_panic(op, E::InvalidExpression);
                return ctx.make_node_with_span<UnaryExpression>(cursor.make_span(op, op), op.type, nullptr);
            }
        }

        auto right = ctx.try_parse<std::unique_ptr<Expression>>(
            [&] { return parse_expression(Precedence::Unary); },
            RecoveryConfig::ForceStopAtBoundary({
                TokenType::Comma, TokenType::RightParen,
                TokenType::RightBracket, TokenType::RightBrace
            })
        );

        return ctx.make_node<UnaryExpression>(op, op.type, std::move(right));
    }

    std::unique_ptr<Expression> ExpressionParser::handle_invalid_expression_start()
    {
        const Token& tok = cursor.peek();
        const Token& next = cursor.peek(1);
        const Token& prev = cursor.previous();

        bool is_stmt_start = TokenTraits::is_statement_start(tok, next.type);
        bool force_location = (tok.type != TokenType::EndOfFile && !is_stmt_start);

        if (is_stmt_start)
        {
            if (tok.line > prev.line)
            {
                if (TokenTraits::is_dangling_operator(prev.type) || TokenTraits::is_grouping_opener(prev.type))
                    cursor.
                        report_error(prev, E::InvalidExpression);
                else cursor.report_error(tok, E::InvalidExpression, force_location);
            }

            const Token& start_tok = cursor.peek();
            if (tok.type == TokenType::At && !ctx.is_at_any_declaration())
            {
                parser.decl_parser->parse_modifiers();
                SourceSpan span = cursor.make_span(start_tok, cursor.previous());
                cursor.report_error_no_panic(span, E::TopLevelDeclarationNotAllowedHere);
                cursor.report_error_no_panic(span, E::ModifiersAttachedToInvalidDeclaration);
            }
            else
            {
                parser.consume_unexpected_statement_gracefully();
                SourceSpan span = cursor.make_span(start_tok, cursor.previous());
                cursor.report_error_no_panic(span, E::TopLevelDeclarationNotAllowedHere);
            }

            if (TokenTraits::is_expression_start(cursor.peek().type)) return parse_expression();
            throw ParseSyncException();
        }

        if (tok.type == TokenType::Case || tok.type == TokenType::Default || tok.type == TokenType::RightBrace || tok.
            type == TokenType::RightParen ||
            tok.type == TokenType::RightBracket || tok.type == TokenType::Return || tok.type == TokenType::Then || tok.
            type == TokenType::Else)
        {
            if (tok.line > prev.line)
            {
                if (TokenTraits::is_dangling_operator(prev.type) || TokenTraits::is_grouping_opener(prev.type))
                    cursor.
                        report_error(prev, E::InvalidExpression);
            }
            cursor.report_error(tok, E::InvalidExpression, force_location);
        }

        if (is_reserved_keyword(tok))
        {
            cursor.report_error_no_panic(tok, E::ReservedKeywordAsIdentifier, true);
            cursor.advance();
            return ctx.make_node_with_span<IdentifierAccess>(cursor.make_span(tok, tok), tok.lexeme);
        }

        cursor.report_error(tok, E::InvalidExpression, force_location);
    }

    std::unique_ptr<Expression> ExpressionParser::parse_function_call(std::unique_ptr<Expression> target,
                                                                      const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;
        ParserContext::CloserTracker tracker(ctx, TokenType::RightParen);
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> arguments;

        try
        {
            if (!cursor.check(TokenType::RightParen) && !cursor.is_at_end())
            {
                const Token& p0 = cursor.peek();
                const TokenType p1 = cursor.peek(1).type;

                if (p0.type == TokenType::Identifier || TokenTraits::acts_like_identifier(p0, p1))
                {
                    if (p1 != TokenType::Colon && TokenTraits::is_binary_operator(p1))
                    {
                        if (!TokenTraits::is_newline_statement_boundary(cursor.previous(), p0, p1))
                            cursor.report_error(
                                cursor.previous(), E::MissingOperatorOrArgumentName);
                    }
                }
                else
                {
                    if (TokenTraits::is_expression_start(p0.type))
                    {
                        if (!TokenTraits::is_newline_statement_boundary(cursor.previous(), p0, p1))
                            cursor.report_error(
                                cursor.previous(), E::MissingOperatorOrArgumentName);
                    }
                    else if (!cursor.check(TokenType::Colon) && !cursor.check(TokenType::Comma))
                    {
                        cursor.report_error(cursor.peek(), E::ExpectedArgumentNameOrClosingParen);
                    }
                }
            }

            ParameterRuleSpec arg_spec{
                .allow_value = true,
                .require_value = true,
                .value_separator = TokenType::Colon,
                .missing_name_err = E::MissingArgumentNameInFunctionCall,
                .missing_value_separator_err = E::MissingColonAfterArgument,
                .missing_value_err = E::InvalidExpression
            };
            auto args_gen = ctx.parse_list<GenericParameter>(
                TokenType::RightParen,
                E::TrailingCommaInFunctionCall,
                E::MissingCommaSeparatorForArgumentsInFunctionCall,
                {},
                [this]()
                {
                    const Token& tok = cursor.peek();
                    return (tok.type == TokenType::Identifier ||
                            TokenTraits::acts_like_identifier(tok, cursor.peek(1).type)) && cursor.peek(1).type ==
                        TokenType::Colon;
                },
                [&]() { return parser.decl_parser->parse_generic_parameter(arg_spec); }
            );

            for (auto& g : args_gen) arguments.emplace_back(g.name.lexeme, std::move(g.value));

            const Token& end_token = cursor.consume(TokenType::RightParen,
                                                    E::ExpectedRightParenAfterArguments);
            return ctx.make_node_with_span<FunctionCall>(
                cursor.combine_spans(target_span, cursor.make_span(end_token, end_token)), std::move(target),
                std::move(arguments));
        }
        catch (const ParseSyncException&)
        {
            ctx.synchronize_and_consume_closer(TokenType::RightParen);
            return ctx.make_node_with_span<FunctionCall>(
                cursor.combine_spans(target_span, cursor.make_span(cursor.previous(), cursor.previous())),
                std::move(target), std::move(arguments));
        }
    }

    std::unique_ptr<Expression> ExpressionParser::parse_tensor_access(std::unique_ptr<Expression> target,
                                                                      const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;
        ParserContext::CloserTracker tracker(ctx, TokenType::RightBracket);
        std::unique_ptr<Expression> index_expr = nullptr;

        try
        {
            auto parse_bound = [&]() -> std::unique_ptr<Expression>
            {
                if (cursor.check(TokenType::Colon) || cursor.check(TokenType::RightBracket)) return nullptr;
                auto expr = parse_expression();
                if (!cursor.check(TokenType::Colon) && !cursor.check(TokenType::RightBracket))
                {
                    if (!TokenTraits::is_newline_statement_boundary(cursor.previous(), cursor.peek(),
                                                                    cursor.peek(1).type))
                    {
                        if (TokenTraits::is_expression_start(cursor.peek().type))
                            cursor.report_error(
                                cursor.peek(), E::MissingOperatorOrExpectedColonOrBracketInTensor);
                        else if (cursor.check(TokenType::Comma))
                            cursor.report_error(
                                cursor.peek(), E::UnexpectedCommaInBracketAccess);
                    }
                }
                return expr;
            };

            index_expr = parse_bound();

            if (cursor.match({TokenType::Colon}))
            {
                std::unique_ptr<Expression> end_expr = parse_bound();
                const SourceSpan colon_span = index_expr ? index_expr->span : target_span;
                const SourceSpan slice_end_span = end_expr
                                                      ? end_expr->span
                                                      : cursor.make_span(cursor.previous(), cursor.previous());
                index_expr = ctx.make_node_with_span<BinaryExpression>(cursor.combine_spans(colon_span, slice_end_span),
                                                                       std::move(index_expr), TokenType::Colon,
                                                                       std::move(end_expr));
            }
            else if (!index_expr)
            {
                cursor.report_error(cursor.previous(), E::EmptyBracketAccess);
            }

            const Token& end_token = cursor.consume(TokenType::RightBracket,
                                                    E::UnmatchedBracketAfterTensorIndex);
            return ctx.make_node_with_span<BracketAccess>(
                cursor.combine_spans(target_span, cursor.make_span(end_token, end_token)), std::move(target),
                std::move(index_expr));
        }
        catch (const ParseSyncException&)
        {
            ctx.synchronize_and_consume_closer(TokenType::RightBracket);
            return ctx.make_node_with_span<BracketAccess>(
                cursor.combine_spans(target_span, cursor.make_span(cursor.previous(), cursor.previous())),
                std::move(target), std::move(index_expr));
        }
    }

    std::unique_ptr<Expression> ExpressionParser::parse_dot_access(std::unique_ptr<Expression> target,
                                                                   const Token& /*op*/)
    {
        Token property_token = ctx.try_consume_identifier(E::ExpectedPropertyName,
                                                          RecoveryConfig::ForceStopAtBoundary({
                                                              TokenType::Assign, TokenType::Comma,
                                                              TokenType::RightParen, TokenType::RightBracket,
                                                              TokenType::RightBrace
                                                          }), true, true);
        return ctx.make_node_with_span<DotAccess>(
            cursor.combine_spans(target->span, cursor.make_span(property_token, property_token)), std::move(target),
            property_token.lexeme);
    }

    std::unique_ptr<Expression> ExpressionParser::parse_tuple_or_grouping()
    {
        const Token& start = cursor.advance();
        ParserContext::CloserTracker tracker(ctx, TokenType::RightParen);

        if (cursor.match({TokenType::RightParen}))
            return ctx.make_node<TupleLiteral>(
                start, std::vector<std::unique_ptr<Expression>>{});

        bool failed = false;

        RecoveryConfig conf;
        conf.stop_tokens = {TokenType::Comma, TokenType::RightParen};
        conf.stop_at_currently_tracked_closers = false;
        conf.stop_at_currently_tracked_sync_tokens = false;

        auto first_expr = ctx.try_parse<std::unique_ptr<Expression>>(
            [&]() { return parse_expression(); },
            conf,
            &failed
        );

        if (cursor.match({TokenType::Comma})) return complete_tuple(std::move(first_expr), start);
        return complete_grouping(std::move(first_expr), failed, start);
    }

    std::unique_ptr<Expression> ExpressionParser::complete_tuple(std::unique_ptr<Expression> first_expr,
                                                                 const Token& start)
    {
        if (cursor.check(TokenType::RightParen))
            cursor.report_error_no_panic(
                cursor.previous(), E::SingleElementTuplesNotAllowed);

        std::vector<std::unique_ptr<Expression>> elements;
        if (first_expr) elements.push_back(std::move(first_expr));

        auto remaining = parse_expression_list(TokenType::RightParen, E::TrailingCommaInTuple);
        elements.insert(elements.end(), std::make_move_iterator(remaining.begin()),
                        std::make_move_iterator(remaining.end()));

        try
        {
            const Token& end = cursor.consume(TokenType::RightParen,
                                              E::ExpectedRightParenAfterTupleElements);
            return ctx.make_node_with_span<TupleLiteral>(cursor.make_span(start, end), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            ctx.synchronize_and_consume_closer(TokenType::RightParen);
            return ctx.make_node<TupleLiteral>(start, std::move(elements));
        }
    }

    std::unique_ptr<Expression> ExpressionParser::complete_grouping(std::unique_ptr<Expression> first_expr, bool failed,
                                                                    const Token& start)
    {
        try
        {
            if (!failed && !cursor.check(TokenType::RightParen) && TokenTraits::is_expression_start(cursor.peek().type))
            {
                if (!TokenTraits::is_newline_statement_boundary(cursor.previous(), cursor.peek(), cursor.peek(1).type))
                {
                    cursor.report_error(cursor.peek(), E::MissingOperatorInsideGrouping);
                }
            }
            const Token& end = cursor.consume(TokenType::RightParen,
                                              E::ExpectedRightParenAfterExpression);
            return ctx.make_node_with_span<GroupingExpression>(cursor.make_span(start, end), std::move(first_expr));
        }
        catch (const ParseSyncException&)
        {
            ctx.synchronize_and_consume_closer(TokenType::RightParen);
            return ctx.make_node<GroupingExpression>(start, std::move(first_expr));
        }
    }

    std::unique_ptr<Expression> ExpressionParser::parse_tensor_literal()
    {
        const Token& start = cursor.advance();
        ParserContext::CloserTracker tracker(ctx, TokenType::RightBracket);
        auto elements = parse_expression_list(TokenType::RightBracket);

        try
        {
            const Token& end = cursor.consume(TokenType::RightBracket,
                                              E::UnmatchedBracketAfterTensorElements);
            return ctx.make_node_with_span<TensorLiteral>(cursor.make_span(start, end), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            ctx.synchronize_and_consume_closer(TokenType::RightBracket);
            return ctx.make_node<TensorLiteral>(start, std::move(elements));
        }
    }

    std::unique_ptr<Expression> ExpressionParser::parse_dict_literal()
    {
        const Token& start = cursor.advance();
        ParserContext::CloserTracker tracker(ctx, TokenType::RightBrace);

        ParameterRuleSpec dict_spec{
            .allow_modifiers = true,
            .allow_value = true,
            .require_value = true,
            .value_separator = TokenType::Colon,
            .missing_name_err = E::ExpectedDictionaryKey,
            .missing_value_separator_err = E::ExpectedColonAfterDictionaryKey,
            .missing_value_err = E::InvalidExpression
        };

        auto items_gen = ctx.parse_list<GenericParameter>(
            TokenType::RightBrace,
            std::nullopt,
            E::ExpectedCommaSeparatorInDictionaryLiteral,
            {},
            [this]()
            {
                const Token& tok = cursor.peek();
                if (tok.type == TokenType::At) return !ctx.is_at_any_declaration();
                if (tok.type == TokenType::Identifier) return true;
                return is_reserved_keyword(tok) && (cursor.peek(1).type == TokenType::Colon);
            },
            [&]() { return parser.decl_parser->parse_generic_parameter(dict_spec); }
        );

        std::vector<DictItem> elements;
        for (auto& g : items_gen) elements.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.value)});

        try
        {
            const Token& end = cursor.consume(TokenType::RightBrace,
                                              E::UnmatchedBraceInDictionaryLiteral);
            return ctx.make_node_with_span<DictLiteral>(cursor.make_span(start, end), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            ctx.synchronize_and_consume_closer(TokenType::RightBrace);
            return ctx.make_node<DictLiteral>(start, std::move(elements));
        }
    }

    std::unique_ptr<Expression> ExpressionParser::parse_conditional_expression()
    {
        const Token& start = cursor.advance();
        ParserContext::SyncSetTracker tracker(ctx, {TokenType::Then, TokenType::Else});

        RecoveryConfig conf;
        conf.stop_tokens = {
            TokenType::Then, TokenType::Else, TokenType::RightParen, TokenType::RightBracket, TokenType::RightBrace
        };
        conf.stop_at_currently_tracked_closers = false;
        conf.stop_at_currently_tracked_sync_tokens = false;
        conf.stop_early_if_unbalanced_blocks_detected = true;

        auto condition = ctx.try_parse<std::unique_ptr<Expression>>([&]() { return parse_expression(); }, conf);
        if (!cursor.match({TokenType::Then}))
            cursor.report_error_no_panic(
                cursor.peek(), E::MissingThenToken);

        auto then_branch = ctx.try_parse<std::unique_ptr<Expression>>([&]() { return parse_expression(); }, conf);
        if (!cursor.match({TokenType::Else}))
            cursor.report_error_no_panic(
                cursor.peek(), E::MissingElseToken);

        std::unique_ptr<Expression> else_branch = parse_expression();
        return ctx.make_node<ConditionalExpression>(start, std::move(condition), std::move(then_branch),
                                                    std::move(else_branch));
    }

    std::unique_ptr<Expression> ExpressionParser::parse_switch_expression()
    {
        const Token& start = cursor.advance();
        auto target = parse_switch_target();

        cursor.consume(TokenType::LeftBrace, E::ExpectedLeftBraceBeforeSwitchBody);
        ParserContext::CloserTracker tracker(ctx, TokenType::RightBrace);

        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression>>> cases;
        std::unique_ptr<Expression> default_case = nullptr;
        parse_switch_body(cases, default_case);

        try
        {
            const Token& end = cursor.consume(TokenType::RightBrace, E::ExpectedRightBraceAfterSwitchBody);
            return ctx.make_node_with_span<SwitchExpression>(cursor.make_span(start, end), std::move(target),
                                                             std::move(cases), std::move(default_case));
        }
        catch (const ParseSyncException&)
        {
            ctx.synchronize_and_consume_closer(TokenType::RightBrace);
            return ctx.make_node<SwitchExpression>(start, std::move(target), std::move(cases), std::move(default_case));
        }
    }

    std::pair<std::vector<std::string>, std::unique_ptr<Expression>> ExpressionParser::parse_switch_case()
    {
        std::vector<std::string> identifiers;

        while (true)
        {
            const Token& tok = cursor.peek();
            if (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, cursor.peek(1).type))
            {
                RecoveryConfig conf;
                conf.stop_tokens = {TokenType::Comma, TokenType::Arrow, TokenType::RightBrace};
                conf.stop_at_statement_boundary_respecting_dangling_op = true;
                conf.stop_at_currently_tracked_closers = false;
                conf.stop_at_currently_tracked_sync_tokens = false;
                Token id = ctx.try_consume_identifier(E::ExpectedEnumCaseNameAfterCase, conf);
                identifiers.push_back(id.lexeme);
            }
            else
            {
                cursor.report_error_no_panic(tok, E::ExpectedEnumCaseNameAfterCase, true);
                if (!cursor.check(TokenType::Comma) && !cursor.check(TokenType::Arrow) && !cursor.check(
                    TokenType::RightBrace))
                {
                    RecoveryConfig conf;
                    conf.stop_tokens = {TokenType::Comma, TokenType::Arrow, TokenType::RightBrace};
                    conf.stop_at_statement_boundary_respecting_dangling_op = true;
                    conf.stop_at_currently_tracked_closers = false;
                    conf.stop_at_currently_tracked_sync_tokens = false;
                    ctx.synchronize_with(conf);
                }
            }

            if (cursor.match({TokenType::Comma})) continue;
            if (cursor.check(TokenType::Identifier))
            {
                cursor.report_error_no_panic(cursor.peek(), E::ExpectedCommaBetweenCaseIdentifiers);
                continue;
            }
            break;
        }

        RecoveryConfig conf;
        conf.stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace};
        conf.stop_at_currently_tracked_closers = false;
        conf.stop_at_currently_tracked_sync_tokens = false;
        conf.stop_early_if_unbalanced_blocks_detected = true;
        auto result = ctx.try_parse<std::unique_ptr<Expression>>([&]() { return parse_switch_result(); }, conf);
        return {std::move(identifiers), std::move(result)};
    }

    std::unique_ptr<Expression> ExpressionParser::parse_switch_default()
    {
        RecoveryConfig conf;
        conf.stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace};
        conf.stop_at_currently_tracked_closers = false;
        conf.stop_at_currently_tracked_sync_tokens = false;
        conf.stop_early_if_unbalanced_blocks_detected = true;
        return ctx.try_parse<std::unique_ptr<Expression>>([&]() { return parse_switch_result(); }, conf);
    }

    std::unique_ptr<Expression> ExpressionParser::parse_switch_result()
    {
        cursor.consume(TokenType::Arrow, E::ExpectedRightArrowAfterSwitchCaseIdentifier);
        auto expr = parse_expression();

        bool should_break_out = ctx.is_missing_closing_brace() && (ctx.is_at_top_level_declaration() || cursor.peek().
            type == TokenType::Return || TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) || (cursor.
                peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), cursor.peek(1).type)));

        if (!should_break_out && !cursor.check(TokenType::Case) && !cursor.check(TokenType::Default) && !cursor.
            check(TokenType::RightBrace) && !cursor.is_at_end())
        {
            if (cursor.peek().line == cursor.previous().line)
            {
                if (TokenTraits::is_expression_start(cursor.peek().type))
                    cursor.report_error_no_panic(
                        cursor.peek(), E::MissingOperatorInSwitchCaseResult, true);
                else
                    cursor.report_error_no_panic(cursor.peek(), E::CaseOrDefaultMissingInSwitchAfterResult, true);
            }
        }
        return expr;
    }

    std::unique_ptr<Expression> ExpressionParser::parse_switch_target()
    {
        bool failed = false;
        RecoveryConfig conf;
        conf.stop_tokens = {TokenType::RightParen, TokenType::LeftBrace};
        conf.stop_at_currently_tracked_closers = false;
        conf.stop_at_currently_tracked_sync_tokens = false;

        auto target = ctx.try_parse<std::unique_ptr<Expression>>(
            [&]()
            {
                cursor.consume(TokenType::LeftParen, E::ExpectedLeftParenAfterSwitch);
                ParserContext::CloserTracker tracker(ctx, TokenType::RightParen);
                auto t = parse_expression();
                cursor.consume(TokenType::RightParen, E::ExpectedRightParenAfterSwitchTarget);
                return t;
            }, conf, &failed
        );

        if (failed)
        {
            if (cursor.check(TokenType::RightParen)) cursor.advance();
            else if (TokenTraits::is_grouping_closer(cursor.peek().type))
            {
                if (!ctx.is_active_closer(cursor.peek().type)) cursor.advance();
            }
        }
        return target;
    }

    void ExpressionParser::parse_switch_body(
        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression>>>& cases,
        std::unique_ptr<Expression>& default_case)
    {
        ParserContext::SyncSetTracker tracker(ctx, {TokenType::Case, TokenType::Default});

        while (!cursor.check(TokenType::RightBrace) && !cursor.is_at_end())
        {
            bool should_break_out = ctx.is_missing_closing_brace() && (ctx.is_at_top_level_declaration() || cursor.
                peek().type == TokenType::Return || TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type)
                || (cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), cursor.peek(1).type)));
            if (should_break_out) break;

            RecoveryConfig conf;
            conf.stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace};
            conf.stop_at_currently_tracked_closers = false;
            conf.stop_at_currently_tracked_sync_tokens = false;
            conf.stop_early_if_unbalanced_blocks_detected = true;
            ctx.attempt_parse_void(
                [&]()
                {
                    if (cursor.match({TokenType::Case})) cases.push_back(parse_switch_case());
                    else if (cursor.match({TokenType::Default}))
                    {
                        if (default_case != nullptr)
                            cursor.report_error_no_panic(cursor.previous(), E::MultipleDefaultCasesInSwitch);
                        default_case = parse_switch_default();
                    }
                    else
                    {
                        if (TokenTraits::is_top_level_token(cursor.peek().type))
                        {
                            const Token& start_tok = cursor.peek();
                            parser.consume_unexpected_statement_gracefully();
                            cursor.report_error_no_panic(cursor.make_span(start_tok, cursor.previous()),
                                                         E::TopLevelDeclarationNotAllowedHere);
                            throw ParseSyncException();
                        }
                        else
                        {
                            cursor.report_error(cursor.peek(), E::ExpectedCaseOrDefaultInsideSwitchBody, true);
                        }
                    }
                }, conf
            );
        }
    }

    std::vector<std::unique_ptr<Expression>> ExpressionParser::parse_expression_list(
        const TokenType closing_token, const std::optional<E> trailing_comma_err,
        const std::vector<TokenType>& recovery_boundaries)
    {
        return ctx.parse_list<std::unique_ptr<Expression>>(
            closing_token,
            trailing_comma_err,
            E::MissingCommaOrOperatorBetweenExpressions,
            recovery_boundaries,
            [this]() { return TokenTraits::is_expression_start(cursor.peek().type); },
            [this]() { return parse_expression(); }
        );
    }

    bool ExpressionParser::is_expression_complete(const Expression* expr) const
    {
        if (!expr) return false;
        if (auto* b = dynamic_cast<const BinaryExpression*>(expr))
            return is_expression_complete(b->left.get()) &&
                is_expression_complete(b->right.get());
        if (auto* u = dynamic_cast<const UnaryExpression*>(expr)) return is_expression_complete(u->right.get());
        if (auto* g = dynamic_cast<const GroupingExpression*>(expr)) return is_expression_complete(g->expression.get());
        if (auto* c = dynamic_cast<const ConditionalExpression*>(expr))
            return
                is_expression_complete(c->condition.get()) && is_expression_complete(c->then_branch.get()) &&
                is_expression_complete(c->else_branch.get());
        if (auto* f = dynamic_cast<const FunctionCall*>(expr))
        {
            if (!is_expression_complete(f->target.get())) return false;
            for (const auto& [name, val] : f->arguments) if (!is_expression_complete(val.get())) return false;
            return true;
        }
        if (auto* d = dynamic_cast<const DictLiteral*>(expr))
        {
            for (const auto& item : d->elements) if (!is_expression_complete(item.value.get())) return false;
            return true;
        }
        if (auto* t = dynamic_cast<const TensorLiteral*>(expr))
        {
            for (const auto& elem : t->elements) if (!is_expression_complete(elem.get())) return false;
            return true;
        }
        if (auto* tup = dynamic_cast<const TupleLiteral*>(expr))
        {
            for (const auto& elem : tup->elements) if (!is_expression_complete(elem.get())) return false;
            return true;
        }
        if (auto* br = dynamic_cast<const BracketAccess*>(expr))
            return is_expression_complete(br->target.get()) &&
                is_expression_complete(br->index.get());
        if (auto* dot = dynamic_cast<const DotAccess*>(expr)) return is_expression_complete(dot->target.get());
        if (auto* sw = dynamic_cast<const SwitchExpression*>(expr))
        {
            if (!is_expression_complete(sw->target.get())) return false;
            for (const auto& [ids, result] : sw->cases) if (!is_expression_complete(result.get())) return false;
            if (sw->default_case && !is_expression_complete(sw->default_case.get())) return false;
            return true;
        }
        return true;
    }
}
