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
        if (ctx.active_closers.empty()) return false;
        for (auto it = ctx.active_closers.rbegin(); it != ctx.active_closers.rend(); ++it)
        {
            TokenType t = *it;
            if (t == TokenType::RightBrace) return false;
            if (t == TokenType::RightParen || t == TokenType::RightBracket ||
                t == TokenType::Then || t == TokenType::Else)
                return true;
        }
        return false;
    }

    bool ExpressionParser::can_continue_expression(const Token& op_tok, const ParseRule& rule, Precedence min_prec,
                                                   bool inside_grouping) const
    {
        if (rule.precedence < min_prec || rule.precedence == Precedence::None || rule.infix == nullptr) return false;
        if (op_tok.line > cursor.previous().line && !inside_grouping)
        {
            bool in_nested_container = ctx.active_closers.size() > ctx.conditional_else_closers_size;
            if (ctx.conditional_else_depth > 0 && rule.infix != nullptr && !in_nested_container)
            {
                // In else branch (not inside a nested brace/dict/tuple container), allow multiline binary infix operators
            }
            else
            {
                if (is_reassignment_start_lookahead()) return false;
                if (!TokenTraits::is_postfix_operator(op_tok.type)) return false;
            }
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
        bool is_valid_lvalue_chain = true;

        TokenType start_type = cursor.peek(offset).type;
        if (start_type == TokenType::LeftBracket || start_type == TokenType::LeftParen)
        {
            // Do not advance offset; the while loop below will handle parsing the balanced brackets/parens.
        }
        else if (start_type == TokenType::Identifier || TokenTraits::acts_like_identifier(
            cursor.peek(offset), cursor.peek(offset + 1).type))
        {
            offset++;
        }
        else
        {
            return false;
        }

        while (true)
        {
            TokenType type = cursor.peek(offset).type;
            if (type == TokenType::LeftBracket)
            {
                is_valid_lvalue_chain = true;
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
                if (offset > 0)
                {
                    is_valid_lvalue_chain = false;
                }
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
                is_valid_lvalue_chain = true;
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
                return is_valid_lvalue_chain;
            }
            else
            {
                return false;
            }
        }
    }

    ExprPtr ExpressionParser::parse_expression(const Precedence min_precedence, bool allow_missing_operator_binding)
    {
        struct ExprDepthGuard
        {
            ParserContext& ctx;
            size_t prev_baseline;
            bool is_root;

            ExprDepthGuard(ParserContext& c) : ctx(c), prev_baseline(c.expr_closers_baseline), is_root(c.expr_depth == 0)
            {
                if (is_root)
                    ctx.expr_closers_baseline = ctx.active_closers.size();
                ctx.expr_depth++;
            }

            ~ExprDepthGuard()
            {
                ctx.expr_depth--;
                if (is_root)
                    ctx.expr_closers_baseline = prev_baseline;
            }
        } depth_guard(ctx);

        struct ScopeRestore
        {
            bool& ref;
            bool prev;
            ScopeRestore(bool& r, bool v) : ref(r), prev(r) { r = v; }
            ~ScopeRestore() { ref = prev; }
        } scope(allow_missing_operator_binding_, allow_missing_operator_binding);

        const Token& start_tok = cursor.peek();
        if (!ctx.is_consuming_unexpected && !ctx.is_parsing_expression_statement && ctx.looks_like_reassignment())
        {
            return handle_invalid_expression_start();
        }
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

                    if (crossed_newline)
                    {
                        break;
                    }

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

                        if (physical_newline)
                        {
                            if (TokenTraits::is_expression_statement_start(op_tok, cursor.peek(1).type) ||
                                TokenTraits::is_statement_start(op_tok, cursor.peek(1).type) ||
                                op_tok.type == TokenType::Return ||
                                ctx.looks_like_reassignment() ||
                                ctx.is_at_any_declaration())
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
        bool is_container_or_multiline = (tok.line == next.line || tok.line > prev.line ||
                                          (ctx.is_in_expression_container() && (prev.type == TokenType::LeftParen || prev.type == TokenType::LeftBracket || prev.type == TokenType::LeftBrace || prev.type == TokenType::Colon || prev.type == TokenType::Arrow || prev.type == TokenType::Then)));

        bool is_import_stmt = tok.type == TokenType::Import &&
                              ((next.type == TokenType::String || next.type == TokenType::DocString) ||
                               (tok.line == next.line && next.type != TokenType::Comma && !ctx.is_active_closer(next.type)));

        bool is_return_stmt = tok.type == TokenType::Return && next.type != TokenType::Comma && !ctx.is_active_closer(next.type) && is_container_or_multiline;

        bool has_declaration_structure =
            (tok.type == TokenType::Struct && (next.type == TokenType::Identifier || next.type == TokenType::LeftBrace || next.type == TokenType::Less)) ||
            (tok.type == TokenType::Func && (next.type == TokenType::Identifier || next.type == TokenType::LeftParen || next.type == TokenType::Less)) ||
            (tok.type == TokenType::Enum && (next.type == TokenType::Identifier || next.type == TokenType::LeftBrace || next.type == TokenType::Colon)) ||
            (tok.type == TokenType::Extension && (next.type == TokenType::Identifier || next.type == TokenType::LeftBrace || next.type == TokenType::Less)) ||
            (tok.type == TokenType::Typealias && (next.type == TokenType::Identifier || next.type == TokenType::Assign || next.type == TokenType::Less)) ||
            (tok.type == TokenType::Let && (next.type == TokenType::Identifier || next.type == TokenType::Colon));

        bool is_declaration_construct = has_declaration_structure && is_container_or_multiline;

        bool is_stmt_start = (tok.line > prev.line && (TokenTraits::is_statement_start(tok, next.type) || tok.type == TokenType::Return)) ||
                             ctx.looks_like_reassignment() ||
                             tok.type == TokenType::At ||
                             tok.type == TokenType::Hash ||
                             is_import_stmt ||
                             is_return_stmt ||
                             is_declaration_construct;
        bool force_location = (tok.type != TokenType::EndOfFile && !is_stmt_start);

        if (is_stmt_start)
        {
            const Token& start_tok = cursor.peek();
            if (tok.type == TokenType::At && !ctx.is_at_any_declaration())
            {
                parser.parse_modifiers();
                SourceSpan span = cursor.make_span(start_tok, cursor.previous());
                cursor.report_error_no_panic(span, E::ModifiersAttachedToInvalidDeclaration);
            }
            else
            {
                if (tok.line > prev.line)
                {
                    if (TokenTraits::is_dangling_operator(prev.type) || TokenTraits::is_grouping_opener(prev.type))
                        cursor.report_error(prev, E::InvalidExpression);
                    else cursor.report_error(tok, E::InvalidExpression, force_location);
                }
                bool is_reassign = ctx.looks_like_reassignment();
                parser.consume_unexpected_statement_gracefully();
                SourceSpan span = cursor.make_span(start_tok, cursor.previous());
                cursor.report_error_no_panic(span, E::InvalidConstructPlacement,
                                             is_reassign ? "reassignment" : "declaration",
                                             ctx.is_parsing_list_element ? "in list" : "in expression");
            }

            if (!cursor.is_at_end() &&
                cursor.peek().line == cursor.previous().line &&
                TokenTraits::is_expression_start(cursor.peek().type))
            {
                return parse_expression();
            }
            throw ParseSyncException();
        }

        bool is_delimiter_or_closer = tok.type == TokenType::RightBrace ||
                                      tok.type == TokenType::RightParen ||
                                      tok.type == TokenType::RightBracket ||
                                      ctx.is_active_closer(tok.type) ||
                                      (tok.line > prev.line && (tok.type == TokenType::Return || tok.type == TokenType::Case || tok.type == TokenType::Default || tok.type == TokenType::Then || tok.type == TokenType::Else));

        if (is_delimiter_or_closer)
        {
            if (tok.line > prev.line)
            {
                if (TokenTraits::is_dangling_operator(prev.type) || TokenTraits::is_grouping_opener(prev.type))
                    cursor.report_error(prev, E::InvalidExpression);
            }
            cursor.report_error(tok, E::InvalidExpression, force_location);
        }

        if (is_reserved_keyword(tok))
        {
            cursor.report_error_no_panic(tok, E::ReservedKeywordAsIdentifier, true);
            cursor.advance();
            return AstFactory::make_node_with_span<IdentifierAccess>(
                cursor.make_span(tok, tok), NodeName{tok.lexeme, cursor.make_span(tok)});
        }
        cursor.report_error(tok, E::InvalidExpression, force_location);
    }

    ExprPtr ExpressionParser::parse_function_call(ExprPtr target, const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;
        CloserTracker tracker(ctx, TokenType::RightParen, ContainerKind::CallArguments);
        std::vector<CallArgument> arguments;

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

            KeyValueContainerGuard kv_guard(ctx);
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

            for (auto& g : args_gen)
            {
                arguments.push_back(CallArgument(
                    NodeName{g.name.lexeme, cursor.make_span(g.name)},
                    std::move(g.value),
                    g.span
                ));
            }

            if (ErrorRecovery::should_yield_closer_to_parent(ctx, TokenType::RightParen) ||
                (!cursor.check(TokenType::RightParen) && ctx.is_active_closer(cursor.peek().type)))
            {
                cursor.report_error_no_panic(cursor.peek(), E::ExpectedRightParenAfterArguments);
                return AstFactory::make_node_with_span<FunctionCall>(
                    cursor.combine_spans(target_span, cursor.make_span(cursor.previous(), cursor.previous())),
                    std::move(target), std::move(arguments));
            }

            const Token& end_token = cursor.consume(TokenType::RightParen, E::ExpectedRightParenAfterArguments);
            return AstFactory::make_node_with_span<FunctionCall>(
                cursor.combine_spans(target_span, cursor.make_span(end_token, end_token)), std::move(target),
                std::move(arguments));
        }
        catch (const ParseSyncException&)
        {
            if (!ctx.is_active_closer(cursor.peek().type) || cursor.check(TokenType::RightParen))
                ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightParen);
            return AstFactory::make_node_with_span<FunctionCall>(
                cursor.combine_spans(target_span, cursor.make_span(cursor.previous(), cursor.previous())),
                std::move(target), std::move(arguments));
        }
    }

    ExprPtr ExpressionParser::parse_tensor_access(ExprPtr target, const Token& /*op*/)
    {
        const SourceSpan target_span = target->span;
        CloserTracker tracker(ctx, TokenType::RightBracket, ContainerKind::BracketAccess);
        ExprPtr index_expr = nullptr;
        bool had_start_error = false;

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

            try
            {
                parse_bound(index_expr);
            }
            catch (const ParseSyncException&)
            {
                had_start_error = true;
                auto config = RecoveryConfig::StopAtBoundary({TokenType::Colon, TokenType::RightBracket});
                config.custom_stop_predicate = [&](const Token& tok, TokenType next)
                {
                    if (tok.type == TokenType::Comma) return true;
                    if (TokenTraits::is_identifier_start(tok) && (next == TokenType::Colon || next == TokenType::Assign))
                        return true;
                    return false;
                };
                ErrorRecovery::synchronize_with(ctx, config);
                if (!cursor.check(TokenType::Colon) && !cursor.check(TokenType::RightBracket))
                {
                    throw;
                }
            }

            size_t colon_count = 0;
            while (cursor.match(TokenType::Colon))
            {
                colon_count++;
                const Token colon_token = cursor.previous();
                if (colon_count > 1)
                {
                    cursor.report_error_no_panic(colon_token, E::TooManyColonsInBracketSlice);
                }
                const SourceSpan colon_span = index_expr ? index_expr->span : target_span;
                ExprPtr bound_expr = nullptr;
                try
                {
                    parse_bound(bound_expr);
                }
                catch (const ParseSyncException&)
                {
                    const SourceSpan slice_end_span = bound_expr ? bound_expr->span : cursor.make_span(colon_token, colon_token);
                    index_expr = AstFactory::make_node_with_span<BinaryExpression>(
                        cursor.combine_spans(colon_span, slice_end_span), std::move(index_expr), TokenType::Colon,
                        std::move(bound_expr));
                    throw;
                }
                const SourceSpan slice_end_span = bound_expr
                                                      ? bound_expr->span
                                                      : cursor.make_span(cursor.previous(), cursor.previous());
                index_expr = AstFactory::make_node_with_span<BinaryExpression>(
                    cursor.combine_spans(colon_span, slice_end_span), std::move(index_expr), TokenType::Colon,
                    std::move(bound_expr));
            }

            if (colon_count == 0 && !index_expr && !had_start_error)
            {
                cursor.report_error(cursor.previous(), E::EmptyBracketAccess);
            }

            if (ErrorRecovery::should_yield_closer_to_parent(ctx, TokenType::RightBracket) ||
                (!cursor.check(TokenType::RightBracket) && ctx.is_active_closer(cursor.peek().type)) ||
                cursor.check(TokenType::Assign))
            {
                cursor.report_error_no_panic(cursor.peek(), E::UnmatchedBracketAfterTensorIndex);
                return AstFactory::make_node_with_span<BracketAccess>(
                    cursor.combine_spans(target_span, cursor.make_span(cursor.previous(), cursor.previous())),
                    std::move(target), std::move(index_expr));
            }

            const Token& end_token = cursor.consume(TokenType::RightBracket, E::UnmatchedBracketAfterTensorIndex);
            return AstFactory::make_node_with_span<BracketAccess>(
                cursor.combine_spans(target_span, cursor.make_span(end_token, end_token)), std::move(target),
                std::move(index_expr));
        }
        catch (const ParseSyncException&)
        {
            if (!ctx.is_active_closer(cursor.peek().type) || cursor.check(TokenType::RightBracket))
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

        SourceSpan prop_span = cursor.make_span(property_token);
        return AstFactory::make_node_with_span<DotAccess>(
            cursor.combine_spans(target->span, prop_span), std::move(target),
            NodeName{property_token.lexeme, prop_span});
    }

    ExprPtr ExpressionParser::parse_tuple_or_grouping()
    {
        const Token& start = cursor.advance();
        CloserTracker tracker(ctx, TokenType::RightParen, ContainerKind::TupleLiteral);

        if (cursor.match(TokenType::RightParen))
            return AstFactory::make_node<TupleLiteral>(
                cursor, start, std::vector<ExprPtr>{});

        bool failed = false;
        RecoveryConfig conf{
            .stop_tokens = {TokenType::Comma, TokenType::RightParen},
            .options = RecoveryOptions::SkipNestedGroupings
        };

        auto first_expr = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_expression(); }, conf, &failed);

        if (cursor.check(TokenType::Comma))
        {
            const Token& next_tok = cursor.peek(1);
            const TokenType next_type = cursor.peek(2).type;

            bool is_parent_boundary =
                (ctx.key_value_container_depth > 0 && TokenTraits::is_identifier_start(next_tok) && (next_type == TokenType::Colon || next_type == TokenType::Assign || next_type == TokenType::Arrow) && ErrorRecovery::is_unclosed_before_parent_boundary(ctx)) ||
                (next_tok.type != TokenType::RightParen && ctx.is_active_closer(next_tok.type));

            if (!is_parent_boundary)
            {
                cursor.advance();
                return complete_tuple(std::move(first_expr), start);
            }
        }
        return complete_grouping(std::move(first_expr), failed, start);
    }

    ExprPtr ExpressionParser::complete_tuple(ExprPtr first_expr, const Token& start)
    {
        if (cursor.check(TokenType::RightParen))
            cursor.report_error_no_panic(
                cursor.previous(), E::SingleElementTuplesNotAllowed);

        std::vector<ExprPtr> elements;
        elements.push_back(std::move(first_expr));

        auto is_at_parent_boundary = [this](size_t offset = 0)
        {
            const Token& tok = cursor.peek(offset);
            const TokenType next = cursor.peek(offset + 1).type;
            if (tok.type == TokenType::EndOfFile) return true;
            if (tok.type == TokenType::Comma || tok.type == TokenType::RightParen) return false;
            if (tok.type != TokenType::RightParen && ctx.is_active_closer(tok.type)) return true;
            if (tok.type == TokenType::Assign || (tok.type == TokenType::Identifier && next == TokenType::Assign)) return true;
            if (tok.line > cursor.previous().line)
            {
                if (offset == 0 && (ctx.looks_like_reassignment() || ctx.is_at_any_declaration())) return true;
                if (tok.type != TokenType::At && (TokenTraits::is_statement_start(tok, next) || tok.type == TokenType::Return)) return true;
            }
            if (offset > 0 && ctx.active_closers.size() > 1)
            {
                if (tok.type == TokenType::Identifier && next == TokenType::Colon)
                    return true;
            }
            return false;
        };

        auto remaining = ListParser<ExprPtr>(ctx)
               .stop_at(TokenType::RightParen)
               .on_trailing_comma(E::TrailingCommaInTuple)
               .on_missing_comma(E::MissingOperator)
               .is_at_parent_boundary(is_at_parent_boundary)
               .is_element_start([this]() { return TokenTraits::is_expression_start(cursor.peek().type); })
               .parse_elements([this]() { return parse_expression(Precedence::Or, false); });

        elements.insert(elements.end(), std::make_move_iterator(remaining.begin()),
                        std::make_move_iterator(remaining.end()));

        if (ErrorRecovery::should_yield_closer_to_parent(ctx, TokenType::RightParen))
        {
            cursor.report_error_no_panic(cursor.peek(), E::ExpectedRightParenAfterTupleElements);
            return AstFactory::make_node<TupleLiteral>(cursor, start, std::move(elements));
        }

        try
        {
            const Token& end = cursor.consume(TokenType::RightParen, E::ExpectedRightParenAfterTupleElements);
            return AstFactory::make_node_with_span<TupleLiteral>(cursor.make_span(start, end), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            if (!is_at_parent_boundary(0))
                ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightParen);
            return AstFactory::make_node<TupleLiteral>(cursor, start, std::move(elements));
        }
    }

    ExprPtr ExpressionParser::complete_grouping(ExprPtr first_expr, bool failed, const Token& start)
    {
        if (ErrorRecovery::should_yield_closer_to_parent(ctx, TokenType::RightParen))
        {
            cursor.report_error_no_panic(cursor.peek(), E::ExpectedRightParenAfterExpression);
            return AstFactory::make_node<GroupingExpression>(cursor, start, std::move(first_expr));
        }

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
        CloserTracker tracker(ctx, TokenType::RightBracket, ContainerKind::TensorLiteral);
        auto elements = parse_expression_list(TokenType::RightBracket);

        if (ErrorRecovery::should_yield_closer_to_parent(ctx, TokenType::RightBracket) ||
            (!cursor.check(TokenType::RightBracket) && ctx.is_active_closer(cursor.peek().type)))
        {
            cursor.report_error_no_panic(cursor.peek(), E::UnmatchedBracketAfterTensorElements);
            return AstFactory::make_node<TensorLiteral>(cursor, start, std::move(elements));
        }

        if (cursor.check(TokenType::RightBracket))
        {
            const Token& end = cursor.advance();
            return AstFactory::make_node_with_span<TensorLiteral>(cursor.make_span(start, end), std::move(elements));
        }

        if (cursor.check(TokenType::Comma))
        {
            cursor.report_error_no_panic(cursor.peek(), E::UnmatchedBracketAfterTensorElements);
            return AstFactory::make_node<TensorLiteral>(cursor, start, std::move(elements));
        }

        try
        {
            const Token& end = cursor.consume(TokenType::RightBracket, E::UnmatchedBracketAfterTensorElements);
            return AstFactory::make_node_with_span<TensorLiteral>(cursor.make_span(start, end), std::move(elements));
        }
        catch (const ParseSyncException&)
        {
            if (!ctx.is_active_closer(cursor.peek().type) || cursor.check(TokenType::RightBracket))
                ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBracket);
            return AstFactory::make_node<TensorLiteral>(cursor, start, std::move(elements));
        }
    }

    ExprPtr ExpressionParser::parse_dict_literal()
    {
        const Token& start = cursor.advance();
        CloserTracker tracker(ctx, TokenType::RightBrace, ContainerKind::DictionaryLiteral);
        KeyValueContainerGuard kv_guard(ctx);

        ParameterRuleSpec dict_spec{
            .allow_modifiers = true, .allow_value = true, .require_value = true, .value_separator = TokenType::Colon,
            .missing_name_err = E::ExpectedDictionaryKey,
            .missing_value_separator_err = E::ExpectedColonAfterDictionaryKey, .missing_value_err = E::InvalidExpression
        };

        auto is_at_parent_boundary = [this](size_t offset = 0)
        {
            const Token& tok = cursor.peek(offset);
            const TokenType next = cursor.peek(offset + 1).type;
            if (tok.type == TokenType::EndOfFile) return true;
            if (tok.type == TokenType::Comma || tok.type == TokenType::Colon || tok.type == TokenType::RightBrace) return false;
            if (TokenTraits::acts_like_identifier(tok, next)) return false;
            if (tok.type != TokenType::At && (TokenTraits::is_statement_start(tok, next) || tok.type == TokenType::Return)) return true;
            if (tok.type != TokenType::RightBrace && ctx.is_active_closer(tok.type)) return true;
            if (offset > 0 && (ctx.is_active_closer(TokenType::RightParen) || ctx.is_active_closer(TokenType::RightBracket)))
            {
                if (TokenTraits::is_binary_operator(tok.type)) return false;
                if (tok.type == TokenType::At)
                {
                    size_t mod_offset = offset;
                    while (cursor.peek(mod_offset).type == TokenType::At)
                    {
                        mod_offset++;
                        if (cursor.peek(mod_offset).type == TokenType::EndOfFile) break;
                        mod_offset++;
                        if (cursor.peek(mod_offset).type == TokenType::LeftParen)
                        {
                            int depth = 1;
                            mod_offset++;
                            while (depth > 0 && cursor.peek(mod_offset).type != TokenType::EndOfFile)
                            {
                                if (cursor.peek(mod_offset).type == TokenType::LeftParen) depth++;
                                else if (cursor.peek(mod_offset).type == TokenType::RightParen) depth--;
                                mod_offset++;
                            }
                        }
                    }
                    TokenType past = cursor.peek(mod_offset).type;
                    if (past == TokenType::Identifier || past == TokenType::String || past == TokenType::Number)
                        return false;
                }
                bool is_key = (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, next)) && next == TokenType::Colon;
                if (!is_key && (tok.type == TokenType::String || tok.type == TokenType::Number) && next == TokenType::Colon)
                    is_key = true;
                if (!is_key) return true;
            }
            return false;
        };

        auto items_gen = ListParser<GenericParameter>(ctx)
                         .stop_at(TokenType::RightBrace)
                         .on_missing_comma(E::ExpectedCommaSeparatorInDictionaryLiteral)
                         .is_at_parent_boundary(is_at_parent_boundary)
                         .is_element_start([this]()
                         {
                             const Token& tok = cursor.peek();
                             if (tok.type == TokenType::At) return !ctx.is_at_any_declaration();
                             if (tok.type == TokenType::Identifier) return true;
                             return is_reserved_keyword(tok) && (cursor.peek(1).type == TokenType::Colon);
                         })
                         .parse_elements([&]() { return parser.parse_generic_parameter(dict_spec, is_at_parent_boundary); });

        std::vector<DictItem> elements;
        elements.reserve(items_gen.size());
        for (auto& g : items_gen) elements.push_back(DictItem(
            std::move(g.modifiers),
            NodeName{g.name.lexeme, cursor.make_span(g.name)},
            std::move(g.value),
            g.span
        ));

        if (ErrorRecovery::should_yield_closer_to_parent(ctx, TokenType::RightBrace) ||
            (!cursor.check(TokenType::RightBrace) && ctx.is_active_closer(cursor.peek().type)))
        {
            cursor.report_error_no_panic(cursor.previous(), E::UnmatchedBraceInDictionaryLiteral);
            return AstFactory::make_node<DictLiteral>(cursor, start, std::move(elements));
        }

        if (cursor.check(TokenType::RightBrace))
        {
            const Token& end = cursor.advance();
            return AstFactory::make_node_with_span<DictLiteral>(cursor.make_span(start, end), std::move(elements));
        }

        cursor.report_error_no_panic(cursor.previous(), E::UnmatchedBraceInDictionaryLiteral);
        if (!cursor.check(TokenType::Comma) && !is_at_parent_boundary(0))
        {
            if (!ctx.is_active_closer(cursor.peek().type) || cursor.check(TokenType::RightBrace))
                ErrorRecovery::synchronize_and_consume_closer(ctx, TokenType::RightBrace);
        }
        return AstFactory::make_node<DictLiteral>(cursor, start, std::move(elements));
    }

    ExprPtr ExpressionParser::parse_conditional_expression()
    {
        const Token& start = cursor.advance();
        SyncSetTracker tracker(ctx, {TokenType::Then, TokenType::Else});
        RecoveryConfig conf{
            .stop_tokens = {
                TokenType::Then, TokenType::Else,
                TokenType::RightParen, TokenType::RightBracket,
                TokenType::RightBrace, TokenType::Comma
            },
            .options = DefaultRecoveryOptions | RecoveryOptions::ForceStopAtBoundaryIgnoringDanglingOp |
                RecoveryOptions::StopEarlyIfUnbalancedBlocks
        };

        ExprPtr condition = nullptr;
        {
            CloserTracker condition_tracker(ctx, TokenType::Then, ContainerKind::IfCondition);
            condition = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_expression(); }, conf);
        }

        bool has_then = cursor.match(TokenType::Then);
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
            CloserTracker then_tracker(ctx, TokenType::Else, ContainerKind::ThenBranch);
            then_branch = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_expression(); }, conf);
        }

        bool has_else = cursor.match(TokenType::Else);
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
            struct ElseDepthGuard
            {
                ParserContext& ctx;
                size_t prev_depth;
                size_t prev_closers_size;
                explicit ElseDepthGuard(ParserContext& c)
                    : ctx(c), prev_depth(c.conditional_else_depth), prev_closers_size(c.conditional_else_closers_size)
                {
                    ctx.conditional_else_depth++;
                    ctx.conditional_else_closers_size = ctx.active_closers.size();
                }
                ~ElseDepthGuard()
                {
                    ctx.conditional_else_depth = prev_depth;
                    ctx.conditional_else_closers_size = prev_closers_size;
                }
            } guard(ctx);

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
        CloserTracker tracker(ctx, TokenType::RightBrace, ContainerKind::SwitchBody);
        size_t prev_baseline = ctx.expr_closers_baseline;
        ctx.expr_closers_baseline = ctx.active_closers.size();

        std::vector<SwitchCase> cases;
        std::vector<Modifier> default_mods;
        ExprPtr default_case = nullptr;
        parse_switch_body(cases, default_mods, default_case);
        ctx.expr_closers_baseline = prev_baseline;

        if (!cursor.check(TokenType::RightBrace) && ctx.is_active_closer(cursor.peek().type))
        {
            cursor.report_error_no_panic(cursor.peek(), E::ExpectedRightBraceAfterSwitchBody);
            return AstFactory::make_node<SwitchExpression>(cursor, start, std::move(target),
                                                           std::move(cases), std::move(default_mods),
                                                           std::move(default_case));
        }

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
        std::vector<NodeName> identifiers;

        while (true)
        {
            const Token& tok = cursor.peek();
            if (tok.type == TokenType::Identifier || TokenTraits::acts_like_identifier(tok, cursor.peek(1).type))
            {
                RecoveryConfig conf{
                    .stop_tokens = {TokenType::Comma, TokenType::Arrow, TokenType::RightBrace},
                    .options = RecoveryOptions::SkipNestedGroupings |
                        RecoveryOptions::StopAtBoundaryRespectingDanglingOp
                };
                Token id = ErrorRecovery::try_consume_identifier(ctx, E::ExpectedEnumCaseNameAfterCase, conf);
                identifiers.emplace_back(NodeName{id.lexeme, cursor.make_span(id)});
            }
            else
            {
                cursor.report_error_no_panic(tok, E::ExpectedEnumCaseNameAfterCase, true);
                identifiers.emplace_back(NodeName{"<error>", cursor.make_span(tok)});

                if (!cursor.check(TokenType::Comma) && !cursor.check(TokenType::Arrow) && !cursor.check(
                    TokenType::RightBrace))
                {
                    RecoveryConfig conf{
                        .stop_tokens = {TokenType::Comma, TokenType::Arrow, TokenType::RightBrace},
                        .options = RecoveryOptions::SkipNestedGroupings |
                            RecoveryOptions::StopAtBoundaryRespectingDanglingOp
                    };
                    ErrorRecovery::synchronize_with(ctx, conf);
                }
            }
            if (cursor.match(TokenType::Comma)) continue;
            if (cursor.check(TokenType::Identifier) || (is_reserved_keyword(cursor.peek()) &&
                TokenTraits::acts_like_identifier(cursor.peek(), cursor.peek(1).type)))
            {
                cursor.report_error_no_panic(cursor.peek(), E::ExpectedCommaBetweenCaseIdentifiers);
                continue;
            }
            break;
        }

        RecoveryConfig conf{
            .stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace},
            .options = RecoveryOptions::SkipNestedGroupings | RecoveryOptions::StopEarlyIfUnbalancedBlocks
        };
        auto result = ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_switch_result(); }, conf);
        return SwitchCase(
            std::move(modifiers),
            std::move(identifiers),
            std::move(result)
        );
    }

    ExprPtr ExpressionParser::parse_switch_default()
    {
        RecoveryConfig conf{
            .stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace},
            .options = RecoveryOptions::SkipNestedGroupings | RecoveryOptions::StopEarlyIfUnbalancedBlocks
        };
        return ErrorRecovery::try_parse<ExprPtr>(ctx, [&]() { return parse_switch_result(); }, conf);
    }

    ExprPtr ExpressionParser::parse_switch_result()
    {
        cursor.consume(TokenType::Arrow, E::ExpectedRightArrowAfterSwitchCaseIdentifier);
        size_t prev_baseline = ctx.expr_closers_baseline;
        ctx.expr_closers_baseline = ctx.active_closers.size();
        auto expr = parse_expression();
        ctx.expr_closers_baseline = prev_baseline;

        bool should_break_out = (cursor.peek().type != TokenType::RightBrace && ctx.is_active_closer(cursor.peek().type)) ||
            (ctx.is_missing_closing_brace() && (ctx.is_at_any_declaration() || cursor.peek().type == TokenType::Return ||
            (cursor.peek().type != TokenType::At && TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type)) ||
            (cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type))));

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
        RecoveryConfig conf{
            .stop_tokens = {TokenType::RightParen, TokenType::LeftBrace},
            .options = RecoveryOptions::SkipNestedGroupings
        };

        if (cursor.check(TokenType::LeftParen))
        {
            auto target = ErrorRecovery::try_parse<ExprPtr>(
                ctx, [&]()
                {
                    cursor.consume(TokenType::LeftParen, E::ExpectedLeftParenAfterSwitch);
                    CloserTracker tracker(ctx, TokenType::RightParen, ContainerKind::SwitchTarget);
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

        cursor.report_error_no_panic(cursor.peek(), E::ExpectedLeftParenAfterSwitch);
        if (cursor.check(TokenType::LeftBrace) || cursor.check(TokenType::RightParen))
        {
            cursor.report_error_no_panic(cursor.peek(), E::InvalidExpression);
            if (cursor.check(TokenType::RightParen)) cursor.advance();
            return nullptr;
        }

        bool has_closing_paren = false;
        auto target = ErrorRecovery::try_parse<ExprPtr>(
            ctx, [&]()
            {
                CloserTracker tracker(ctx, TokenType::RightParen, ContainerKind::SwitchTarget);
                auto t = parse_expression();
                if (cursor.check(TokenType::RightParen))
                {
                    has_closing_paren = true;
                    cursor.advance();
                }
                return t;
            }, conf, &failed
        );

        if (!has_closing_paren || failed)
        {
            if (cursor.check(TokenType::RightParen)) cursor.advance();
            else if (TokenTraits::is_grouping_closer(cursor.peek().type))
            {
                if (!ctx.is_active_closer(cursor.peek().type)) cursor.advance();
            }
            return nullptr;
        }
        return target;
    }

    void ExpressionParser::parse_switch_body(std::vector<SwitchCase>& cases, std::vector<Modifier>& default_mods,
                                             ExprPtr& default_case)
    {
        cases.reserve(8);
        SyncSetTracker tracker(ctx, {TokenType::Case, TokenType::Default});
        while (!cursor.check(TokenType::RightBrace) && !cursor.is_at_end())
        {
            bool should_break_out = (cursor.peek().type != TokenType::RightBrace && ctx.is_active_closer(cursor.peek().type)) ||
                (ctx.is_missing_closing_brace() && (ctx.is_at_any_declaration() || cursor.peek().type == TokenType::Return ||
                (cursor.peek().type != TokenType::At && TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type)) ||
                (cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type))));
            if (should_break_out) break;

            RecoveryConfig conf{
                .stop_tokens = {TokenType::Case, TokenType::Default, TokenType::RightBrace},
                .options = RecoveryOptions::SkipNestedGroupings | RecoveryOptions::StopEarlyIfUnbalancedBlocks
            };
            ErrorRecovery::attempt_parse_void(ctx, [&]()
            {
                auto modifiers = parser.parse_modifiers();

                if (cursor.match(TokenType::Case))
                {
                    const Token& case_tok = cursor.previous();
                    auto sc = parse_switch_case(std::move(modifiers));
                    if (!sc.modifiers.empty())
                    {
                        sc.span = cursor.combine_spans(sc.modifiers.front().span, cursor.make_span(cursor.previous(), cursor.previous()));
                    }
                    else
                    {
                        sc.span = cursor.make_span(case_tok, cursor.previous());
                    }
                    cases.push_back(std::move(sc));
                }
                else if (cursor.match(TokenType::Default))
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
                                                     E::InvalidConstructPlacement, "declaration", "inside switch body");
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
