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
                cursor.report_error(cursor.peek(), E::MissingOperator);
            }
        }
    }

    std::unique_ptr<Assignment> StatementParser::parse_assignment(std::vector<Modifier> modifiers)
    {
        const Token& start = cursor.peek();
        cursor.consume(TokenType::Let, E::ExpectedLetToken);

        std::vector<std::pair<std::string, TypeAnnPtr>> targets;
        do
        {
            auto inner_mods = parser.parse_modifiers();
            ctx.reject_modifiers(inner_mods, E::ModifiersAttachedToMultiAssignmentSingleElements);

            Token target = ErrorRecovery::try_consume_identifier(
                ctx,
                E::ExpectedIdentifier,
                RecoveryConfig::StopAtBoundary({TokenType::Colon, TokenType::Comma, TokenType::Assign})
            );

            TypeAnnPtr type_annotation = nullptr;
            if (cursor.match({TokenType::Colon}))
            {
                type_annotation = ErrorRecovery::try_parse<TypeAnnPtr>(
                    ctx,
                    [&]() { return parser.parse_type_annotation(); },
                    RecoveryConfig::StopAtBoundary({TokenType::Comma, TokenType::Assign})
                );
            }
            targets.emplace_back(target.lexeme, std::move(type_annotation));

            if (!cursor.match({TokenType::Comma}))
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
            return cursor.is_at_end() || TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) ||
                (cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), cursor.peek(1).type)) ||
                ctx.is_active_closer(cursor.peek().type);
        };

        if (cursor.match({TokenType::Assign}))
        {
            if (is_at_boundary()) cursor.report_error_no_panic(cursor.peek(), E::MissingValueAfterEquals, false);
            else value = parser.parse_expression();
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
        return AstFactory::make_node<Assignment>(cursor, start, std::move(modifiers), std::move(targets),
                                                 std::move(value));
    }

    StmtPtr StatementParser::parse_expression_statement()
    {
        auto expr = parser.parse_expression();
        const SourceSpan start_span = expr->span;

        if (cursor.match({TokenType::Comma})) cursor.report_error(cursor.previous(), E::MultiReassignmentNotSupported);

        if (cursor.match({TokenType::Assign}))
        {
            if (!TokenTraits::is_valid_lvalue(expr.get()))
            {
                if (expr && expr->is_complete())
                {
                    cursor.report_error(cursor.previous(), E::InvalidLeftSideExpressionInReassignment);
                }
            }

            ExprPtr value = nullptr;
            bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) ||
            (cursor.peek().line > cursor.previous().line && cursor.peek().type == TokenType::Identifier && cursor.
                peek(1).type == TokenType::Assign);

            if (cursor.is_at_end() || is_pseudo_stmt)
            {
                cursor.report_error_no_panic(cursor.peek(), E::MissingValueAfterEquals, false);
            }
            else
            {
                value = parser.parse_expression();
            }

            const SourceSpan end_span = value ? value->span : start_span;
            if (value) verify_statement_end();

            return AstFactory::make_node_with_span<Reassignment>(cursor.combine_spans(start_span, end_span),
                                                                 std::move(expr), std::move(value));
        }

        if (dynamic_cast<FunctionCall*>(expr.get()) == nullptr)
        {
            if (expr && expr->is_complete()) cursor.report_error(cursor.previous(), E::InvalidStandaloneStatement);
            return nullptr;
        }

        verify_statement_end();
        return AstFactory::make_node_with_span<ExpressionStatement>(start_span, std::move(expr));
    }

    std::unique_ptr<ReturnStatement> StatementParser::parse_return_statement()
    {
        const Token& start = cursor.advance();
        std::vector<ExprPtr> return_values;

        do
        {
            return_values.push_back(ErrorRecovery::try_parse<ExprPtr>(
                    ctx, [&]() { return parser.parse_expression(); },
                    RecoveryConfig::ForceStopAtBoundary({TokenType::Comma}))
            );
        }
        while (cursor.match({TokenType::Comma}));

        verify_statement_end();
        return AstFactory::make_node<ReturnStatement>(cursor, start, std::move(return_values));
    }
}
