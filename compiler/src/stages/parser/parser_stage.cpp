#include "stages/parser/parser_stage.h"
#include <iostream>
#include <memory>
#include <ostream>
#include "errors/valuascript_exception.h"
#include "stages/lexer/token.h"
#include "stages/parser/ast.h"

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

                while (!is_at_end()) {
                    if (check(TokenType::At)) {
                        program->directives.push_back(parse_directive());
                    } else if (check(TokenType::Let)) {
                        program->execution_steps.push_back(parse_assignment());
                    } else if (check(TokenType::Func)) {
                        program->function_definitions.push_back(parse_function_definition());
                    } else {
                        throw error(peek(), ErrorCode::UnexpectedToken,
                                    "Syntax Error: Invalid syntax. Expected '@', 'let', or 'func'.");
                    }
                }
                return program;
            }

        private:
            std::unique_ptr<Directive> parse_directive() {
                consume(TokenType::At, ErrorCode::UnexpectedToken, "Expected '@'.");

                const Token &name_token = consume(TokenType::Identifier, ErrorCode::MissingDirectiveName,
                                                  "Syntax Error: Expected directive name after '@'.");

                std::string directive_name = name_token.lexeme;
                std::unique_ptr<Expression> value = nullptr;

                if (match({TokenType::Assign})) {
                    if (is_at_end() || check(TokenType::At) || check(TokenType::Let) || check(TokenType::Func)) {
                        throw error(previous(), ErrorCode::MissingValueAfterEquals,
                                    "Syntax Error: Missing value after '='.");
                    }
                    value = parse_expression();
                } else if (!is_at_end() && !check(TokenType::At) && !check(TokenType::Let) && !check(TokenType::Func)) {
                    value = parse_expression();
                }

                return std::make_unique<Directive>(directive_name, std::move(value));
            }

            std::unique_ptr<Assignment> parse_assignment() {
                consume(TokenType::Let, ErrorCode::ExpectedLetToken, "Expected 'let'.");

                std::vector<std::string> targets;
                do {
                    if (is_reserved_keyword(peek().type)) {
                        throw error(peek(), ErrorCode::ReservedKeywordAsIdentifier,
                                    "Syntax Error: Cannot use a reserved keyword as a variable name.");
                    }
                    const Token &target = consume(TokenType::Identifier, ErrorCode::InvalidIdentifier,
                                                  "Syntax Error: Invalid identifier name.");
                    targets.push_back(target.lexeme);
                } while (match({TokenType::Comma}));

                consume(TokenType::Assign, ErrorCode::IncompleteAssignment,
                        "Syntax Error: Incomplete assignment. Expected '='.");

                if (is_at_end() || check(TokenType::Let) || check(TokenType::Func) || check(TokenType::At)) {
                    throw error(previous(), ErrorCode::MissingValueAfterEquals,
                                "Syntax Error: Missing value after '='.");
                }

                auto value = parse_expression();
                return std::make_unique<Assignment>(std::move(targets), std::move(value));
            }

            std::unique_ptr<TypeAnnotation> parse_type_annotation() {
                const Token &name_token = consume(TokenType::Identifier, ErrorCode::MissingTypeAnnotation,
                                                  "Syntax Error: Expected a type name.");

                std::vector<std::unique_ptr<TypeAnnotation> > generic_args;


                if (match({TokenType::Less})) {
                    do {
                        generic_args.push_back(parse_type_annotation());
                    } while (match({TokenType::Comma}));

                    consume(TokenType::Greater, ErrorCode::UnmatchedBracket,
                            "Expected '>' after generic type arguments.");
                }

                return std::make_unique<TypeAnnotation>(name_token.lexeme, std::move(generic_args));
            }

            std::unique_ptr<FunctionDefinition> parse_function_definition() {
                consume(TokenType::Func, ErrorCode::UnexpectedToken, "Expected 'func'.");
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
                if (match({TokenType::LeftParen})) {
                    // Tuple return type: -> (scalar, vector)
                    do {
                        return_types.push_back(parse_type_annotation());
                    } while (match({TokenType::Comma}));
                    consume(TokenType::RightParen, ErrorCode::UnmatchedBracket, "Expected ')' after return types.");
                } else {
                    // Single return type: -> scalar
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

                consume(TokenType::RightBrace, ErrorCode::UnmatchedBracket, "Expected '}' after function body.");

                return std::make_unique<FunctionDefinition>(name.lexeme, std::move(params), std::move(return_types),
                                                            std::move(body), std::move(docstring));
            }

            std::unique_ptr<Statement> parse_statement() {
                if (check(TokenType::Let)) {
                    return parse_assignment();
                }

                if (match({TokenType::Return})) {
                    std::vector<std::unique_ptr<Expression> > return_values;

                    do {
                        return_values.push_back(parse_expression());
                    } while (match({TokenType::Comma}));

                    return std::make_unique<ReturnStatement>(std::move(return_values));
                }

                throw error(peek(), ErrorCode::UnexpectedToken,
                            "Syntax Error: Expected statement ('let' or 'return').");
            }

            std::unique_ptr<Expression> parse_expression() {
                if (match({TokenType::If})) {
                    auto condition = parse_or_expression();
                    consume(TokenType::Then, ErrorCode::MissingThenToken, "Expected 'then' after condition.");
                    auto then_branch = parse_or_expression();
                    consume(TokenType::Else, ErrorCode::MissingElseToken, "Expected 'else' after then branch.");
                    auto else_branch = parse_expression();
                    return std::make_unique<ConditionalExpression>(std::move(condition), std::move(then_branch),
                                                                   std::move(else_branch));
                }
                return parse_or_expression();
            }

            std::unique_ptr<Expression> parse_or_expression() {
                auto expr = parse_and_expression();
                while (match({TokenType::Or})) {
                    Token op = previous();
                    auto right = parse_and_expression();
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_and_expression() {
                auto expr = parse_comparison_expression();
                while (match({TokenType::And})) {
                    Token op = previous();
                    auto right = parse_comparison_expression();
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
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
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));

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
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_multiplication_expression() {
                auto expr = parse_power_expression();
                while (match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
                    Token op = previous();
                    auto right = parse_power_expression();
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_power_expression() {
                auto expr = parse_atom();
                while (match({TokenType::Caret})) {
                    Token op = previous();
                    auto right = parse_atom();
                    expr = std::make_unique<BinaryExpression>(std::move(expr), op.type, std::move(right));
                }
                return expr;
            }

            std::unique_ptr<Expression> parse_atom() {
                // Handling Unary Minus (e.g., -1) attached to a literal/atom
                if (match({TokenType::Minus, TokenType::Plus, TokenType::Not})) {
                    Token op = previous();
                    return std::make_unique<UnaryExpression>(op.type, parse_atom());
                }

                std::unique_ptr<Expression> expr;

                if (match({TokenType::LeftParen})) {
                    if (match({TokenType::RightParen})) {
                        expr = std::make_unique<TupleLiteral>(std::vector<std::unique_ptr<Expression>>{});
                    } else {
                        expr = parse_expression();

                        if (match({TokenType::Comma})) {
                            std::vector<std::unique_ptr<Expression>> elements;
                            elements.push_back(std::move(expr));

                            do {
                                elements.push_back(parse_expression());
                            } while (match({TokenType::Comma}));

                            consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesisInTuple, "Expected ')' after tuple elements.");
                            expr = std::make_unique<TupleLiteral>(std::move(elements));
                        }
                        else {
                            consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesis, "Expected ')' after expression.");
                        }
                    }
                } else if (match({TokenType::Number})) {
                    expr = std::make_unique<NumberLiteral>(previous().lexeme);
                } else if (match({TokenType::String})) {
                    expr = std::make_unique<StringLiteral>(previous().lexeme);
                } else if (match({TokenType::True, TokenType::False})) {
                    expr = std::make_unique<BooleanLiteral>(previous().type == TokenType::True);
                } else if (match({TokenType::LeftBracket})) {
                    // Vector Literal: [1, 2, 3]
                    std::vector<std::unique_ptr<Expression> > elements;
                    if (!check(TokenType::RightBracket)) {
                        do {
                            elements.push_back(parse_expression());
                        } while (match({TokenType::Comma}));
                    }
                    consume(TokenType::RightBracket, ErrorCode::UnmatchedBracket,
                            "Expected ']' after vector elements.");
                    expr = std::make_unique<TensorLiteral>(std::move(elements));
                } else if (match({TokenType::LeftParen})) {
                    // Grouping: (1 + 2)
                    expr = parse_expression();
                    consume(TokenType::RightParen, ErrorCode::UnmatchedBracket, "Expected ')' after expression.");
                } else if (match({TokenType::Identifier})) {
                    std::string name = previous().lexeme;
                    expr = std::make_unique<IdentifierAccess>(name);
                } else {
                    throw error(peek(), ErrorCode::InvalidExpression, "Syntax Error: Expected an expression.");
                }

                // POSTFIX PARSING (Function calls and Tensor access)
                // We loop to support infinite chaining: matrix[0][1] or get_func()(arg)
                while (true) {
                    if (match({TokenType::LeftParen})) {
                        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

                        if (!check(TokenType::RightParen)) {
                            do {
                                Token arg_name = consume(TokenType::Identifier, ErrorCode::MissingArgumentName,
                                                         "Expected argument name in function call.");

                                consume(TokenType::Colon, ErrorCode::MissingColonAfterArgument,
                                        "Expected ':' after argument name.");

                                std::unique_ptr<Expression> arg_value = parse_expression();

                                arguments.emplace_back(arg_name.lexeme, std::move(arg_value));
                            } while (match({TokenType::Comma}));
                        }

                        consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesis,
                                "Expected ')' after arguments.");
                        expr = std::make_unique<FunctionCall>(std::move(expr), std::move(arguments));
                    } else if (match({TokenType::LeftBracket})) {
                        std::unique_ptr<Expression> index_expr = nullptr;

                        if (!check(TokenType::Colon) && !check(TokenType::RightBracket)) {
                            index_expr = parse_expression();
                        }

                        if (match({TokenType::Colon})) {
                            std::unique_ptr<Expression> end_expr = nullptr;

                            if (!check(TokenType::RightBracket)) {
                                end_expr = parse_expression();
                            }

                            index_expr = std::make_unique<BinaryExpression>(
                                std::move(index_expr),
                                TokenType::Colon,
                                std::move(end_expr)
                            );
                        } else if (!index_expr) {
                            // If it wasn't a slice, and we didn't get an index, they typed `tensor[]`
                            // We throw an error because empty access is meaningless.
                            throw error(previous(), ErrorCode::EmptyVectorAccess, "Expected an index or slice inside '[]'.");
                        }

                        consume(TokenType::RightBracket, ErrorCode::UnmatchedBracket,
                                "Expected ']' after vector index.");

                        expr = std::make_unique<TensorAccess>(std::move(expr), std::move(index_expr));
                    } else {
                        break;
                    }
                }

                return expr;
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

            [[nodiscard]] bool check(TokenType type) const {
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

            const Token &consume(TokenType type, ErrorCode code, const std::string &message) {
                if (check(type)) return advance();
                throw error(peek(), code, message);
            }

            [[nodiscard]] ValuaScriptException error(const Token &token, ErrorCode code,
                                                     const std::string &message) const {
                return ValuaScriptException(
                    ErrorCategory::Syntax,
                    code,
                    {token.line, token.column, file_path_},
                    message
                );
            }

            [[nodiscard]] bool is_reserved_keyword(TokenType type) const {
                switch (type) {
                    case TokenType::Let:
                    case TokenType::Func:
                    case TokenType::If:
                    case TokenType::Then:
                    case TokenType::Else:
                    case TokenType::Return:
                    case TokenType::True:
                    case TokenType::False:
                    case TokenType::And:
                    case TokenType::Or:
                    case TokenType::Not:
                        return true;
                    default:
                        return false;
                }
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
