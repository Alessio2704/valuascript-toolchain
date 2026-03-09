#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    std::unique_ptr<Statement> Parser::parse_statement() {
        std::vector<Modifier> modifiers;
        if (cursor_.check(TokenType::At)) {
            modifiers = parse_modifiers();
        }

        switch (cursor_.peek().type) {
            case TokenType::Let:
            case TokenType::Var:
                return parse_assignment(std::move(modifiers));

            case TokenType::Return:
                if (!modifiers.empty()) {
                    throw cursor_.error(cursor_.peek(), ErrorCode::UnexpectedToken,
                                        "Syntax Error: Modifiers can only be attached to variable declarations (let, var).");
                }
                return parse_return_statement();

            default:
                if (!modifiers.empty()) {
                    throw cursor_.error(cursor_.peek(), ErrorCode::UnexpectedToken,
                                        "Syntax Error: Modifiers can only be attached to variable declarations (let, var).");
                }
                return parse_expression_statement();
        }
    }

    std::unique_ptr<Assignment> Parser::parse_assignment(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.peek();
        bool is_mutable = false;

        if (cursor_.match({TokenType::Let})) {
            is_mutable = false;
        } else if (cursor_.match({TokenType::Var})) {
            is_mutable = true;
        }

        std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > targets;
        do {
            if (is_reserved_keyword(cursor_.peek().type)) {
                throw cursor_.error(cursor_.peek(), ErrorCode::ReservedKeywordAsIdentifier,
                                    "Syntax Error: Cannot use a reserved keyword as a variable name.");
            }
            const Token &target = cursor_.consume(TokenType::Identifier, ErrorCode::InvalidIdentifier,
                                                  "Syntax Error: Invalid identifier name.");

            std::unique_ptr<TypeAnnotation> type_annotation = nullptr;
            if (cursor_.match({TokenType::Colon})) {
                type_annotation = parse_type_annotation();
            }

            targets.emplace_back(target.lexeme, std::move(type_annotation));
        } while (cursor_.match({TokenType::Comma}));

        cursor_.consume(TokenType::Assign, ErrorCode::IncompleteAssignment,
                        "Syntax Error: Incomplete assignment. Expected '='.");

        if (cursor_.is_at_end() || cursor_.check(TokenType::Let) || cursor_.check(TokenType::Var) || cursor_.
            check(TokenType::Func) || cursor_.check(TokenType::At)) {
            throw cursor_.error(cursor_.previous(), ErrorCode::MissingValueAfterEquals,
                                "Syntax Error: Missing value after '='.");
        }

        auto value = parse_expression();
        auto assign = std::make_unique<Assignment>(std::move(modifiers), std::move(targets), std::move(value),
                                                   is_mutable);
        assign->span = cursor_.make_span(start_token, cursor_.previous());
        return assign;
    }

    std::unique_ptr<Statement> Parser::parse_expression_statement() {
        auto expr = parse_expression();
        SourceSpan start_span = expr->span;

        if (cursor_.match({TokenType::Comma})) {
            throw cursor_.error(cursor_.previous(), ErrorCode::MultiReassignmentNotSupported,
                                "Syntax Error: Multiple reassignment is not supported. You must reassign variables individually.");
        }

        if (cursor_.match({TokenType::Assign})) {
            if (!is_valid_lvalue(expr.get())) {
                throw cursor_.error(cursor_.previous(), ErrorCode::InvalidLeftSideExpressionInReassignment,
                                    "Syntax Error: Invalid assignment target. You can only assign to variables, properties, or indices.");
            }

            auto value = parse_expression();
            SourceSpan end_span = value->span;

            auto reassignment = std::make_unique<Reassignment>(std::move(expr), std::move(value));
            reassignment->span = cursor_.combine_spans(start_span, end_span);
            return reassignment;
        }

        if (dynamic_cast<FunctionCall *>(expr.get()) == nullptr) {
            throw cursor_.error(cursor_.previous(), ErrorCode::InvalidStandaloneStatement,
                                "Syntax Error: Invalid statement. Expected an assignment, reassignment, or function call.");
        }

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

        auto ret_stmt = std::make_unique<ReturnStatement>(std::move(return_values));
        ret_stmt->span = cursor_.make_span(start_token, cursor_.previous());
        return ret_stmt;
    }
}
