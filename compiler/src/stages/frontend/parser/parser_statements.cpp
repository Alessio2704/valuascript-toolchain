#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    void Parser::parse_statement_or_declaration(ParseContext ctx, Program *program,
                                                std::vector<std::unique_ptr<Statement> > &block) {
        std::vector<Modifier> modifiers = parse_modifiers(true);

        TokenType type = cursor_.peek().type;

        Program dummy_program;

        if (type == TokenType::EndOfFile) {
            if (!modifiers.empty()) {
                cursor_.report_error_no_panic(cursor_.peek(),
                                              ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
            }
            return;
        }

        if (ctx == ParseContext::FunctionBody && TokenTraits::is_top_level_only_declaration(type)) {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::TopLevelDeclarationNotAllowedHere,
                                          true);

            ctx = ParseContext::TopLevel;
            program = &dummy_program;
        }

        switch (type) {
            case TokenType::Import: {
                if (!modifiers.empty()) {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                }
                auto stmt = parse_import_statement();
                if (program) program->import_statements.push_back(std::move(stmt));
                break;
            }
            case TokenType::Hash: {
                if (!modifiers.empty()) {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                }
                auto dir = parse_directive();
                if (program) program->directives.push_back(std::move(dir));
                break;
            }
            case TokenType::Func: {
                auto func = parse_function_definition(std::move(modifiers));
                if (program) program->function_definitions.push_back(std::move(func));
                break;
            }
            case TokenType::Struct: {
                auto str = parse_struct_definition(std::move(modifiers));
                if (program) program->struct_definitions.push_back(std::move(str));
                break;
            }
            case TokenType::Enum: {
                auto enm = parse_enum_definition(std::move(modifiers));
                if (program) program->enum_definitions.push_back(std::move(enm));
                break;
            }
            case TokenType::Typealias: {
                auto alias_def = parse_type_alias_definition(std::move(modifiers));
                if (program) program->type_aliases.push_back(std::move(alias_def));
                break;
            }
            case TokenType::Let:
            case TokenType::Var: {
                auto assign = parse_assignment(std::move(modifiers));
                if (program) program->execution_steps.push_back(std::move(assign));
                else block.push_back(std::move(assign));
                break;
            }
            case TokenType::Return: {
                if (!modifiers.empty()) {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                }

                if (ctx == ParseContext::TopLevel) {
                    cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::ReturnUsedInToplevel, true);
                    auto ret = parse_return_statement();
                    if (program) program->execution_steps.push_back(std::move(ret));
                } else {
                    auto ret = parse_return_statement();
                    block.push_back(std::move(ret));
                }
                break;
            }
            case TokenType::Identifier:
            case TokenType::LeftParen:
            case TokenType::LeftBracket:
            default: {
                if (!modifiers.empty()) {
                    cursor_.report_error_no_panic(cursor_.peek(),
                                                  ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                }
                auto expr_stmt = parse_expression_statement();
                if (program) program->execution_steps.push_back(std::move(expr_stmt));
                else block.push_back(std::move(expr_stmt));
                break;
            }
        }
    }

    std::unique_ptr<Assignment> Parser::parse_assignment(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.peek();

        bool is_mutable = cursor_.match({TokenType::Var});
        if (!is_mutable) cursor_.consume(TokenType::Let, ValuascriptErrorCode::ExpectedLetOrVarToken);

        std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > targets;
        do {
            auto inner_mods = parse_modifiers();
            if (!inner_mods.empty()) {
                auto first = inner_mods.front().span;
                auto back = inner_mods.back().span;
                auto combined_span = cursor_.combine_spans(first, back);
                cursor_.report_error_no_panic(combined_span,
                                              ValuascriptErrorCode::ModifiersAttachedToMultiAssignmentSingleElements);
            }

            const Token &target = consume_identifier(ValuascriptErrorCode::InvalidIdentifier);

            std::unique_ptr<TypeAnnotation> type_annotation = nullptr;

            if (cursor_.match({TokenType::Colon})) {
                type_annotation = parse_type_annotation();
            }

            targets.emplace_back(target.lexeme, std::move(type_annotation));

            if (!cursor_.match({TokenType::Comma})) {
                if (cursor_.peek().type == TokenType::Identifier || cursor_.peek().type == TokenType::At) {
                    cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::ExpectedCommaInMultiAssignment);
                    continue;
                }
                break;
            }
        } while (true);

        std::unique_ptr<Expression> value = nullptr;

        if (cursor_.match({TokenType::Assign})) {
            bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                                  (cursor_.peek().line > cursor_.previous().line &&
                                   cursor_.peek().type == TokenType::Identifier &&
                                   cursor_.peek(1).type == TokenType::Assign);

            if (cursor_.is_at_end() || is_pseudo_stmt) {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingValueAfterEquals, false);
            } else {
                value = parse_expression();
            }
        } else {
            cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::IncompleteAssignment);

            if (cursor_.is_at_end() || TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type)) {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingValueAfterEquals);
            } else {
                throw ParseSyncException();
            }
        }

        if (value) {
            verify_statement_end();
        }

        auto assign = std::make_unique<Assignment>(std::move(modifiers), std::move(targets), std::move(value),
                                                   is_mutable);
        assign->span = cursor_.make_span(start_token, cursor_.previous());
        return assign;
    }

    std::unique_ptr<Statement> Parser::parse_expression_statement() {
        auto expr = parse_expression();
        const SourceSpan start_span = expr->span;

        if (cursor_.match({TokenType::Comma})) {
            cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MultiReassignmentNotSupported);
        }

        if (cursor_.match({TokenType::Assign})) {
            if (!TokenTraits::is_valid_lvalue(expr.get())) {
                cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::InvalidLeftSideExpressionInReassignment);
            }

            std::unique_ptr<Expression> value = nullptr;

            bool is_pseudo_stmt = TokenTraits::is_statement_start(cursor_.peek(), cursor_.peek(1).type) ||
                                  (cursor_.peek().line > cursor_.previous().line &&
                                   cursor_.peek().type == TokenType::Identifier &&
                                   cursor_.peek(1).type == TokenType::Assign);

            if (cursor_.is_at_end() || is_pseudo_stmt) {
                cursor_.report_error_no_panic(cursor_.peek(), ValuascriptErrorCode::MissingValueAfterEquals, false);
            } else {
                value = parse_expression();
            }

            const SourceSpan end_span = value ? value->span : start_span;
            if (value) verify_statement_end();

            auto reassignment = std::make_unique<Reassignment>(std::move(expr), std::move(value));
            reassignment->span = cursor_.combine_spans(start_span, end_span);
            return reassignment;
        }

        if (dynamic_cast<FunctionCall *>(expr.get()) == nullptr) {
            cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::InvalidStandaloneStatement);
        }

        verify_statement_end();

        auto expr_stmt = std::make_unique<ExpressionStatement>(std::move(expr));
        expr_stmt->span = start_span;
        return expr_stmt;
    }

    std::unique_ptr<ReturnStatement> Parser::parse_return_statement() {
        const Token &start_token = cursor_.advance();

        std::vector<std::unique_ptr<Expression> > return_values;

        do {
            return_values.push_back(parse_expression());
        } while (cursor_.match({TokenType::Comma}));

        verify_statement_end();

        auto ret_stmt = std::make_unique<ReturnStatement>(std::move(return_values));
        ret_stmt->span = cursor_.make_span(start_token, cursor_.previous());
        return ret_stmt;
    }
}
