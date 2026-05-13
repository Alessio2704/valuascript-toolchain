#include "parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler
{
    void Parser::parse_statement_or_declaration(ParseContext ctx, Program* program,
                                                std::vector<std::unique_ptr<Statement>>& block)
    {
        const Token& start_token = cursor_.peek();
        std::vector<Modifier> modifiers = parse_modifiers(true);

        TokenType type = cursor_.peek().type;

        Program dummy_program;

        if (type == TokenType::EndOfFile ||
            (type == TokenType::RightBrace && ctx == ParseContext::FunctionBody))
        {
            if (!modifiers.empty())
            {
                SourceSpan span = cursor_.make_span(start_token, cursor_.previous());
                cursor_.report_error_no_panic(
                    span, ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
            }
            return;
        }

        bool is_invalid_top_level = false;

        bool prev_suppress = cursor_.get_suppress_errors();

        if (ctx == ParseContext::FunctionBody && TokenTraits::is_top_level_only_declaration(type))
        {
            is_invalid_top_level = true;
            ctx = ParseContext::TopLevel;
            program = &dummy_program;
            cursor_.set_suppress_errors(true);
        }

        try
        {
            switch (type)
            {
            case TokenType::Import:
                {
                    if (!modifiers.empty())
                    {
                        SourceSpan span = cursor_.make_span(start_token, cursor_.previous());
                        cursor_.report_error_no_panic(
                            span, ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                    }
                    auto stmt = parse_import_statement();
                    if (program) program->import_statements.push_back(std::move(stmt));
                    break;
                }
            case TokenType::Hash:
                {
                    if (!modifiers.empty())
                    {
                        SourceSpan span = cursor_.make_span(start_token, cursor_.previous());
                        cursor_.report_error_no_panic(
                            span, ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                    }
                    auto dir = parse_directive();
                    if (program) program->directives.push_back(std::move(dir));
                    break;
                }
            case TokenType::Func:
                {
                    auto func = parse_function_definition(std::move(modifiers));
                    if (program) program->function_definitions.push_back(std::move(func));
                    break;
                }
            case TokenType::Struct:
                {
                    auto str = parse_struct_definition(std::move(modifiers));
                    if (program) program->struct_definitions.push_back(std::move(str));
                    break;
                }
            case TokenType::Enum:
                {
                    auto enm = parse_enum_definition(std::move(modifiers));
                    if (program) program->enum_definitions.push_back(std::move(enm));
                    break;
                }
            case TokenType::Typealias:
                {
                    auto alias_def = parse_type_alias_definition(std::move(modifiers));
                    if (program) program->type_aliases.push_back(std::move(alias_def));
                    break;
                }
            case TokenType::Let:
                {
                    auto assign = parse_assignment(std::move(modifiers));
                    if (program) program->execution_steps.push_back(std::move(assign));
                    else block.push_back(std::move(assign));
                    break;
                }
            case TokenType::Return:
                {
                    if (!modifiers.empty())
                    {
                        SourceSpan span = cursor_.make_span(start_token, cursor_.previous());
                        cursor_.report_error_no_panic(
                            span, ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                    }

                    if (ctx == ParseContext::TopLevel)
                    {
                        const Token& ret_start = cursor_.peek();
                        auto ret = parse_return_statement();
                        SourceSpan span = cursor_.make_span(ret_start, cursor_.previous());
                        cursor_.report_error_no_panic(span, ValuascriptErrorCode::ReturnUsedInToplevel);
                        if (program) program->execution_steps.push_back(std::move(ret));
                    }
                    else
                    {
                        auto ret = parse_return_statement();
                        block.push_back(std::move(ret));
                    }
                    break;
                }
            case TokenType::Identifier:
            case TokenType::LeftParen:
            case TokenType::LeftBracket:
            default:
                {
                    if (!modifiers.empty())
                    {
                        SourceSpan span = cursor_.make_span(start_token, cursor_.previous());
                        cursor_.report_error_no_panic(
                            span, ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                    }

                    if (auto expr_stmt = parse_expression_statement())
                    {
                        if (program) program->execution_steps.push_back(std::move(expr_stmt));
                        else block.push_back(std::move(expr_stmt));
                    }
                    break;
                }
            }
        }
        catch (const ParseSyncException&)
        {
            if (is_invalid_top_level)
            {
                cursor_.set_suppress_errors(prev_suppress);
                SourceSpan span = cursor_.make_span(start_token, cursor_.previous());
                cursor_.report_error_no_panic(span, ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere);
            }
            throw;
        }

        if (is_invalid_top_level)
        {
            cursor_.set_suppress_errors(prev_suppress);
            SourceSpan span = cursor_.make_span(start_token, cursor_.previous());
            cursor_.report_error_no_panic(span, ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere);
        }
    }

    std::unique_ptr<Assignment> Parser::parse_assignment(std::vector<Modifier> modifiers)
    {
        const Token& start_token = cursor_.peek();
        cursor_.consume(TokenType::Let, ValuascriptErrorCode::ExpectedLetToken);

        std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation>>> targets;
        do
        {
            auto inner_mods = parse_modifiers();
            if (!inner_mods.empty())
            {
                auto first = inner_mods.front().span;
                auto back = inner_mods.back().span;
                auto combined_span = cursor_.combine_spans(first, back);
                cursor_.report_error_no_panic(combined_span,
                                              ValuascriptErrorCode::ModifiersAttachedToMultiAssignmentSingleElements);
            }

            Token target(TokenType::Identifier, "<error>", cursor_.peek().line, cursor_.peek().column);
            try
            {
                target = consume_identifier(ValuascriptErrorCode::InvalidIdentifier);
            }
            catch (const ParseSyncException&)
            {
                synchronize_with({
                    .stop_tokens = {TokenType::Colon, TokenType::Comma, TokenType::Assign},
                    .stop_at_statement_boundary_respecting_dangling_op = true
                });
            }

            std::unique_ptr<TypeAnnotation> type_annotation = nullptr;

            if (cursor_.match({TokenType::Colon}))
            {
                try
                {
                    type_annotation = parse_type_annotation();
                }
                catch (const ParseSyncException&)
                {
                    synchronize_with({
                        .stop_tokens = {TokenType::Comma, TokenType::Assign},
                        .stop_at_statement_boundary_respecting_dangling_op = true
                    });
                }
            }

            targets.emplace_back(target.lexeme, std::move(type_annotation));

            if (!cursor_.match({TokenType::Comma}))
            {
                if (cursor_.peek().type == TokenType::Identifier || cursor_.peek().type == TokenType::At)
                {
                    if (cursor_.peek().line == cursor_.previous().line)
                    {
                        cursor_.report_error_no_panic(cursor_.peek(),
                                                      ValuascriptErrorCode::ExpectedCommaInMultiAssignment);
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
            return cursor_.is_at_end() ||
                TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                (cursor_.peek().line > cursor_.previous().line &&
                    TokenTraits::is_expression_statement_start(cursor_.peek(), cursor_.peek(1).type)) ||
                is_active_closer(cursor_.peek().type);
        };

        if (cursor_.match({TokenType::Assign}))
        {
            if (is_at_boundary())
            {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingValueAfterEquals, false);
            }
            else
            {
                value = parse_expression();
            }
        }
        else
        {
            const Token& report_at = (cursor_.peek().line > cursor_.previous().line && is_at_boundary()) ||
                                     is_active_closer(cursor_.peek().type)
                                         ? cursor_.previous()
                                         : cursor_.peek();


            cursor_.report_error_no_panic(report_at, ValuascriptErrorCode::IncompleteAssignment);

            if (!is_at_boundary() && TokenTraits::is_expression_start(cursor_.peek().type))
            {
                try
                {
                    value = parse_expression();
                }
                catch (const ParseSyncException&)
                {
                }
            }
        }

        if (value)
        {
            verify_statement_end();
        }

        return make_node<Assignment>(start_token, std::move(modifiers), std::move(targets), std::move(value));
    }

    std::unique_ptr<Statement> Parser::parse_expression_statement()
    {
        auto expr = parse_expression();
        const SourceSpan start_span = expr->span;

        if (cursor_.match({TokenType::Comma}))
        {
            cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MultiReassignmentNotSupported);
        }

        if (cursor_.match({TokenType::Assign}))
        {
            if (!TokenTraits::is_valid_lvalue(expr.get()))
            {
                if (is_expression_complete(expr.get()))
                {
                    cursor_.report_error(cursor_.previous(),
                                         ValuascriptErrorCode::InvalidLeftSideExpressionInReassignment);
                }
            }

            std::unique_ptr<Expression> value = nullptr;

            bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
            (cursor_.peek().line > cursor_.previous().line &&
                cursor_.peek().type == TokenType::Identifier &&
                cursor_.peek(1).type == TokenType::Assign);

            if (cursor_.is_at_end() || is_pseudo_stmt)
            {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingValueAfterEquals, false);
            }
            else
            {
                value = parse_expression();
            }

            const SourceSpan end_span = value ? value->span : start_span;
            if (value) verify_statement_end();

            return make_node_with_span<Reassignment>(
                cursor_.combine_spans(start_span, end_span), std::move(expr), std::move(value));
        }

        if (dynamic_cast<FunctionCall*>(expr.get()) == nullptr)
        {
            if (is_expression_complete(expr.get()))
            {
                cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::InvalidStandaloneStatement);
            }

            return nullptr;
        }

        verify_statement_end();

        return make_node_with_span<ExpressionStatement>(start_span, std::move(expr));
    }

    std::unique_ptr<ReturnStatement> Parser::parse_return_statement()
    {
        const Token& start_token = cursor_.advance();

        std::vector<std::unique_ptr<Expression>> return_values;

        do
        {
            try
            {
                return_values.push_back(parse_expression());
            }
            catch (const ParseSyncException&)
            {
                return_values.push_back(nullptr);

                synchronize_with({
                    .stop_tokens = {TokenType::Comma},
                    .stop_at_statement_boundary_respecting_dangling_op = true,
                    .force_stop_at_statement_boundary_ignoring_dangling_op = true
                });
            }
        }
        while (cursor_.match({TokenType::Comma}));

        verify_statement_end();

        return make_node<ReturnStatement>(start_token, std::move(return_values));
    }
}