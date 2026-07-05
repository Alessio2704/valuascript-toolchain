#include "expression_parser.h"
#include "parser.h"
#include "token/reserved_keyword_lookup.h"
#include "ast_factory.h"
#include "list_parser.h"
#include "error_recovery.h"
#include "declaration_rules.h"
#include <algorithm>

namespace valuascript::compiler
{
    using E = ParserErrorCode;

    ExpressionParser::ExpressionParser(Parser& p) : parser(p), ctx(p.ctx), cursor(p.ctx.cursor)
    {
    }

    bool ExpressionParser::is_inside_expr_grouping() const
    {
        return std::any_of(ctx.active_closers.begin(), ctx.active_closers.end(),
                           [](TokenType t) { return t == TokenType::RightParen || t == TokenType::RightBracket; });
    }

    bool ExpressionParser::can_continue_expression(const Token& op_tok, const ParseRule& rule, Precedence min_prec,
                                                   bool inside_grouping) const
    {
        if (rule.precedence < min_prec || rule.precedence == Precedence::None || rule.infix == nullptr) return false;
        if (op_tok.line > cursor.previous().line && !inside_grouping)
        {
            if (!TokenTraits::is_postfix_operator(op_tok.type)) return false;
        }
        return true;
    }

    bool ExpressionParser::is_dangling_binary_operator(const Token& op) const
    {
        const Token& next = cursor.peek();
        return next.type == TokenType::EndOfFile || (next.line > op.line && (
            TokenTraits::is_statement_start(next, cursor.peek(1).type) || TokenTraits::is_expression_statement_start(
                next, cursor.peek(1).type)));
    }

    ExprPtr ExpressionParser::handle_dangling_binary_operator(ExprPtr left, const Token& op)
    {
        cursor.report_error_no_panic(op, E::InvalidExpression);
        return AstFactory::make_node_with_span<BinaryExpression>(
            cursor.combine_spans(left->span, cursor.make_span(op, op)), std::move(left), op.type, nullptr);
    }

    void ExpressionParser::check_comparison_chaining(const ParseRule& previous_rule) const
    {
        if (previous_rule.precedence == Precedence::Comparison && get_rule(cursor.peek().type).precedence ==
            Precedence::Comparison)
            cursor.report_error_no_panic(cursor.peek(), E::ChainingNotAllowedForComparisonOperations);
    }

    bool ExpressionParser::is_reassignment_start_lookahead() const
    {
        size_t offset = 0;

        TokenType start_type = cursor.peek(offset).type;
        if (start_type != TokenType::Identifier && !TokenTraits::acts_like_identifier(
            cursor.peek(offset), cursor.peek(offset + 1).type))
        {
            return false;
        }

        offset++;

        while (true)
        {
            TokenType type = cursor.peek(offset).type;
            if (type == TokenType::LeftBracket)
            {
                int depth = 1;
                offset++;
                while (depth > 0 && cursor.peek(offset).type != TokenType::EndOfFile)
                {
                    if (cursor.peek(offset).type == TokenType::LeftBracket) depth++;
                    else if (cursor.peek(offset).type == TokenType::RightBracket) depth--;
                    offset++;
                }
            }
            else if (type == TokenType::LeftParen)
            {
                int depth = 1;
                offset++;
                while (depth > 0 && cursor.peek(offset).type != TokenType::EndOfFile)
                {
                    if (cursor.peek(offset).type == TokenType::LeftParen) depth++;
                    else if (cursor.peek(offset).type == TokenType::RightParen) depth--;
                    offset++;
                }
            }
            else if (type == TokenType::Dot)
            {
                offset++;
                if (cursor.peek(offset).type == TokenType::Identifier)
                {
                    offset++;
                }
                else
                {
                    return false;
                }
            }
            else if (type == TokenType::Assign)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    ExprPtr ExpressionParser::parse_expression(const Precedence min_precedence, bool allow_missing_operator_binding)
    {
        struct ScopeRestore
        {
            bool& ref;
            bool prev;
            ScopeRestore(bool& r, bool v) : ref(r), prev(r) { r = v; }
            ~ScopeRestore() { ref = prev; }
        } scope(allow_missing_operator_binding_, allow_missing_operator_binding);

        const Token& start_tok = cursor.peek();
        ParseRule rule = get_rule(start_tok.type);

        if (rule.prefix == nullptr) return handle_invalid_expression_start();

        auto left = (this->*(rule.prefix))();

        while (true)
        {
            const Token& op_tok = cursor.peek();
            ParseRule infix_rule = get_rule(op_tok.type);
            bool inside_expr_grouping = is_inside_expr_grouping();

            if (!can_continue_expression(op_tok, infix_rule, min_precedence, inside_expr_grouping))
            {
                if (TokenTraits::is_expression_start(op_tok.type))
                {
                    if (infix_rule.infix != nullptr)
                    {
                        break;
                    }

                    bool physical_newline = op_tok.line > cursor.previous().line;
                    bool crossed_newline = physical_newline && !inside_expr_grouping;

                    if (is_reassignment_start_lookahead())
                    {
                        break;
                    }

                    if (!crossed_newline)
                    {
                        if (TokenTraits::is_statement_start(op_tok, cursor.peek(1).type))
                        {
                            break;
                        }

                        if (op_tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(
                            op_tok, cursor.peek(1).type))
                        {
                            if (cursor.peek(1).type == TokenType::Colon || cursor.peek(1).type == TokenType::Assign)
                            {
                                break;
                            }
                        }

                        if (min_precedence > Precedence::Postfix)
                        {
                            break;
                        }

                        if (!allow_missing_operator_binding)
                        {
                            break;
                        }

                        if (op_tok.type == TokenType::LeftBrace)
                        {
                            TokenType next = cursor.peek(1).type;
                            if (next == TokenType::Case || next == TokenType::Default)
                            {
                                break;
                            }
                        }

                        if (physical_newline && inside_expr_grouping)
                        {
                            if (TokenTraits::is_expression_statement_start(op_tok, cursor.peek(1).type))
                            {
                                break;
                            }
                        }

                        cursor.report_error_no_panic(op_tok, E::MissingOperator);

                        Token fake_op = op_tok;
                        fake_op.type = TokenType::Error;
                        fake_op.lexeme = "<missing_operator>";

                        auto right = ErrorRecovery::try_parse<ExprPtr>(
                            ctx, [&]() { return parse_expression(Precedence::Postfix); },
                            RecoveryConfig::ForceStopAtBoundary({
                                TokenType::Comma, TokenType::RightParen, TokenType::RightBracket, TokenType::RightBrace
                            })
                        );

                        const SourceSpan right_span = right
                                                          ? right->span
                                                          : cursor.make_span(cursor.previous(), cursor.previous());
                        left = AstFactory::make_node_with_span<BinaryExpression>(
                            cursor.combine_spans(left->span, right_span),
                            std::move(left), TokenType::Error, std::move(right)
                        );
                        continue;
                    }
                }
                break;
            }

            Token op = cursor.advance();
            if (!inside_expr_grouping && infix_rule.infix == &ExpressionParser::parse_infix_binary)
                if (is_dangling_binary_operator(op)) return handle_dangling_binary_operator(std::move(left), op);

            left = (this->*(infix_rule.infix))(std::move(left), op);
            check_comparison_chaining(infix_rule);
        }
        return left;
    }

    ExprPtr ExpressionParser::parse_infix_binary(ExprPtr left, const Token& op)
    {
        ParseRule infix_rule = get_rule(op.type);
        Precedence next_precedence = infix_rule.is_right_associative
                                         ? infix_rule.precedence
                                         : static_cast<Precedence>(static_cast<int>(infix_rule.precedence) + 1);

        auto right = ErrorRecovery::try_parse<ExprPtr>(
            ctx, [&]() { return parse_expression(next_precedence, allow_missing_operator_binding_); },
            RecoveryConfig::ForceStopAtBoundary({
                TokenType::Comma, TokenType::RightParen, TokenType::RightBracket, TokenType::RightBrace
            })
        );
        const SourceSpan right_span = right ? right->span : cursor.make_span(cursor.previous(), cursor.previous());
        return AstFactory::make_node_with_span<BinaryExpression>(cursor.combine_spans(left->span, right_span),
                                                                 std::move(left), op.type, std::move(right));
    }

    ExprPtr ExpressionParser::parse_prefix_unary()
    {
        Token op = cursor.advance();
        bool inside_expr_grouping = std::any_of(
            ctx.active_closers.begin(), ctx.active_closers.end(),
            [](TokenType t)
            {
                return t == TokenType::RightParen || t == TokenType::RightBracket;
            }
        );

        if (!inside_expr_grouping)
        {
            const Token& next = cursor.peek();
            if (next.type == TokenType::EndOfFile || (next.line > op.line && (
                TokenTraits::is_statement_start(next, cursor.peek(1).type) ||
                TokenTraits::is_expression_statement_start(next, cursor.peek(1).type))))
            {
                cursor.report_error_no_panic(op, E::InvalidExpression);
                return AstFactory::make_node_with_span<UnaryExpression>(cursor.make_span(op, op), op.type, nullptr);
            }
        }

        auto right = ErrorRecovery::try_parse<ExprPtr>(
            ctx, [&] { return parse_expression(Precedence::Unary, allow_missing_operator_binding_); },
            RecoveryConfig::ForceStopAtBoundary({
                TokenType::Comma, TokenType::RightParen, TokenType::RightBracket, TokenType::RightBrace
            })
        );
        return AstFactory::make_node<UnaryExpression>(cursor, op, op.type, std::move(right));
    }

    ExprPtr ExpressionParser::handle_invalid_expression_start()
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
                parser.parse_modifiers();
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
            return AstFactory::make_node_with_span<IdentifierAccess>(cursor.make_span(tok, tok), tok.lexeme);
        }
        cursor.report_error(tok, E::InvalidExpression, force_location);
    }

    ExprPtr ExpressionParser::parse_function_call(ExprPtr target, const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;
        CloserTracker tracker(ctx, TokenType::RightParen);
        std::vector<std::pair<std::string, ExprPtr>> arguments;

        try
        {
            if (!cursor.check(TokenType::RightParen) && !cursor.is_at_end())
            {
                const Token& p0 = cursor.peek();
                const TokenType p1 = cursor.peek(1).type;

                if (p0.type == TokenType::Identifier || TokenTraits::acts_like_identifier(p0, p1))
                {
                    if (p1 != TokenType::Colon && TokenTraits::is_binary_operator(p1))
                        if (!TokenTraits::is_newline_statement_boundary(cursor.previous(), p0, p1))
                            cursor.report_error(
                                cursor.previous(), E::MissingOperator);
                }
                else
                {
                    if (TokenTraits::is_expression_start(p0.type))
                    {
                        if (!TokenTraits::is_newline_statement_boundary(cursor.previous(), p0, p1))
                            cursor.report_error(
                                cursor.previous(), E::MissingOperator);
                    }
                    else if (!cursor.check(TokenType::Colon) && !cursor.check(TokenType::Comma))
                        cursor.report_error(
                            cursor.peek(), E::ExpectedArgumentNameOrClosingParen);
                }
            }

            ParameterRuleSpec arg_spec{
                .allow_value = true, .require_value = true, .value_separator = TokenType::Colon,
                .missing_name_err = E::MissingArgumentNameInFunctionCall,
                .missing_value_separator_err = E::MissingColonAfterArgument, .missing_value_err = E::InvalidExpression
            };

            auto args_gen = ListParser<GenericParameter>(ctx)
                            .stop_at(TokenType::RightParen)
                            .on_trailing_comma(E::TrailingCommaInFunctionCall)
                            .on_missing_comma(E::MissingCommaSeparatorForArgumentsInFunctionCall)
                            .is_element_start([this]()
                            {
                                const Token& tok = cursor.peek();
                                return (tok.type == TokenType::Identifier ||
                                        TokenTraits::acts_like_identifier(tok, cursor.peek(1).type)) && cursor.peek(1).
                                    type
                                    == TokenType::Colon;
                            })
                            .parse_elements([&]() { return parser.parse_generic_parameter(arg_spec); });

            for (auto& g : args_gen) arguments.emplace_back(g.name.lexeme, std::move(g.value));

            const Token& end_token = cursor.consume(TokenType::RightParen, E::ExpectedRightParenAfterArguments);
            return AstFactory::make_node_with_span<FunctionCall>(
                cursor.combine_spans(target_span, cursor.make_span(end_token, end_token)), std::move(target),
                std::move(arguments));
        }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightParen);
            return AstFactory::make_node_with_span<FunctionCall>(
                cursor.combine_spans(target_span, cursor.make_span(cursor.previous(), cursor.previous())),
                std::move(target), std::move(arguments));
        }
    }

    ExprPtr ExpressionParser::parse_tensor_access(ExprPtr target, const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;
        CloserTracker tracker(ctx, TokenType::RightBracket);
        ExprPtr index_expr = nullptr;

        try
        {
            auto parse_bound = [&](ExprPtr& out_expr)
            {
                if (cursor.check(TokenType::Colon) || cursor.check(TokenType::RightBracket)) return;
                out_expr = parse_expression();
                if (!cursor.check(TokenType::Colon) && !cursor.check(TokenType::RightBracket))
                {
                    if (!TokenTraits::is_newline_statement_boundary(cursor.previous(), cursor.peek(),
                                                                    cursor.peek(1).type))
                    {
                        if (TokenTraits::is_expression_start(cursor.peek().type))
                            cursor.report_error(cursor.peek(), E::MissingOperator);
                        else if (cursor.check(TokenType::Comma))
                            cursor.report_error(cursor.peek(), E::UnexpectedCommaInBracketAccess);
                    }
                }
            };

            parse_bound(index_expr);

            if (cursor.match({TokenType::Colon}))
            {
                ExprPtr end_expr = nullptr;
                parse_bound(end_expr);
                const SourceSpan colon_span = index_expr ? index_expr->span : target_span;
                const SourceSpan slice_end_span = end_expr
                                                      ? end_expr->span
                                                      : cursor.make_span(cursor.previous(), cursor.previous());
                index_expr = AstFactory::make_node_with_span<BinaryExpression>(
                    cursor.combine_spans(colon_span, slice_end_span), std::move(index_expr), TokenType::Colon,
                    std::move(end_expr));
            }
            else if (!index_expr) cursor.report_error(cursor.previous(), E::EmptyBracketAccess);

            const Token& end_token = cursor.consume(TokenType::RightBracket, E::UnmatchedBracketAfterTensorIndex);
            return AstFactory::make_node_with_span<BracketAccess>(
                cursor.combine_spans(target_span, cursor.make_span(end_token, end_token)), std::move(target),
                std::move(index_expr));
        }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBracket);
            return AstFactory::make_node_with_span<BracketAccess>(
                cursor.combine_spans(target_span, cursor.make_span(cursor.previous(), cursor.previous())),
                std::move(target), std::move(index_expr));
        }
    }

    ExprPtr ExpressionParser::parse_dot_access(ExprPtr target, const Token& /*op*/)
    {
        Token property_token = ErrorRecovery::try_consume_identifier(
            ctx, E::ExpectedPropertyName,
            RecoveryConfig::ForceStopAtBoundary({
                TokenType::Assign,
                TokenType::Comma,
                TokenType::Dot,
                TokenType::LeftParen,
                TokenType::LeftBracket,
                TokenType::LeftBrace
            }), true, true);

        return AstFactory::make_node_with_span<DotAccess>(
            cursor.combine_spans(target->span, cursor.make_span(property_token, property_token)), std::move(target),
            property_token.lexeme);
    }

    ExprPtr ExpressionParser::parse_tuple_or_grouping()
    {
        const Token& start = cursor.advance();
        CloserTracker tracker(ctx, TokenType::RightParen);

        if (cursor.match({TokenType::RightParen}))
            return AstFactory::make_node<TupleLiteral>(
                cursor, start, std::vector<ExprPtr>{});

        bool failed = false;
        RecoveryConfig conf;
        conf.stop_tokens = {TokenType::Comma, TokenType::RightParen};
        conf.options = RecoveryOptions::SkipNestedGroupings;

        auto first_expr = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_expression(); }, conf, &failed);

        if (cursor.match({TokenType::Comma})) return complete_tuple(std::move(first_expr), start);
        return complete_grouping(std::move(first_expr), failed, start);
    }

    ExprPtr ExpressionParser::complete_tuple(ExprPtr first_expr, const Token& start)
    {
        if (cursor.check(TokenType::RightParen))
            cursor.report_error_no_panic(
                cursor.previous(), E::SingleElementTuplesNotAllowed);

        std::vector<ExprPtr> elements;
        if (first_expr) elements.push_back(std::move(first_expr));

        auto remaining = parse_expression_list(TokenType::RightParen, E::TrailingCommaInTuple);
        elements.insert(elements.end(), std::make_move_iterator(remaining.begin()),
                        std::make_move_iterator(remaining.end()));

        try
        {
            const Token& end = cursor.consume(TokenType::RightParen, E::ExpectedRightParenAfterTupleElements);
            return AstFactory::make_node_with_span<TupleLiteral>(cursor.make_span(start, end), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightParen);
            return AstFactory::make_node<TupleLiteral>(cursor, start, std::move(elements));
        }
    }

    ExprPtr ExpressionParser::complete_grouping(ExprPtr first_expr, bool failed, const Token& start)
    {
        try
        {
            if (!failed && !cursor.check(TokenType::RightParen) && TokenTraits::is_expression_start(cursor.peek().type))
            {
                if (!TokenTraits::is_newline_statement_boundary(cursor.previous(), cursor.peek(), cursor.peek(1).type))
                    cursor.report_error(cursor.peek(), E::MissingOperator);
            }
            const Token& end = cursor.consume(TokenType::RightParen, E::ExpectedRightParenAfterExpression);
            return AstFactory::make_node_with_span<GroupingExpression>(cursor.make_span(start, end),
                                                                       std::move(first_expr));
        }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightParen);
            return AstFactory::make_node<GroupingExpression>(cursor, start, std::move(first_expr));
        }
    }

    ExprPtr ExpressionParser::parse_tensor_literal()
    {
        const Token& start = cursor.advance();
        CloserTracker tracker(ctx, TokenType::RightBracket);
        auto elements = parse_expression_list(TokenType::RightBracket);

        try
        {
            const Token& end = cursor.consume(TokenType::RightBracket, E::UnmatchedBracketAfterTensorElements);
            return AstFactory::make_node_with_span<TensorLiteral>(cursor.make_span(start, end), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBracket);
            return AstFactory::make_node<TensorLiteral>(cursor, start, std::move(elements));
        }
    }

    ExprPtr ExpressionParser::parse_dict_literal()
    {
        const Token& start = cursor.advance();
        CloserTracker tracker(ctx, TokenType::RightBrace);

        ParameterRuleSpec dict_spec{
            .allow_modifiers = true, .allow_value = true, .require_value = true, .value_separator = TokenType::Colon,
            .missing_name_err = E::ExpectedDictionaryKey,
            .missing_value_separator_err = E::ExpectedColonAfterDictionaryKey, .missing_value_err = E::InvalidExpression
        };

        auto items_gen = ListParser<GenericParameter>(ctx)
                         .stop_at(TokenType::RightBrace)
                         .on_missing_comma(E::ExpectedCommaSeparatorInDictionaryLiteral)
                         .is_element_start([this]()
                         {
                             const Token& tok = cursor.peek();
                             if (tok.type == TokenType::At) return !ctx.is_at_any_declaration();
                             if (tok.type == TokenType::Identifier) return true;
                             return is_reserved_keyword(tok) && (cursor.peek(1).type == TokenType::Colon);
                         })
                         .parse_elements([&]() { return parser.parse_generic_parameter(dict_spec); });

        std::vector<DictItem> elements;
        elements.reserve(items_gen.size());
        for (auto& g : items_gen) elements.push_back({std::move(g.modifiers), g.name.lexeme, std::move(g.value)});

        try
        {
            const Token& end = cursor.consume(TokenType::RightBrace, E::UnmatchedBraceInDictionaryLiteral);
            return AstFactory::make_node_with_span<DictLiteral>(cursor.make_span(start, end), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBrace);
            return AstFactory::make_node<DictLiteral>(cursor, start, std::move(elements));
        }
    }

    ExprPtr ExpressionParser::parse_conditional_expression()
    {
        const Token& start = cursor.advance();
        SyncSetTracker tracker(ctx, {TokenType::Then, TokenType::Else});
        RecoveryConfig conf;
        conf.stop_tokens = {
            TokenType::Then, TokenType::Else,
            TokenType::RightParen, TokenType::RightBracket,
            TokenType::RightBrace, TokenType::Comma
        };
        conf.options = DefaultRecoveryOptions | RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp |
            RecoveryOptions::StopEarlyIfUnbalancedBlocks;

        auto condition = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_expression(); }, conf);

        bool has_then = cursor.match({TokenType::Then});
        if (!has_then) cursor.report_error_no_panic(cursor.peek(), E::MissingThenToken);

        ExprPtr then_branch = nullptr;
        bool skip_then = false;
        if (!has_then)
        {
            if (cursor.is_at_end() || cursor.check(TokenType::Else) || ctx.is_active_closer(cursor.peek().type) ||
                !TokenTraits::is_expression_start(cursor.peek().type))
            {
                skip_then = true;
            }
            else if (cursor.peek().line > cursor.previous().line && TokenTraits::is_newline_statement_boundary(
                cursor.previous(), cursor.peek(), cursor.peek(1).type))
            {
                skip_then = true;
            }
        }

        if (!skip_then)
        {
            then_branch = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_expression(); }, conf);
        }

        bool has_else = cursor.match({TokenType::Else});
        if (!has_else) cursor.report_error_no_panic(cursor.peek(), E::MissingElseToken);

        ExprPtr else_branch = nullptr;
        bool skip_else = false;
        if (!has_else)
        {
            if (cursor.is_at_end() || ctx.is_active_closer(cursor.peek().type) ||
                !TokenTraits::is_expression_start(cursor.peek().type))
            {
                skip_else = true;
            }
            else if (cursor.peek().line > cursor.previous().line && TokenTraits::is_newline_statement_boundary(
                cursor.previous(), cursor.peek(), cursor.peek(1).type))
            {
                skip_else = true;
            }
        }

        if (!skip_else)
        {
            else_branch = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_expression(); }, conf);
        }

        return AstFactory::make_node<ConditionalExpression>(cursor, start, std::move(condition), std::move(then_branch),
                                                            std::move(else_branch));
    }

    ExprPtr ExpressionParser::parse_switch_expression()
    {
        const Token& start = cursor.advance();
        auto target = parse_switch_target();

        cursor.consume(TokenType::LeftBrace, E::ExpectedLeftBraceBeforeSwitchBody);
        CloserTracker tracker(ctx, TokenType::RightBrace);

        std::vector<SwitchCase> cases;
        std::vector<Modifier> default_mods;
        ExprPtr default_case = nullptr;
        parse_switch_body(cases, default_mods, default_case);

        try
        {
            const Token& end = cursor.consume(TokenType::RightBrace, E::ExpectedRightBraceAfterSwitchBody);
            return AstFactory::make_node_with_span<SwitchExpression>(cursor.make_span(start, end), std::move(target),
                                                                     std::move(cases), std::move(default_mods),
                                                                     std::move(default_case));
        }
        catch (const ParseSyncException&)
        {
            ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBrace);
            return AstFactory::make_node<SwitchExpression>(cursor, start, std::move(target), std::move(cases),
                                                           std::move(default_mods), std::move(default_case));
        }
    }

    SwitchCase ExpressionParser::parse_switch_case(std::vector<Modifier> modifiers)
    {
        std::vector<std::string> identifiers;

        while (true)
        {
            const Token& tok = cursor.peek();
            if (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, cursor.peek(1).type))
            {
                RecoveryConfig conf;
                conf.stop_tokens = {TokenType::Comma, TokenType::Arrow, TokenType::RightBrace};
                conf.options = RecoveryOptions::SkipNestedGroupings |
                    RecoveryOptions::StopAtBoundaryRespectingDanglingOp;
                Token id = ErrorRecovery::try_consume_identifier(ctx, E::ExpectedEnumCaseNameAfterCase, conf);
                identifiers.push_back(id.lexeme);
            }
            else
            {
                cursor.report_error_no_panic(tok, E::ExpectedEnumCaseNameAfterCase, true);
                identifiers.emplace_back("<error>");

                if (!cursor.check(TokenType::Comma) && !cursor.check(TokenType::Arrow) && !cursor.check(
                    TokenType::RightBrace))
                {
                    RecoveryConfig conf;
                    conf.stop_tokens = {TokenType::Comma, TokenType::Arrow, TokenType::RightBrace};
                    conf.options = RecoveryOptions::SkipNestedGroupings |
                        RecoveryOptions::StopAtBoundaryRespectingDanglingOp;
                    ErrorRecovery::synchronize_with(ctx, conf);
                }
            }
            if (cursor.match({TokenType::Comma})) continue;
            if (cursor.check(TokenType::Identifier) || (is_reserved_keyword(cursor.peek()) &&
                TokenTraits::acts_like_identifier(cursor.peek(), cursor.peek(1).type)))
            {
                cursor.report_error_no_panic(cursor.peek(), E::ExpectedCommaBetweenCaseIdentifiers);
                continue;
            }
            break;
        }

        RecoveryConfig conf;
        conf.stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace};
        conf.options = RecoveryOptions::SkipNestedGroupings | RecoveryOptions::StopEarlyIfUnbalancedBlocks;
        auto result = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_switch_result(); }, conf);
        return {std::move(modifiers), std::move(identifiers), std::move(result)};
    }

    ExprPtr ExpressionParser::parse_switch_default()
    {
        RecoveryConfig conf;
        conf.stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace};
        conf.options = RecoveryOptions::SkipNestedGroupings | RecoveryOptions::StopEarlyIfUnbalancedBlocks;
        return ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_switch_result(); }, conf);
    }

    ExprPtr ExpressionParser::parse_switch_result()
    {
        cursor.consume(TokenType::Arrow, E::ExpectedRightArrowAfterSwitchCaseIdentifier);
        auto expr = parse_expression();

        bool should_break_out = ctx.is_missing_closing_brace() && (ctx.is_at_top_level_declaration() || cursor.peek().
            type == TokenType::Return ||
            TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) || (cursor.peek().line > cursor.
                previous().line &&
                TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type)));

        if (!should_break_out && !cursor.check(TokenType::Case) && !cursor.check(TokenType::Default) && !cursor.
            check(TokenType::RightBrace) && !cursor.is_at_end())
        {
            if (cursor.peek().line == cursor.previous().line)
            {
                if (TokenTraits::is_expression_start(cursor.peek().type))
                    cursor.report_error_no_panic(
                        cursor.peek(), E::MissingOperator, true);
                else cursor.report_error_no_panic(cursor.peek(), E::CaseOrDefaultMissingInSwitchAfterResult, true);
            }
        }
        return expr;
    }

    ExprPtr ExpressionParser::parse_switch_target()
    {
        bool failed = false;
        RecoveryConfig conf;
        conf.stop_tokens = {TokenType::RightParen, TokenType::LeftBrace};
        conf.options = RecoveryOptions::SkipNestedGroupings;

        auto target = ErrorRecovery::try_parse<ExprPtr>(
            ctx, [&]()
            {
                cursor.consume(TokenType::LeftParen, E::ExpectedLeftParenAfterSwitch);
                CloserTracker tracker(ctx, TokenType::RightParen);
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

    void ExpressionParser::parse_switch_body(std::vector<SwitchCase>& cases, std::vector<Modifier>& default_mods,
                                             ExprPtr& default_case)
    {
        SyncSetTracker tracker(ctx, {TokenType::Case, TokenType::Default});
        while (!cursor.check(TokenType::RightBrace) && !cursor.is_at_end())
        {
            bool should_break_out = ctx.is_missing_closing_brace() && (ctx.is_at_top_level_declaration() || cursor.
                peek().type == TokenType::Return ||
                TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) || (cursor.peek().line > cursor.
                    previous().line &&
                    TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type)));
            if (should_break_out) break;

            RecoveryConfig conf;
            conf.stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace};
            conf.options = RecoveryOptions::SkipNestedGroupings | RecoveryOptions::StopEarlyIfUnbalancedBlocks;
            ErrorRecovery::attempt_parse_void(ctx, [&]()
            {
                auto modifiers = parser.parse_modifiers();

                if (cursor.match({TokenType::Case})) cases.push_back(parse_switch_case(std::move(modifiers)));
                else if (cursor.match({TokenType::Default}))
                {
                    if (default_case != nullptr)
                        cursor.report_error_no_panic(
                            cursor.previous(), E::MultipleDefaultCasesInSwitch);
                    default_mods = std::move(modifiers);
                    default_case = parse_switch_default();
                }
                else
                {
                    ctx.reject_modifiers(modifiers);
                    if (TokenTraits::is_top_level_token(cursor.peek().type))
                    {
                        const Token& start_tok = cursor.peek();
                        parser.consume_unexpected_statement_gracefully();
                        cursor.report_error_no_panic(cursor.make_span(start_tok, cursor.previous()),
                                                     E::TopLevelDeclarationNotAllowedHere);
                        throw ParseSyncException();
                    }
                    else cursor.report_error(cursor.peek(), E::ExpectedCaseOrDefaultInsideSwitchBody, true);
                }
            }, conf);
        }
    }

    std::vector<ExprPtr> ExpressionParser::parse_expression_list(const TokenType closing_token,
                                                                 const std::optional<E> trailing_comma_err,
                                                                 const std::vector<TokenType>& recovery_boundaries)
    {
        return ListParser<ExprPtr>(ctx)
               .stop_at(closing_token)
               .on_trailing_comma(trailing_comma_err)
               .on_missing_comma(E::MissingOperator)
               .with_recovery_boundaries(recovery_boundaries)
               .is_element_start([this]() { return TokenTraits::is_expression_start(cursor.peek().type); })
               .parse_elements([this]() { return parse_expression(Precedence::Or, false); });
    }
}
