#include "stages/frontend/parser/parser_stage.h"

#include <memory>

#include "errors/valuascript_exception.h"
#include "stages/frontend/lexer/token.h"
#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler {
    namespace {
        class Parser {
        private:
            const std::vector<Token> &tokens_;
            std::string file_path_;
            size_t current_ = 0;

        public:
            Parser(const std::vector<Token> &tokens, std::string file_path)
                : tokens_(tokens), file_path_(std::move(file_path)) {
            }

            std::unique_ptr<Program> parse_program() {
                auto program = std::make_unique<Program>();
                const Token &start_token = peek();

                while (!is_at_end()) {
                    if (check(TokenType::Import)) {
                        program->import_statements.push_back(parse_import_statement());
                    } else if (check(TokenType::Hash)) {
                        program->directives.push_back(parse_directive());
                    } else if (check(TokenType::At) || check(TokenType::Let) || check(TokenType::Var) ||
                               check(TokenType::Func) || check(TokenType::Struct) || check(TokenType::Enum)) {
                        std::vector<Modifier> modifiers = parse_modifiers();

                        if (check(TokenType::Let) || check(TokenType::Var)) {
                            program->execution_steps.push_back(parse_assignment(std::move(modifiers)));
                        } else if (check(TokenType::Func)) {
                            program->function_definitions.push_back(parse_function_definition(std::move(modifiers)));
                        } else if (check(TokenType::Struct)) {
                            program->struct_definitions.push_back(parse_struct_definition(std::move(modifiers)));
                        } else if (check(TokenType::Enum)) {
                            program->enum_definitions.push_back(parse_enum_definition(std::move(modifiers)));
                        } else {
                            throw error(peek(), ErrorCode::UnexpectedToken,
                                        "Syntax Error: Modifiers must be attached to a declaration (let, var, func, struct, enum).");
                        }
                    } else if (check(TokenType::Identifier)) {
                        program->execution_steps.push_back(parse_expression_statement());
                    } else {
                        throw error(peek(), ErrorCode::UnexpectedToken,
                                    "Syntax Error: Invalid syntax. Expected '#', 'let', 'var', 'enum', 'struct', 'func' or an identifier.");
                    }
                }

                program->span = make_span(start_token, previous());
                return program;
            }

        private:
            std::unique_ptr<ImportStatement> parse_import_statement() {
                const Token &start_token = consume(TokenType::Import, ErrorCode::ExpectedImportToken,
                                                   "Expected 'import'.");

                const Token &path = consume(TokenType::String, ErrorCode::MissingImportPathString,
                                            "Syntax Error: Expected path after 'import'.");

                std::string path_string = path.lexeme;

                auto stmt = std::make_unique<ImportStatement>(path_string);
                stmt->span = make_span(start_token, path);
                return stmt;
            }

            std::unique_ptr<Directive> parse_directive() {
                const Token &start_token = consume(TokenType::Hash, ErrorCode::UnexpectedToken, "Expected '#'.");

                const Token &name_token = consume(TokenType::Identifier, ErrorCode::MissingDirectiveName,
                                                  "Syntax Error: Expected directive name after '#'.");

                std::string directive_name = name_token.lexeme;
                std::unique_ptr<Expression> value = nullptr;

                if (match({TokenType::Assign})) {
                    if (is_at_end() || check(TokenType::Hash) || check(TokenType::Let) || check(TokenType::Func)) {
                        throw error(previous(), ErrorCode::MissingValueAfterEquals,
                                    "Syntax Error: Missing value after '='.");
                    }
                    value = parse_expression();
                } else if (!is_at_end() && !check(TokenType::Hash) && !check(TokenType::Let) && !check(TokenType::Var)
                           && !check(TokenType::Func)) {
                    value = parse_expression();
                }

                auto dir = std::make_unique<Directive>(directive_name, std::move(value));
                dir->span = make_span(start_token, previous());
                return dir;
            }

            std::vector<Modifier> parse_modifiers() {
                std::vector<Modifier> modifiers;
                while (match({TokenType::At})) {
                    const Token &start_token = previous();
                    Token name_token = consume(TokenType::Identifier, ErrorCode::ExpectedModifierName,
                                               "Syntax Error: Expected modifier name after '@'.");

                    std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

                    if (match({TokenType::LeftParen})) {
                        if (!check(TokenType::RightParen)) {
                            do {
                                Token arg_name = consume(TokenType::Identifier, ErrorCode::MissingArgumentName,
                                                         "Expected argument name in modifier.");
                                consume(TokenType::Colon, ErrorCode::MissingColonAfterArgument,
                                        "Expected ':' after argument name.");
                                arguments.emplace_back(arg_name.lexeme, parse_expression());
                            } while (match({TokenType::Comma}));
                        }
                        consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesis,
                                "Expected ')' after modifier arguments.");
                    }

                    Modifier mod;
                    mod.name = name_token.lexeme;
                    mod.arguments = std::move(arguments);
                    mod.span = make_span(start_token, previous());
                    modifiers.push_back(std::move(mod));
                }
                return modifiers;
            }

            std::unique_ptr<StructDefinition> parse_struct_definition(std::vector<Modifier> modifiers) {
                const Token &start_token = consume(TokenType::Struct, ErrorCode::ExpectedStructToken,
                                                   "Expected 'struct' in struct definition.");

                Token name_token = consume(TokenType::Identifier, ErrorCode::ExpectedStructName,
                                           "Expected struct name.");

                consume(TokenType::LeftBrace, ErrorCode::ExpectedBraceInStructDefinition,
                        "Expected '{' before struct body.");

                std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > fields;

                if (!check(TokenType::RightBrace)) {
                    do {
                        Token field_name = consume(TokenType::Identifier, ErrorCode::ExpectedStructFieldName,
                                                   "Expected field name in struct.");
                        consume(TokenType::Colon, ErrorCode::ExpectedColonAfterStructFieldName,
                                "Expected ':' after field name.");

                        std::unique_ptr<TypeAnnotation> field_type = parse_type_annotation();

                        fields.emplace_back(field_name.lexeme, std::move(field_type));
                    } while (match({TokenType::Comma}));
                }

                const Token &end_token = consume(TokenType::RightBrace, ErrorCode::ExpectedBraceInStructDefinition,
                                                 "Expected '}' after struct body.");

                auto struct_def = std::make_unique<StructDefinition>(std::move(modifiers), name_token.lexeme,
                                                                     std::move(fields));
                struct_def->span = make_span(start_token, end_token);
                return struct_def;
            }

            std::unique_ptr<EnumDefinition> parse_enum_definition(std::vector<Modifier> modifiers) {
                const Token &start_token = consume(TokenType::Enum, ErrorCode::ExpectedEnumToken,
                                                   "Expected 'enum' keyword.");

                Token name_token = consume(TokenType::Identifier, ErrorCode::ExpectedEnumName,
                                           "Expected enum name.");

                consume(TokenType::Colon, ErrorCode::ExpectedColonAfterEnumName,
                        "Expected ':' and underlying type after enum name.");
                auto underlying_type = parse_type_annotation();

                consume(TokenType::LeftBrace, ErrorCode::ExpectedLeftBrace,
                        "Expected '{' before enum body.");

                std::vector<std::pair<std::string, std::unique_ptr<Expression> > > cases;

                if (!check(TokenType::RightBrace)) {
                    do {
                        Token case_name = consume(TokenType::Identifier, ErrorCode::ExpectedEnumCaseName,
                                                  "Expected enum case identifier.");

                        std::unique_ptr<Expression> raw_value = nullptr;

                        if (match({TokenType::Assign})) {
                            raw_value = parse_expression();
                        }

                        cases.emplace_back(case_name.lexeme, std::move(raw_value));
                    } while (match({TokenType::Comma}));
                }

                const Token &end_token = consume(TokenType::RightBrace, ErrorCode::ExpectedRightBrace,
                                                 "Expected '}' after enum body.");

                auto enum_def = std::make_unique<EnumDefinition>(std::move(modifiers), name_token.lexeme,
                                                                 std::move(underlying_type),
                                                                 std::move(cases));
                enum_def->span = make_span(start_token, end_token);
                return enum_def;
            }

            std::unique_ptr<Assignment> parse_assignment(std::vector<Modifier> modifiers) {
                const Token &start_token = peek();

                bool is_mutable = false;
                if (match({TokenType::Let})) {
                    is_mutable = false;
                } else if (match({TokenType::Var})) {
                    is_mutable = true;
                }

                std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > targets;
                do {
                    if (is_reserved_keyword(peek().type)) {
                        throw error(peek(), ErrorCode::ReservedKeywordAsIdentifier,
                                    "Syntax Error: Cannot use a reserved keyword as a variable name.");
                    }
                    const Token &target = consume(TokenType::Identifier, ErrorCode::InvalidIdentifier,
                                                  "Syntax Error: Invalid identifier name.");

                    std::unique_ptr<TypeAnnotation> type_annotation = nullptr;
                    if (match({TokenType::Colon})) {
                        type_annotation = parse_type_annotation();
                    }

                    targets.emplace_back(target.lexeme, std::move(type_annotation));
                } while (match({TokenType::Comma}));

                consume(TokenType::Assign, ErrorCode::IncompleteAssignment,
                        "Syntax Error: Incomplete assignment. Expected '='.");

                if (is_at_end() || check(TokenType::Let) || check(TokenType::Var) || check(TokenType::Func) || check(
                        TokenType::At)) {
                    throw error(previous(), ErrorCode::MissingValueAfterEquals,
                                "Syntax Error: Missing value after '='.");
                }

                auto value = parse_expression();
                auto assign = std::make_unique<Assignment>(std::move(modifiers), std::move(targets), std::move(value),
                                                           is_mutable);
                assign->span = make_span(start_token, previous());
                return assign;
            }

            std::unique_ptr<TypeAnnotation> parse_type_annotation() {
                const Token &start_token = peek();

                if (match({TokenType::LeftParen})) {
                    std::vector<std::unique_ptr<TypeAnnotation> > elements;

                    if (!check(TokenType::RightParen)) {
                        do {
                            elements.push_back(parse_type_annotation());
                        } while (match({TokenType::Comma}));
                    }

                    const Token &end_token = consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesisInTuple,
                                                     "Expected ')' after tuple type elements.");

                    auto tuple_type = std::make_unique<TupleTypeAnnotation>(std::move(elements));
                    tuple_type->span = make_span(start_token, end_token);
                    return tuple_type;
                }

                Token name_token = consume(TokenType::Identifier, ErrorCode::MissingTypeAnnotation,
                                           "Expected a type name.");

                std::vector<std::unique_ptr<TypeAnnotation> > generic_args;

                if (match({TokenType::Less})) {
                    do {
                        generic_args.push_back(parse_type_annotation());
                    } while (match({TokenType::Comma}));

                    consume(TokenType::Greater, ErrorCode::UnmatchedBracket,
                            "Expected '>' after generic type arguments.");
                }

                auto type_ann = std::make_unique<TypeAnnotation>(name_token.lexeme, std::move(generic_args));
                type_ann->span = make_span(start_token, previous());
                return type_ann;
            }

            std::unique_ptr<FunctionDefinition> parse_function_definition(std::vector<Modifier> modifiers) {
                const Token &start_token = consume(TokenType::Func, ErrorCode::UnexpectedToken, "Expected 'func'.");

                const Token &name = consume(TokenType::Identifier, ErrorCode::MissingFunctionName,
                                            "Syntax Error: Expected function name.");

                consume(TokenType::LeftParen, ErrorCode::UnmatchedBracket, "Expected '(' after function name.");

                std::vector<FunctionParameter> params;

                if (!check(TokenType::RightParen)) {
                    do {
                        const Token &param_name = consume(TokenType::Identifier, ErrorCode::MissingParameterName,
                                                          "Syntax Error: Expected parameter name.");

                        consume(TokenType::Colon, ErrorCode::MissingColonAfterParameter,
                                "Expected ':' after parameter name.");

                        params.push_back({param_name.lexeme, parse_type_annotation()});
                    } while (match({TokenType::Comma}));
                }
                consume(TokenType::RightParen, ErrorCode::UnmatchedBracket, "Expected ')' after parameters.");

                consume(TokenType::Arrow, ErrorCode::MissingArrowInFunction, "Expected '->' before return type.");

                std::vector<std::unique_ptr<TypeAnnotation> > return_types;

                return_types.push_back(parse_type_annotation());

                while (match({TokenType::Comma})) {
                    return_types.push_back(parse_type_annotation());
                }

                consume(TokenType::LeftBrace, ErrorCode::UnmatchedBracket, "Expected '{' before function body.");

                std::optional<std::string> docstring = std::nullopt;
                if (check(TokenType::DocString)) {
                    docstring = advance().lexeme;
                }

                std::vector<std::unique_ptr<Statement> > body;

                while (!check(TokenType::RightBrace) && !is_at_end()) {
                    body.push_back(parse_statement());
                }

                const Token &end_token = consume(TokenType::RightBrace, ErrorCode::UnmatchedBracket,
                                                 "Expected '}' after function body.");

                auto func_def = std::make_unique<FunctionDefinition>(std::move(modifiers), name.lexeme,
                                                                     std::move(params),
                                                                     std::move(return_types),
                                                                     std::move(body), std::move(docstring));
                func_def->span = make_span(start_token, end_token);
                return func_def;
            }

            std::unique_ptr<Statement> parse_expression_statement() {
                auto expr = parse_expression();
                SourceSpan start_span = expr->span;

                if (match({TokenType::Comma})) {
                    throw error(previous(), ErrorCode::MultiReassignmentNotSupported,
                                "Syntax Error: Multiple reassignment is not supported. You must reassign variables individually.");
                }

                if (match({TokenType::Assign})) {
                    if (!is_valid_lvalue(expr.get())) {
                        throw error(previous(), ErrorCode::InvalidLeftSideExpressionInReassignment,
                                    "Syntax Error: Invalid assignment target. You can only assign to variables, properties, or indices.");
                    }

                    auto value = parse_expression();
                    SourceSpan end_span = value->span;

                    auto reassignment = std::make_unique<Reassignment>(std::move(expr), std::move(value));
                    reassignment->span = combine_spans(start_span, end_span);
                    return reassignment;
                }

                if (dynamic_cast<FunctionCall *>(expr.get()) == nullptr) {
                    throw error(previous(), ErrorCode::InvalidStandaloneStatement,
                                "Syntax Error: Invalid statement. Expected an assignment, reassignment, or function call.");
                }

                auto expr_stmt = std::make_unique<ExpressionStatement>(std::move(expr));
                expr_stmt->span = start_span;
                return expr_stmt;
            }

            std::unique_ptr<Statement> parse_statement() {
                std::vector<Modifier> modifiers;
                if (check(TokenType::At)) {
                    modifiers = parse_modifiers();
                }

                if (check(TokenType::Let) || check(TokenType::Var)) {
                    return parse_assignment(std::move(modifiers));
                }

                if (!modifiers.empty()) {
                    throw error(peek(), ErrorCode::UnexpectedToken,
                                "Syntax Error: Modifiers can only be attached to variable declarations (let, var).");
                }

                if (match({TokenType::Return})) {
                    const Token &start_token = previous();
                    std::vector<std::unique_ptr<Expression> > return_values;

                    do {
                        return_values.push_back(parse_expression());
                    } while (match({TokenType::Comma}));

                    auto ret_stmt = std::make_unique<ReturnStatement>(std::move(return_values));
                    ret_stmt->span = make_span(start_token, previous());
                    return ret_stmt;
                }

                return parse_expression_statement();
            }

            std::unique_ptr<Expression> parse_expression() {
                if (match({TokenType::If})) {
                    const Token &start_token = previous();
                    auto condition = parse_or_expression();
                    consume(TokenType::Then, ErrorCode::MissingThenToken, "Expected 'then' after condition.");
                    auto then_branch = parse_or_expression();
                    consume(TokenType::Else, ErrorCode::MissingElseToken, "Expected 'else' after then branch.");
                    auto else_branch = parse_expression();

                    auto cond_expr = std::make_unique<ConditionalExpression>(
                        std::move(condition), std::move(then_branch), std::move(else_branch));
                    cond_expr->span = make_span(start_token, previous());
                    return cond_expr;
                }
                return parse_or_expression();
            }

            std::unique_ptr<Expression> parse_or_expression() {
                auto expr = parse_and_expression();
                while (match({TokenType::Or})) {
                    Token op = previous();
                    auto right = parse_and_expression();
                    SourceSpan combined = combine_spans(expr->span, right->span);
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                    expr->span = combined;
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_and_expression() {
                auto expr = parse_comparison_expression();
                while (match({TokenType::And})) {
                    Token op = previous();
                    auto right = parse_comparison_expression();
                    SourceSpan combined = combine_spans(expr->span, right->span);
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                    expr->span = combined;
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_comparison_expression() {
                auto expr = parse_addition_expression();

                if (match({
                    TokenType::Equals, TokenType::NotEquals, TokenType::Greater,
                    TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual
                })) {
                    Token op = previous();
                    auto right = parse_addition_expression();
                    SourceSpan combined = combine_spans(expr->span, right->span);
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                    expr->span = combined;

                    if (match({
                        TokenType::Equals, TokenType::NotEquals, TokenType::Greater,
                        TokenType::GreaterEqual, TokenType::Less, TokenType::LessEqual
                    })) {
                        throw error(previous(), ErrorCode::ChainingNotAllowedForComparisonOperations,
                                    "Syntax Error: Chaining comparison operators is not allowed.");
                    }
                }

                return expr;
            }

            std::unique_ptr<Expression> parse_addition_expression() {
                auto expr = parse_multiplication_expression();
                while (match({TokenType::Plus, TokenType::Minus})) {
                    Token op = previous();
                    auto right = parse_multiplication_expression();
                    SourceSpan combined = combine_spans(expr->span, right->span);
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                    expr->span = combined;
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_multiplication_expression() {
                auto expr = parse_power_expression();
                while (match({TokenType::Star, TokenType::Slash, TokenType::Mod})) {
                    Token op = previous();
                    auto right = parse_power_expression();
                    SourceSpan combined = combine_spans(expr->span, right->span);
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                    expr->span = combined;
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_power_expression() {
                auto expr = parse_unary_expression();
                while (match({TokenType::Caret})) {
                    Token op = previous();
                    auto right = parse_unary_expression();
                    SourceSpan combined = combine_spans(expr->span, right->span);
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                    expr->span = combined;
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_unary_expression() {
                if (match({TokenType::Minus, TokenType::Plus, TokenType::Not})) {
                    Token op = previous();
                    auto right = parse_unary_expression();
                    auto unary = std::make_unique<UnaryExpression>(op.type, std::move(right));
                    unary->span = make_span(op, previous());
                    return unary;
                }

                return parse_postfix_expression();
            }

            std::unique_ptr<Expression> parse_postfix_expression() {
                auto expr = parse_primary_expression();

                while (true) {
                    SourceSpan start_span = expr->span;

                    if (match({TokenType::LeftParen})) {
                        expr = parse_function_call(std::move(expr));
                    } else if (match({TokenType::LeftBracket})) {
                        expr = parse_tensor_access(std::move(expr));
                    } else if (match({TokenType::Dot})) {
                        Token property_token = consume(TokenType::Identifier, ErrorCode::ExpectedPropertyName,
                                                       "Expected property name after '.'.");
                        expr = std::make_unique<DotAccess>(std::move(expr), property_token.lexeme);
                        expr->span = combine_spans(start_span, make_span(property_token, property_token));
                    } else {
                        break;
                    }
                }

                return expr;
            }

            std::unique_ptr<Expression> parse_primary_expression() {
                if (match({TokenType::Number})) {
                    auto node = std::make_unique<NumberLiteral>(previous().lexeme);
                    node->span = make_span(previous(), previous());
                    return node;
                }
                if (match({TokenType::PercentageLiteral})) {
                    auto node = std::make_unique<PercentageLiteral>(previous().lexeme);
                    node->span = make_span(previous(), previous());
                    return node;
                }

                if (match({TokenType::String})) {
                    auto node = std::make_unique<StringLiteral>(previous().lexeme);
                    node->span = make_span(previous(), previous());
                    return node;
                }
                if (match({TokenType::True, TokenType::False})) {
                    auto node = std::make_unique<BooleanLiteral>(previous().type == TokenType::True);
                    node->span = make_span(previous(), previous());
                    return node;
                }
                if (match({TokenType::Identifier})) {
                    auto node = std::make_unique<IdentifierAccess>(previous().lexeme);
                    node->span = make_span(previous(), previous());
                    return node;
                }

                if (match({TokenType::Switch})) return parse_switch_expression();

                if (match({TokenType::LeftParen})) return parse_tuple_or_grouping();
                if (match({TokenType::LeftBracket})) return parse_tensor_literal();
                if (match({TokenType::LeftBrace})) return parse_dict_literal();

                throw error(peek(), ErrorCode::InvalidExpression, "Syntax Error: Expected an expression.");
            }

            std::unique_ptr<Expression> parse_tuple_or_grouping() {
                const Token &start_token = previous();

                if (match({TokenType::RightParen})) {
                    auto node = std::make_unique<TupleLiteral>(std::vector<std::unique_ptr<Expression> >{});
                    node->span = make_span(start_token, previous());
                    return node;
                }

                auto expr = parse_expression();

                if (match({TokenType::Comma})) {
                    std::vector<std::unique_ptr<Expression> > elements;
                    elements.push_back(std::move(expr));

                    do {
                        elements.push_back(parse_expression());
                    } while (match({TokenType::Comma}));

                    const Token &end_token = consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesisInTuple,
                                                     "Expected ')' after tuple elements.");
                    auto node = std::make_unique<TupleLiteral>(std::move(elements));
                    node->span = make_span(start_token, end_token);
                    return node;
                }

                const Token &end_token = consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesis,
                                                 "Expected ')' after expression.");
                expr->span = make_span(start_token, end_token);
                return expr;
            }

            std::unique_ptr<Expression> parse_tensor_literal() {
                const Token &start_token = previous();
                std::vector<std::unique_ptr<Expression> > elements;
                if (!check(TokenType::RightBracket)) {
                    do {
                        elements.push_back(parse_expression());
                    } while (match({TokenType::Comma}));
                }
                const Token &end_token = consume(TokenType::RightBracket, ErrorCode::UnmatchedBracket,
                                                 "Expected ']' after vector elements.");

                auto node = std::make_unique<TensorLiteral>(std::move(elements));
                node->span = make_span(start_token, end_token);
                return node;
            }

            std::unique_ptr<Expression> parse_dict_literal() {
                const Token &start_token = previous();
                std::vector<std::pair<std::string, std::unique_ptr<Expression> > > pairs;

                if (!check(TokenType::RightBrace)) {
                    do {
                        Token key_token = consume(TokenType::Identifier, ErrorCode::ExpectedDictionaryKey,
                                                  "Expected key in dictionary.");
                        consume(TokenType::Colon, ErrorCode::ExpectedColonAfterDictionaryKey,
                                "Expected ':' after dictionary key.");
                        pairs.emplace_back(key_token.lexeme, parse_expression());
                    } while (match({TokenType::Comma}));
                }

                const Token &end_token = consume(TokenType::RightBrace, ErrorCode::UnmatchedBraceInDictionaryLiteral,
                                                 "Expected '}' after dictionary literal.");

                auto node = std::make_unique<DictLiteral>(std::move(pairs));
                node->span = make_span(start_token, end_token);
                return node;
            }

            std::unique_ptr<Expression> parse_function_call(std::unique_ptr<Expression> target) {
                SourceSpan target_span = target->span;
                std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

                if (!check(TokenType::RightParen)) {
                    do {
                        Token arg_name = consume(TokenType::Identifier, ErrorCode::MissingArgumentName,
                                                 "Expected argument name in function call.");
                        consume(TokenType::Colon, ErrorCode::MissingColonAfterArgument,
                                "Expected ':' after argument name.");
                        arguments.emplace_back(arg_name.lexeme, parse_expression());
                    } while (match({TokenType::Comma}));
                }

                const Token &end_token = consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesis,
                                                 "Expected ')' after arguments.");

                auto func_call = std::make_unique<FunctionCall>(std::move(target), std::move(arguments));
                func_call->span = combine_spans(target_span, make_span(end_token, end_token));
                return func_call;
            }

            std::unique_ptr<Expression> parse_tensor_access(std::unique_ptr<Expression> target) {
                SourceSpan target_span = target->span;
                std::unique_ptr<Expression> index_expr = nullptr;

                if (!check(TokenType::Colon) && !check(TokenType::RightBracket)) {
                    index_expr = parse_expression();
                }

                if (match({TokenType::Colon})) {
                    std::unique_ptr<Expression> end_expr = nullptr;
                    if (!check(TokenType::RightBracket)) {
                        end_expr = parse_expression();
                    }

                    SourceSpan colon_span = index_expr ? index_expr->span : target_span;
                    SourceSpan slice_end_span = end_expr ? end_expr->span : make_span(previous(), previous());

                    index_expr = std::make_unique<BinaryExpression>(std::move(index_expr), TokenType::Colon,
                                                                    std::move(end_expr));
                    index_expr->span = combine_spans(colon_span, slice_end_span);
                } else if (!index_expr) {
                    throw error(previous(), ErrorCode::EmptyBracketAccess, "Expected an index or slice inside '[]'.");
                }

                const Token &end_token = consume(TokenType::RightBracket, ErrorCode::UnmatchedBracket,
                                                 "Expected ']' after vector index.");

                auto bracket_access = std::make_unique<BracketAccess>(std::move(target), std::move(index_expr));
                bracket_access->span = combine_spans(target_span, make_span(end_token, end_token));
                return bracket_access;
            }

            std::unique_ptr<Expression> parse_switch_expression() {
                const Token &start_token = previous();
                consume(TokenType::LeftParen, ErrorCode::ExpectedLeftParen,
                        "Expected '(' after 'switch'.");
                auto target = parse_expression();
                consume(TokenType::RightParen, ErrorCode::ExpectedRightParen,
                        "Expected ')' after switch target.");

                consume(TokenType::LeftBrace, ErrorCode::ExpectedLeftBrace,
                        "Expected '{' before switch body.");

                std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > cases;
                std::unique_ptr<Expression> default_case = nullptr;

                while (!check(TokenType::RightBrace) && !is_at_end()) {
                    if (match({TokenType::Case})) {
                        std::vector<std::string> case_identifiers;

                        do {
                            Token id_token = consume(TokenType::Identifier, ErrorCode::ExpectedEnumCaseName,
                                                     "Expected enum case identifier after 'case'.");
                            case_identifiers.push_back(id_token.lexeme);
                        } while (match({TokenType::Comma}));

                        consume(TokenType::Arrow, ErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier,
                                "Expected '->' after case identifiers.");
                        auto result_expr = parse_expression();

                        cases.emplace_back(std::move(case_identifiers), std::move(result_expr));
                    } else if (match({TokenType::Default})) {
                        if (default_case != nullptr) {
                            throw error(peek(), ErrorCode::MultipleDefaultCasesInSwitch,
                                        "Syntax Error: A switch expression can only have one 'default' case.");
                        }

                        consume(TokenType::Arrow, ErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier,
                                "Expected '->' after 'default'.");
                        default_case = parse_expression();
                    } else {
                        throw error(peek(), ErrorCode::CaseOrDefaultMissingInSwitch,
                                    "Syntax Error: Expected 'case' or 'default' inside switch body.");
                    }
                }

                const Token &end_token = consume(TokenType::RightBrace, ErrorCode::ExpectedRightBrace,
                                                 "Expected '}' after switch body.");

                auto switch_expr = std::make_unique<SwitchExpression>(
                    std::move(target),
                    std::move(cases),
                    std::move(default_case)
                );
                switch_expr->span = make_span(start_token, end_token);
                return switch_expr;
            }

            static bool is_valid_lvalue(const Expression *expr) {
                if (dynamic_cast<const IdentifierAccess *>(expr) != nullptr) return true;
                if (dynamic_cast<const DotAccess *>(expr) != nullptr) return true;
                if (dynamic_cast<const BracketAccess *>(expr) != nullptr) return true;

                return false;
            }

            [[nodiscard]] const Token &peek() const {
                return tokens_[current_];
            }

            [[nodiscard]] const Token &previous() const {
                return tokens_[current_ - 1];
            }

            [[nodiscard]] bool is_at_end() const {
                return peek().type == TokenType::EndOfFile;
            }

            [[nodiscard]] bool check(const TokenType type) const {
                if (is_at_end()) return false;
                return peek().type == type;
            }

            const Token &advance() {
                if (!is_at_end()) current_++;
                return previous();
            }

            bool match(std::initializer_list<TokenType> types) {
                for (const TokenType type: types) {
                    if (check(type)) {
                        advance();
                        return true;
                    }
                }
                return false;
            }

            const Token &consume(const TokenType type, const ErrorCode code, const std::string &message) {
                if (check(type)) return advance();
                throw error(peek(), code, message);
            }

            [[nodiscard]] SourceSpan make_span(const Token &start_token, const Token &end_token) const {
                size_t end_col = end_token.column;
                if (end_token.type != TokenType::EndOfFile) {
                    end_col += end_token.lexeme.length();
                }
                return {start_token.line, start_token.column, end_token.line, end_col, file_path_};
            }

            [[nodiscard]] SourceSpan combine_spans(const SourceSpan &start, const SourceSpan &end) const {
                return {start.line_start, start.column_start, end.line_end, end.column_end, file_path_};
            }

            [[nodiscard]] ValuaScriptException error(const Token &token, const ErrorCode code,
                                                     const std::string &message) const {
                return ValuaScriptException(
                    ErrorCategory::Syntax,
                    code,
                    {token.line, token.column, file_path_},
                    message
                );
            }
        };
    }

    ParserStage::ParserStage()
        : CompilerStage(
            "ParserStage",
            CompilerStageArtifactCode::Ast,
            {CompilerStageArtifactCode::TokenStream, CompilerStageArtifactCode::FilePath}
        ) {
    }

    CompilerStageArtifact ParserStage::run(const std::vector<CompilerStageArtifact> &artifacts) {
        const auto tokens = extract_artifact_data<std::vector<Token> >(
            artifacts, CompilerStageArtifactCode::TokenStream);
        const auto file_path = extract_artifact_data<std::string>(artifacts, CompilerStageArtifactCode::FilePath);

        Parser parser(tokens, file_path);

        std::shared_ptr<Program> ast = parser.parse_program();

        return {CompilerStageArtifactCode::Ast, ast};
    }
}
