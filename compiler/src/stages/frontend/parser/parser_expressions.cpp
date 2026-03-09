#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    std::unique_ptr<Expression> Parser::parse_expression() {
        if (cursor_.match({TokenType::If})) {
            const Token &start_token = cursor_.previous();
            auto condition = parse_or_expression();
            cursor_.consume(TokenType::Then, ErrorCode::MissingThenToken, "Expected 'then' after condition.");
            auto then_branch = parse_or_expression();
            cursor_.consume(TokenType::Else, ErrorCode::MissingElseToken, "Expected 'else' after then branch.");
            auto else_branch = parse_expression();

            auto cond_expr = std::make_unique<ConditionalExpression>(std::move(condition), std::move(then_branch),
                                                                     std::move(else_branch));
            cond_expr->span = cursor_.make_span(start_token, cursor_.previous());
            return cond_expr;
        }
        return parse_or_expression();
    }

    std::unique_ptr<Expression> Parser::parse_or_expression() {
        auto expr = parse_and_expression();
        while (cursor_.match({TokenType::Or})) {
            Token op = cursor_.previous();
            auto right = parse_and_expression();
            SourceSpan combined = cursor_.combine_spans(expr->span, right->span);
            expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
            expr->span = combined;
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_and_expression() {
        auto expr = parse_comparison_expression();
        while (cursor_.match({TokenType::And})) {
            Token op = cursor_.previous();
            auto right = parse_comparison_expression();
            SourceSpan combined = cursor_.combine_spans(expr->span, right->span);
            expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
            expr->span = combined;
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_comparison_expression() {
        auto expr = parse_addition_expression();

        if (cursor_.match({
            TokenType::Equals, TokenType::NotEquals, TokenType::Greater, TokenType::GreaterEqual, TokenType::Less,
            TokenType::LessEqual
        })) {
            Token op = cursor_.previous();
            auto right = parse_addition_expression();
            SourceSpan combined = cursor_.combine_spans(expr->span, right->span);
            expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
            expr->span = combined;

            if (cursor_.match({
                TokenType::Equals, TokenType::NotEquals, TokenType::Greater, TokenType::GreaterEqual, TokenType::Less,
                TokenType::LessEqual
            })) {
                throw cursor_.error(cursor_.previous(), ErrorCode::ChainingNotAllowedForComparisonOperations,
                                    "Syntax Error: Chaining comparison operators is not allowed.");
            }
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_addition_expression() {
        auto expr = parse_multiplication_expression();
        while (cursor_.match({TokenType::Plus, TokenType::Minus})) {
            Token op = cursor_.previous();
            auto right = parse_multiplication_expression();
            SourceSpan combined = cursor_.combine_spans(expr->span, right->span);
            expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
            expr->span = combined;
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_multiplication_expression() {
        auto expr = parse_power_expression();
        while (cursor_.match({TokenType::Star, TokenType::Slash, TokenType::Mod})) {
            Token op = cursor_.previous();
            auto right = parse_power_expression();
            SourceSpan combined = cursor_.combine_spans(expr->span, right->span);
            expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
            expr->span = combined;
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_power_expression() {
        auto expr = parse_unary_expression();
        while (cursor_.match({TokenType::Caret})) {
            Token op = cursor_.previous();
            auto right = parse_unary_expression();
            SourceSpan combined = cursor_.combine_spans(expr->span, right->span);
            expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
            expr->span = combined;
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_unary_expression() {
        if (cursor_.match({TokenType::Minus, TokenType::Plus, TokenType::Not})) {
            Token op = cursor_.previous();
            auto right = parse_unary_expression();
            auto unary = std::make_unique<UnaryExpression>(op.type, std::move(right));
            unary->span = cursor_.make_span(op, cursor_.previous());
            return unary;
        }
        return parse_postfix_expression();
    }

    std::unique_ptr<Expression> Parser::parse_postfix_expression() {
        auto expr = parse_primary_expression();

        while (true) {
            SourceSpan start_span = expr->span;

            if (cursor_.match({TokenType::LeftParen})) {
                expr = parse_function_call(std::move(expr));
            } else if (cursor_.match({TokenType::LeftBracket})) {
                expr = parse_tensor_access(std::move(expr));
            } else if (cursor_.match({TokenType::Dot})) {
                Token property_token = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedPropertyName,
                                                       "Expected property name after '.'.");
                expr = std::make_unique<DotAccess>(std::move(expr), property_token.lexeme);
                expr->span = cursor_.combine_spans(start_span, cursor_.make_span(property_token, property_token));
            } else {
                break;
            }
        }
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_primary_expression() {
        switch (cursor_.peek().type) {
            case TokenType::Number: {
                cursor_.advance();
                auto node = std::make_unique<NumberLiteral>(cursor_.previous().lexeme);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::PercentageLiteral: {
                cursor_.advance();
                auto node = std::make_unique<PercentageLiteral>(cursor_.previous().lexeme);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::String: {
                cursor_.advance();
                auto node = std::make_unique<StringLiteral>(cursor_.previous().lexeme);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::True:
            case TokenType::False: {
                cursor_.advance();
                auto node = std::make_unique<BooleanLiteral>(cursor_.previous().type == TokenType::True);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::Identifier: {
                cursor_.advance();
                auto node = std::make_unique<IdentifierAccess>(cursor_.previous().lexeme);
                node->span = cursor_.make_span(cursor_.previous(), cursor_.previous());
                return node;
            }
            case TokenType::Switch:
                cursor_.advance();
                return parse_switch_expression();
            case TokenType::LeftParen:
                cursor_.advance();
                return parse_tuple_or_grouping();
            case TokenType::LeftBracket:
                cursor_.advance();
                return parse_tensor_literal();
            case TokenType::LeftBrace:
                cursor_.advance();
                return parse_dict_literal();
            default:
                throw cursor_.error(cursor_.peek(), ErrorCode::InvalidExpression,
                                    "Syntax Error: Expected an expression.");
        }
    }

    std::unique_ptr<Expression> Parser::parse_tuple_or_grouping() {
        const Token &start_token = cursor_.previous();

        if (cursor_.match({TokenType::RightParen})) {
            auto node = std::make_unique<TupleLiteral>(std::vector<std::unique_ptr<Expression> >{});
            node->span = cursor_.make_span(start_token, cursor_.previous());
            return node;
        }

        auto expr = parse_expression();

        if (cursor_.match({TokenType::Comma})) {
            std::vector<std::unique_ptr<Expression> > elements;
            elements.push_back(std::move(expr));

            do {
                elements.push_back(parse_expression());
            } while (cursor_.match({TokenType::Comma}));

            const Token &end_token = cursor_.consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesisInTuple,
                                                     "Expected ')' after tuple elements.");
            auto node = std::make_unique<TupleLiteral>(std::move(elements));
            node->span = cursor_.make_span(start_token, end_token);
            return node;
        }

        const Token &end_token = cursor_.consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesis,
                                                 "Expected ')' after expression.");
        expr->span = cursor_.make_span(start_token, end_token);
        return expr;
    }

    std::unique_ptr<Expression> Parser::parse_tensor_literal() {
        const Token &start_token = cursor_.previous();
        std::vector<std::unique_ptr<Expression> > elements;
        if (!cursor_.check(TokenType::RightBracket)) {
            do {
                elements.push_back(parse_expression());
            } while (cursor_.match({TokenType::Comma}));
        }
        const Token &end_token = cursor_.consume(TokenType::RightBracket, ErrorCode::UnmatchedBracket,
                                                 "Expected ']' after vector elements.");

        auto node = std::make_unique<TensorLiteral>(std::move(elements));
        node->span = cursor_.make_span(start_token, end_token);
        return node;
    }

    std::unique_ptr<Expression> Parser::parse_dict_literal() {
        const Token &start_token = cursor_.previous();
        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > pairs;

        if (!cursor_.check(TokenType::RightBrace)) {
            do {
                Token key_token = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedDictionaryKey,
                                                  "Expected key in dictionary.");
                cursor_.consume(TokenType::Colon, ErrorCode::ExpectedColonAfterDictionaryKey,
                                "Expected ':' after dictionary key.");
                pairs.emplace_back(key_token.lexeme, parse_expression());
            } while (cursor_.match({TokenType::Comma}));
        }

        const Token &end_token = cursor_.consume(TokenType::RightBrace, ErrorCode::UnmatchedBraceInDictionaryLiteral,
                                                 "Expected '}' after dictionary literal.");
        auto node = std::make_unique<DictLiteral>(std::move(pairs));
        node->span = cursor_.make_span(start_token, end_token);
        return node;
    }

    std::unique_ptr<Expression> Parser::parse_function_call(std::unique_ptr<Expression> target) {
        SourceSpan target_span = target->span;
        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

        if (!cursor_.check(TokenType::RightParen)) {
            do {
                Token arg_name = cursor_.consume(TokenType::Identifier, ErrorCode::MissingArgumentName,
                                                 "Expected argument name in function call.");
                cursor_.consume(TokenType::Colon, ErrorCode::MissingColonAfterArgument,
                                "Expected ':' after argument name.");
                arguments.emplace_back(arg_name.lexeme, parse_expression());
            } while (cursor_.match({TokenType::Comma}));
        }

        const Token &end_token = cursor_.consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesis,
                                                 "Expected ')' after arguments.");
        auto func_call = std::make_unique<FunctionCall>(std::move(target), std::move(arguments));
        func_call->span = cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token));
        return func_call;
    }

    std::unique_ptr<Expression> Parser::parse_tensor_access(std::unique_ptr<Expression> target) {
        SourceSpan target_span = target->span;
        std::unique_ptr<Expression> index_expr = nullptr;

        if (!cursor_.check(TokenType::Colon) && !cursor_.check(TokenType::RightBracket)) {
            index_expr = parse_expression();
        }

        if (cursor_.match({TokenType::Colon})) {
            std::unique_ptr<Expression> end_expr = nullptr;
            if (!cursor_.check(TokenType::RightBracket)) {
                end_expr = parse_expression();
            }

            SourceSpan colon_span = index_expr ? index_expr->span : target_span;
            SourceSpan slice_end_span = end_expr
                                            ? end_expr->span
                                            : cursor_.make_span(cursor_.previous(), cursor_.previous());

            index_expr = std::make_unique<BinaryExpression>(std::move(index_expr), TokenType::Colon,
                                                            std::move(end_expr));
            index_expr->span = cursor_.combine_spans(colon_span, slice_end_span);
        } else if (!index_expr) {
            throw cursor_.error(cursor_.previous(), ErrorCode::EmptyBracketAccess,
                                "Expected an index or slice inside '[]'.");
        }

        const Token &end_token = cursor_.consume(TokenType::RightBracket, ErrorCode::UnmatchedBracket,
                                                 "Expected ']' after vector index.");
        auto bracket_access = std::make_unique<BracketAccess>(std::move(target), std::move(index_expr));
        bracket_access->span = cursor_.combine_spans(target_span, cursor_.make_span(end_token, end_token));
        return bracket_access;
    }

    std::unique_ptr<Expression> Parser::parse_switch_expression() {
        const Token &start_token = cursor_.previous();
        cursor_.consume(TokenType::LeftParen, ErrorCode::ExpectedLeftParen, "Expected '(' after 'switch'.");
        auto target = parse_expression();
        cursor_.consume(TokenType::RightParen, ErrorCode::ExpectedRightParen, "Expected ')' after switch target.");
        cursor_.consume(TokenType::LeftBrace, ErrorCode::ExpectedLeftBrace, "Expected '{' before switch body.");

        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > cases;
        std::unique_ptr<Expression> default_case = nullptr;

        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end()) {
            if (cursor_.match({TokenType::Case})) {
                std::vector<std::string> case_identifiers;
                do {
                    Token id_token = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedEnumCaseName,
                                                     "Expected enum case identifier after 'case'.");
                    case_identifiers.push_back(id_token.lexeme);
                } while (cursor_.match({TokenType::Comma}));

                cursor_.consume(TokenType::Arrow, ErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier,
                                "Expected '->' after case identifiers.");
                auto result_expr = parse_expression();
                cases.emplace_back(std::move(case_identifiers), std::move(result_expr));
            } else if (cursor_.match({TokenType::Default})) {
                if (default_case != nullptr) {
                    throw cursor_.error(cursor_.peek(), ErrorCode::MultipleDefaultCasesInSwitch,
                                        "Syntax Error: A switch expression can only have one 'default' case.");
                }
                cursor_.consume(TokenType::Arrow, ErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier,
                                "Expected '->' after 'default'.");
                default_case = parse_expression();
            } else {
                throw cursor_.error(cursor_.peek(), ErrorCode::CaseOrDefaultMissingInSwitch,
                                    "Syntax Error: Expected 'case' or 'default' inside switch body.");
            }
        }

        const Token &end_token = cursor_.consume(TokenType::RightBrace, ErrorCode::ExpectedRightBrace,
                                                 "Expected '}' after switch body.");
        auto switch_expr = std::make_unique<SwitchExpression>(std::move(target), std::move(cases),
                                                              std::move(default_case));
        switch_expr->span = cursor_.make_span(start_token, end_token);
        return switch_expr;
    }
}
