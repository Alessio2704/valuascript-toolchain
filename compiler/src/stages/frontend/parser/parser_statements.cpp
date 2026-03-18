#include "stages/frontend/parser/parser.h"
#include "token/reserved_keyword_lookup.h"

namespace valuascript::compiler {
    std::unique_ptr<Statement> Parser::parse_statement() {
        std::vector<Modifier> modifiers;
        if (cursor_.check(TokenType::At)) {
            modifiers = parse_modifiers();
        }

        std::unique_ptr<Statement> stmt;
        switch (cursor_.peek().type) {
            case TokenType::Let:
            case TokenType::Var:
                stmt = parse_assignment(std::move(modifiers));
                break;

            case TokenType::Return:
                if (!modifiers.empty()) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ModifiersOnNonVariableDeclaration);
                }
                stmt = parse_return_statement();
                break;

            case TokenType::Enum:
            case TokenType::Struct:
            case TokenType::Func:
            case TokenType::Hash:
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::TopLevelDeclarationInsideFunction, true);
                break;
            default:
                if (!modifiers.empty()) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ModifiersOnNonVariableDeclaration);
                }
                stmt = parse_expression_statement();
                break;
        }

        verify_statement_end();
        return stmt;
    }

    std::unique_ptr<Assignment> Parser::parse_assignment(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.peek();
        bool is_mutable = cursor_.match({TokenType::Var});
        if (!is_mutable) cursor_.consume(TokenType::Let, ValuascriptErrorCode::ExpectedLetOrVarToken);

        std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > targets;
        do {
            if (is_reserved_keyword(cursor_.peek())) {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ReservedKeywordAsIdentifier);
            }
            const Token &target = cursor_.consume(TokenType::Identifier, ValuascriptErrorCode::InvalidIdentifier);
            std::unique_ptr<TypeAnnotation> type_annotation = nullptr;
            if (cursor_.match({TokenType::Colon})) {
                type_annotation = parse_type_annotation();
            }

            targets.emplace_back(target.lexeme, std::move(type_annotation));
            if (!cursor_.match({TokenType::Comma})) {
                if (cursor_.peek().type == TokenType::Identifier) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ExpectedCommaInMultiAssignment);
                }
                break;
            }
        } while (true);
        cursor_.consume(TokenType::Assign, ValuascriptErrorCode::IncompleteAssignment);
        if (cursor_.is_at_end() || cursor_.check(TokenType::Let) || cursor_.check(TokenType::Var) ||
            cursor_.check(TokenType::Func) || cursor_.check(TokenType::At)) {
            cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MissingValueAfterEquals);
        }

        auto value = parse_expression();

        verify_statement_end();
        auto assign = std::make_unique<Assignment>(std::move(modifiers), std::move(targets), std::move(value),
                                                   is_mutable);
        assign->span = cursor_.make_span(start_token, cursor_.previous());
        return assign;
    }

    std::unique_ptr<Statement> Parser::parse_expression_statement() {
        auto expr = parse_expression();
        SourceSpan start_span = expr->span;

        if (cursor_.match({TokenType::Comma})) {
            cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MultiReassignmentNotSupported);
        }

        if (cursor_.match({TokenType::Assign})) {
            if (!is_valid_lvalue(expr.get())) {
                cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::InvalidLeftSideExpressionInReassignment);
            }

            auto value = parse_expression();
            SourceSpan end_span = value->span;
            verify_statement_end();

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
