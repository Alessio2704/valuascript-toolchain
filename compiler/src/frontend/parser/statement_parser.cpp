#include "statement_parser.h"
#include "parser.h"
#include "expression_parser.h"
#include "declaration_parser.h"
#include "type_parser.h"

namespace valuascript::compiler
{
    using E = ValuascriptErrorCode;

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

        std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation>>> targets;
        do
        {
            auto inner_mods = parser.decl_parser->parse_modifiers();
            ctx.reject_modifiers(inner_mods, E::ModifiersAttachedToMultiAssignmentSingleElements);

            Token target = ctx.try_consume_identifier(E::InvalidIdentifier,
                                                      RecoveryConfig::StopAtBoundary({
                                                          TokenType::Colon, TokenType::Comma, TokenType::Assign
                                                      }));

            std::unique_ptr<TypeAnnotation> type_annotation = nullptr;
            if (cursor.match({TokenType::Colon}))
            {
                type_annotation = ctx.try_parse<std::unique_ptr<TypeAnnotation>>(
                    [&]() { return parser.type_parser->parse_type_annotation(); },
                    RecoveryConfig::StopAtBoundary({TokenType::Comma, TokenType::Assign}));
            }
            targets.emplace_back(target.lexeme, std::move(type_annotation));

            if (!cursor.match({TokenType::Comma}))
            {
                if (cursor.peek().type == TokenType::Identifier || cursor.peek().type == TokenType::At)
                {
                    if (cursor.peek().line == cursor.previous().line)
                    {
                        cursor.report_error_no_panic(cursor.peek(),
                                                     E::ExpectedCommaInMultiAssignment);
                        continue;
                    }
                }
                break;
            }
        }
        while (true);

        std::unique_ptr<Expression> value = nullptr;
        auto is_at_boundary = [&]()
        {
            return cursor.is_at_end() || TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) ||
                (cursor.peek().line > cursor.previous().line && TokenTraits::is_expression_statement_start(
                    cursor.peek(), cursor.peek(1).type)) ||
                ctx.is_active_closer(cursor.peek().type);
        };

        if (cursor.match({TokenType::Assign}))
        {
            if (is_at_boundary())
                cursor.report_error_no_panic(cursor.peek(),
                                             E::MissingValueAfterEquals, false);
            else value = parser.expr_parser->parse_expression();
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
                try { value = parser.expr_parser->parse_expression(); }
                catch (const ParseSyncException&)
                {
                }
            }
        }

        if (value) verify_statement_end();
        return ctx.make_node<Assignment>(start, std::move(modifiers), std::move(targets), std::move(value));
    }

    std::unique_ptr<Statement> StatementParser::parse_expression_statement()
    {
        auto expr = parser.expr_parser->parse_expression();
        const SourceSpan start_span = expr->span;

        if (cursor.match({TokenType::Comma}))
            cursor.report_error(cursor.previous(),
                                E::MultiReassignmentNotSupported);

        if (cursor.match({TokenType::Assign}))
        {
            if (!TokenTraits::is_valid_lvalue(expr.get()))
            {
                if (parser.expr_parser->is_expression_complete(expr.get()))
                    cursor.report_error(
                        cursor.previous(), E::InvalidLeftSideExpressionInReassignment);
            }

            std::unique_ptr<Expression> value = nullptr;
            bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor.peek(), cursor.peek(1).type) ||
            (cursor.peek().line > cursor.previous().line && cursor.peek().type == TokenType::Identifier && cursor.
                peek(1).type == TokenType::Assign);

            if (cursor.is_at_end() || is_pseudo_stmt)
                cursor.report_error_no_panic(
                    cursor.peek(), E::MissingValueAfterEquals, false);
            else value = parser.expr_parser->parse_expression();

            const SourceSpan end_span = value ? value->span : start_span;
            if (value) verify_statement_end();

            return ctx.make_node_with_span<Reassignment>(cursor.combine_spans(start_span, end_span), std::move(expr),
                                                         std::move(value));
        }

        if (dynamic_cast<FunctionCall*>(expr.get()) == nullptr)
        {
            if (parser.expr_parser->is_expression_complete(expr.get()))
                cursor.report_error(
                    cursor.previous(), E::InvalidStandaloneStatement);
            return nullptr;
        }

        verify_statement_end();
        return ctx.make_node_with_span<ExpressionStatement>(start_span, std::move(expr));
    }

    std::unique_ptr<ReturnStatement> StatementParser::parse_return_statement()
    {
        const Token& start = cursor.advance();
        std::vector<std::unique_ptr<Expression>> return_values;

        do
        {
            return_values.push_back(
                ctx.try_parse<std::unique_ptr<Expression>>([&]() { return parser.expr_parser->parse_expression(); },
                                                           RecoveryConfig::ForceStopAtBoundary({TokenType::Comma}))
            );
        }
        while (cursor.match({TokenType::Comma}));

        verify_statement_end();
        return ctx.make_node<ReturnStatement>(start, std::move(return_values));
    }
}
