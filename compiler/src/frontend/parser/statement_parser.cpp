#include "statement_parser.h"
#include "parser.h"
#include "ast_factory.h"
#include "error_recovery.h"

namespace valuascript::compiler
{
    using E = ParserErrorCode;

    StatementParser::StatementParser(Parser& p) : parser(p), ctx(p.ctx), cursor(p.ctx.cursor)
    {
    }

    void StatementParser::verify_statement_end() const
    {
        if (!cursor.is_at_end() && cursor.peek().line == cursor.previous().line)
        {
            if (TokenTraits::is_expression_start(cursor.peek().type))
            {
                cursor.report_error_no_panic(cursor.peek(), E::MissingOperator);
                ErrorRecovery::synchronize_with(ctx, RecoveryConfig::ForceStopAtBoundary());
            }
        }
    }

    std::unique_ptr<Assignment> StatementParser::parse_assignment(const std::vector<Modifier>& modifiers)
    {
        const SourceSpan start_span = !modifiers.empty() ? modifiers.front().span : cursor.make_span(cursor.peek());
        cursor.consume(TokenType::Let, E::ExpectedLetToken);

        std::vector<AssignmentTarget> targets;
        do
        {
            const Token& target_start = cursor.peek();
            std::vector<Modifier> target_mods = clone_nodes(modifiers);

            auto inner_mods = parser.parse_modifiers();

            target_mods.insert(target_mods.end(),
                               std::make_move_iterator(inner_mods.begin()),
                               std::make_move_iterator(inner_mods.end()));

            Token target = ErrorRecovery::try_consume_identifier(
                ctx,
                E::ExpectedIdentifier,
                RecoveryConfig::StopAtBoundary({TokenType::Colon, TokenType::Comma, TokenType::Assign})
            );

            TypeAnnPtr type_annotation = nullptr;
            if (cursor.match(TokenType::Colon))
            {
                auto is_at_parent_boundary = [this](size_t offset = 0)
                {
                    const Token& tok = cursor.peek(offset);
                    if (tok.type == TokenType::Comma)
                    {
                        const Token& after_comma = cursor.peek(offset + 1);
                        const TokenType after_comma_next = cursor.peek(offset + 2).type;
                        if (after_comma.type == TokenType::Identifier || TokenTraits::acts_like_identifier(after_comma, after_comma_next))
                        {
                            if (after_comma_next == TokenType::Colon)
                            {
                                return ErrorRecovery::is_unclosed_before_parent_boundary(ctx, TokenType::RightParen) ||
                                       ErrorRecovery::is_unclosed_before_parent_boundary(ctx, TokenType::Greater);
                            }
                            if (after_comma_next == TokenType::Assign)
                            {
                                if (ctx.is_active_closer(TokenType::Greater))
                                {
                                    return ErrorRecovery::is_unclosed_before_parent_boundary(ctx, TokenType::Greater);
                                }
                                if (ctx.is_active_closer(TokenType::RightParen))
                                {
                                    if (offset == 0 && cursor.current() >= 1)
                                    {
                                        if (cursor.previous().type == TokenType::RightParen ||
                                            (cursor.current() >= 2 && cursor.previous(2).type == TokenType::Comma))
                                        {
                                            return ErrorRecovery::is_unclosed_before_parent_boundary(ctx, TokenType::RightParen);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    return false;
                };

                type_annotation = ErrorRecovery::try_parse<TypeAnnPtr>(
                    ctx,
                    [&]() { return parser.parse_type_annotation(is_at_parent_boundary); },
                    RecoveryConfig::StopAtBoundary({TokenType::Comma, TokenType::Assign})
                );
            }
            targets.push_back(AssignmentTarget(
                std::move(target_mods),
                NodeName{target.lexeme, cursor.make_span(target)},
                std::move(type_annotation),
                cursor.make_span(target_start, cursor.previous())
            ));

            if (!cursor.match(TokenType::Comma))
            {
                if (cursor.peek().type == TokenType::Identifier || cursor.peek().type == TokenType::At)
                {
                    if (cursor.peek().line == cursor.previous().line)
                    {
                        cursor.report_error_no_panic(cursor.peek(), E::ExpectedCommaInMultiAssignment);
                        continue;
                    }
                }
                break;
            }
        }
        while (true);

        ExprPtr value = nullptr;
        auto is_at_boundary = [&]()
        {
            if (cursor.peek().line > cursor.previous().line) return true;
            if (cursor.peek().type == TokenType::At) return false;
            return cursor.is_at_end() || ctx.is_active_closer(cursor.peek().type);
        };

        if (cursor.match(TokenType::Assign))
        {
            if (is_at_boundary()) cursor.report_error_no_panic(cursor.peek(), E::MissingValueAfterEquals, false);
            else
            {
                value = ErrorRecovery::try_parse<ExprPtr>(
                    ctx, [&]() { return parser.parse_expression(); }, RecoveryConfig::StopAtNewline()
                );
                if (value && cursor.peek().type == TokenType::Assign)
                {
                    SourceSpan error_span = value->span;
                    while (cursor.peek().type == TokenType::Assign)
                    {
                        cursor.advance();
                        try
                        {
                            auto rhs = parser.parse_expression();
                            if (rhs) value = std::move(rhs);
                        }
                        catch (const ParseSyncException&)
                        {
                        }
                    }
                    cursor.report_error_no_panic(cursor.combine_spans(error_span, value->span),
                                                 E::ChainedAssignmentNotSupported);
                }
            }
        }
        else
        {
            const Token& report_at = (cursor.peek().line > cursor.previous().line && is_at_boundary()) || ctx.
                                     is_active_closer(cursor.peek().type)
                                         ? cursor.previous()
                                         : cursor.peek();
            cursor.report_error_no_panic(report_at, E::IncompleteAssignment);
            if (!is_at_boundary() && TokenTraits::is_expression_start(cursor.peek().type))
            {
                try { value = parser.parse_expression(); }
                catch (const ParseSyncException&)
                {
                }
            }
        }

        if (value) verify_statement_end();

        return AstFactory::make_node_with_span<Assignment>(
            cursor.combine_spans(start_span, cursor.make_span(cursor.previous())),
            std::move(targets), std::move(value));
    }

    StmtPtr StatementParser::parse_expression_statement()
    {
        bool prev_expr_stmt = ctx.is_parsing_expression_statement;
        ctx.is_parsing_expression_statement = true;
        auto expr = parser.parse_expression();
        ctx.is_parsing_expression_statement = prev_expr_stmt;
        const SourceSpan start_span = expr->span;

        if (cursor.match(TokenType::Comma)) cursor.report_error(cursor.previous(), E::MultiReassignmentNotSupported);

        if (cursor.match(TokenType::Assign))
        {
            if (!TokenTraits::is_valid_lvalue(expr.get()))
            {
                if (expr && expr->is_complete())
                    cursor.report_error(expr->span, E::InvalidLeftSideExpressionInReassignment);
            }

            ExprPtr value = nullptr;
            TokenType peek_target = ctx.peek_past_modifiers();
            bool is_pseudo_stmt = (cursor.peek().line > cursor.previous().line && ctx.is_at_any_declaration()) ||
                (cursor.peek().line > cursor.previous().line && cursor.peek().type != TokenType::At && TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type)) ||
                (cursor.peek().line > cursor.previous().line && peek_target == TokenType::Return) ||
                (cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type)) ||
                ctx.is_active_closer(cursor.peek().type);

            if (cursor.is_at_end() || is_pseudo_stmt)
            {
                const Token& report_at = (cursor.peek().line > cursor.previous().line && is_pseudo_stmt)
                                             ? cursor.previous()
                                             : cursor.peek();
                cursor.report_error_no_panic(report_at, E::MissingValueAfterEquals, false);
            }
            else
            {
                prev_expr_stmt = ctx.is_parsing_expression_statement;
                ctx.is_parsing_expression_statement = ctx.looks_like_reassignment();
                value = ErrorRecovery::try_parse<ExprPtr>(
                    ctx, [&]() { return parser.parse_expression(); }, RecoveryConfig::StopAtNewline()
                );
                ctx.is_parsing_expression_statement = prev_expr_stmt;

                if (value && cursor.peek().type == TokenType::Assign)
                {
                    SourceSpan error_span = value->span;
                    while (cursor.peek().type == TokenType::Assign)
                    {
                        cursor.advance();
                        try
                        {
                            ctx.is_parsing_expression_statement = ctx.looks_like_reassignment();
                            auto rhs = parser.parse_expression();
                            ctx.is_parsing_expression_statement = prev_expr_stmt;
                            if (rhs) value = std::move(rhs);
                        }
                        catch (const ParseSyncException&)
                        {
                            ctx.is_parsing_expression_statement = prev_expr_stmt;
                        }
                    }
                    cursor.report_error_no_panic(cursor.combine_spans(error_span, value->span),
                                                 E::ChainedAssignmentNotSupported);
                }
            }

            const SourceSpan end_span = value ? value->span : start_span;
            if (value) verify_statement_end();

            return AstFactory::make_node_with_span<Reassignment>(cursor.combine_spans(start_span, end_span),
                                                                 std::move(expr), std::move(value));
        }

        if (!expr || expr->kind != AstKind::FunctionCall)
        {
            if (expr && expr->is_complete()) cursor.report_error(expr->span, E::InvalidStandaloneStatement);
            return nullptr;
        }

        verify_statement_end();
        return AstFactory::make_node_with_span<ExpressionStatement>(expr->span, std::move(expr));
    }

    std::unique_ptr<ReturnStatement> StatementParser::parse_return_statement(std::vector<Modifier> modifiers)
    {
        const SourceSpan start_span = !modifiers.empty() ? modifiers.front().span : cursor.make_span(cursor.peek());
        const Token& start = cursor.advance();
        std::vector<ExprPtr> return_values;
        return_values.reserve(2);

        if (!cursor.is_at_end() && cursor.peek().line > start.line &&
            (TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) ||
             cursor.peek().type == TokenType::Return ||
             cursor.peek().type == TokenType::At ||
             cursor.peek().type == TokenType::Hash ||
             cursor.peek().type == TokenType::RightBrace ||
             TokenTraits::is_expression_statement_start(cursor.peek(), cursor.peek(1).type)))
        {
            verify_statement_end();
            return AstFactory::make_node_with_span<ReturnStatement>(
                cursor.combine_spans(start_span, cursor.make_span(cursor.previous())),
                std::move(modifiers), std::move(return_values));
        }

        do
        {
            return_values.push_back(ErrorRecovery::try_parse<ExprPtr>(
                    ctx, [&]() { return parser.parse_expression(); },
                    RecoveryConfig::ForceStopAtBoundary({TokenType::Comma}))
            );
        }
        while (cursor.match(TokenType::Comma));

        verify_statement_end();
        return AstFactory::make_node_with_span<ReturnStatement>(
            cursor.combine_spans(start_span, cursor.make_span(cursor.previous())),
            std::move(modifiers), std::move(return_values));
    }
}
