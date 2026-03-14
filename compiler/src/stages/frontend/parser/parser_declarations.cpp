#include "stages/frontend/parser/parser.h"

namespace valuascript::compiler {
    void Parser::parse_top_level_declaration(Program *program) {
        std::vector<Modifier> modifiers = parse_modifiers();

        switch (cursor_.peek().type) {
            case TokenType::Let:
            case TokenType::Var:
                program->execution_steps.push_back(parse_assignment(std::move(modifiers)));
                verify_statement_end();
                break;
            case TokenType::Func:
                program->function_definitions.push_back(parse_function_definition(std::move(modifiers)));
                break;
            case TokenType::Struct:
                program->struct_definitions.push_back(parse_struct_definition(std::move(modifiers)));
                break;
            case TokenType::Enum:
                program->enum_definitions.push_back(parse_enum_definition(std::move(modifiers)));
                break;
            default:
                cursor_.report_error(cursor_.peek(), ErrorCode::UnexpectedToken,
                                     "Syntax Error: Modifiers must be attached to a declaration (let, var, func, struct, enum).");
        }
    }

    std::unique_ptr<ImportStatement> Parser::parse_import_statement() {
        const Token &start_token = cursor_.consume(TokenType::Import, ErrorCode::ExpectedImportToken,
                                                   "Expected 'import'.");
        const Token &path = cursor_.consume(TokenType::String, ErrorCode::MissingImportPathString,
                                            "Syntax Error: Expected path after 'import'.");

        auto stmt = std::make_unique<ImportStatement>(path.lexeme);
        stmt->span = cursor_.make_span(start_token, path);
        return stmt;
    }

    std::unique_ptr<Directive> Parser::parse_directive() {
        const Token &start_token = cursor_.consume(TokenType::Hash, ErrorCode::UnexpectedToken, "Expected '#'.");
        const Token &name_token = cursor_.consume(TokenType::Identifier, ErrorCode::MissingDirectiveName,
                                                  "Syntax Error: Expected directive name after '#'.");

        std::string directive_name = name_token.lexeme;
        std::unique_ptr<Expression> value = nullptr;

        if (cursor_.match({TokenType::Assign})) {
            if (cursor_.is_at_end() || cursor_.check(TokenType::Hash) || cursor_.check(TokenType::Let) || cursor_.check(
                    TokenType::Func)) {
                cursor_.report_error(cursor_.previous(), ErrorCode::MissingValueAfterEquals,
                                     "Syntax Error: Missing value after '='.");
            }
            value = parse_expression();
        } else if (!cursor_.is_at_end() && !cursor_.check(TokenType::Hash) && !cursor_.check(TokenType::Let) && !cursor_
                   .check(TokenType::Var) && !cursor_.check(TokenType::Func)) {
            value = parse_expression();
        }
        verify_statement_end();;

        auto dir = std::make_unique<Directive>(directive_name, std::move(value));
        dir->span = cursor_.make_span(start_token, cursor_.previous());
        return dir;
    }

    std::vector<Modifier> Parser::parse_modifiers() {
        std::vector<Modifier> modifiers;
        while (cursor_.match({TokenType::At})) {
            const Token &start_token = cursor_.previous();
            Token name_token = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedModifierName,
                                               "Syntax Error: Expected modifier name after '@'.");

            std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

            if (cursor_.match({TokenType::LeftParen})) {
                arguments = parse_key_value_list(
                    TokenType::RightParen,
                    ErrorCode::MissingArgumentName, "Expected argument name in modifier.",
                    ErrorCode::MissingColonAfterArgument, "Expected ':' after argument name.",
                    ErrorCode::MissingCommaSeparatorForArgumentsInModifier);

                cursor_.consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesis,
                                "Expected ')' after modifier arguments.");
            }

            Modifier mod;
            mod.name = name_token.lexeme;
            mod.arguments = std::move(arguments);
            mod.span = cursor_.make_span(start_token, cursor_.previous());
            modifiers.push_back(std::move(mod));
        }
        return modifiers;
    }

    std::unique_ptr<StructDefinition> Parser::parse_struct_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Struct, ErrorCode::ExpectedStructToken,
                                                   "Expected 'struct'.");
        Token name_token = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedStructName,
                                           "Expected struct name.");
        cursor_.consume(TokenType::LeftBrace, ErrorCode::ExpectedBraceInStructDefinition,
                        "Expected '{' before struct body.");

        auto fields = parse_comma_separated_list<std::pair<std::string, std::unique_ptr<TypeAnnotation> > >(
            TokenType::RightBrace,
            ErrorCode::ExpectedCommaSeparatorInStruct,
            "Syntax Error: Missing comma ',' between struct fields.",
            [&]() {
                Token field_name = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedStructFieldName,
                                                   "Expected field name.");
                cursor_.consume(TokenType::Colon, ErrorCode::ExpectedColonAfterStructFieldName, "Expected ':'.");
                return std::make_pair(field_name.lexeme, parse_type_annotation());
            }
        );

        const Token &end_token = cursor_.consume(TokenType::RightBrace, ErrorCode::ExpectedBraceInStructDefinition,
                                                 "Expected '}'.");
        auto struct_def = std::make_unique<
            StructDefinition>(std::move(modifiers), name_token.lexeme, std::move(fields));
        struct_def->span = cursor_.make_span(start_token, end_token);
        return struct_def;
    }

    std::unique_ptr<EnumDefinition> Parser::parse_enum_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Enum, ErrorCode::ExpectedEnumToken,
                                                   "Expected 'enum' keyword.");
        Token name_token = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedEnumName, "Expected enum name.");

        cursor_.consume(TokenType::Colon, ErrorCode::ExpectedColonAfterEnumName,
                        "Expected ':' and underlying type after enum name.");
        auto underlying_type = parse_type_annotation();

        cursor_.consume(TokenType::LeftBrace, ErrorCode::ExpectedLeftBrace, "Expected '{' before enum body.");

        auto cases = parse_comma_separated_list<std::pair<std::string, std::unique_ptr<Expression> > >(
            TokenType::RightBrace,
            ErrorCode::ExpectedCommaSeparatorInEnum,
            "Syntax Error: Missing comma ',' between enum cases.",
            [&]() {
                Token case_name = cursor_.consume(TokenType::Identifier, ErrorCode::ExpectedEnumCaseName,
                                                  "Expected enum case identifier.");
                std::unique_ptr<Expression> raw_value = nullptr;
                if (cursor_.match({TokenType::Assign})) {
                    raw_value = parse_expression();
                }
                return std::make_pair(case_name.lexeme, std::move(raw_value));
            }
        );

        const Token &end_token = cursor_.consume(TokenType::RightBrace, ErrorCode::ExpectedRightBrace,
                                                 "Expected '}' after enum body.");
        auto enum_def = std::make_unique<EnumDefinition>(std::move(modifiers), name_token.lexeme,
                                                         std::move(underlying_type), std::move(cases));
        enum_def->span = cursor_.make_span(start_token, end_token);
        return enum_def;
    }

    std::unique_ptr<FunctionDefinition> Parser::parse_function_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Func, ErrorCode::UnexpectedToken, "Expected 'func'.");
        const Token &name = cursor_.consume(TokenType::Identifier, ErrorCode::MissingFunctionName,
                                            "Syntax Error: Expected function name.");

        cursor_.consume(TokenType::LeftParen, ErrorCode::ExpectedLeftParen, "Expected '(' after function name.");

        auto params = parse_comma_separated_list<FunctionParameter>(
            TokenType::RightParen,
            ErrorCode::ExpectedCommaSeparatorInParameterList,
            "Syntax Error: Missing comma ',' between parameters.",
            [&]() {
                const Token &param_name = cursor_.consume(TokenType::Identifier, ErrorCode::MissingParameterName,
                                                          "Syntax Error: Expected parameter name.");
                cursor_.consume(TokenType::Colon, ErrorCode::MissingColonAfterParameter,
                                "Expected ':' after parameter name.");
                return FunctionParameter{param_name.lexeme, parse_type_annotation()};
            }
        );

        cursor_.consume(TokenType::RightParen, ErrorCode::ExpectedRightParen, "Expected ')' after parameters.");
        cursor_.consume(TokenType::Arrow, ErrorCode::MissingArrowInFunction, "Expected '->' before return type.");

        auto return_types = parse_comma_separated_list<std::unique_ptr<TypeAnnotation> >(
            TokenType::LeftBrace,
            ErrorCode::ExpectedCommaSeparatorInReturnTypeList,
            "Syntax Error: Missing comma ',' between return types.",
            [&]() {
                return parse_type_annotation();
            }
        );

        if (return_types.empty()) {
            cursor_.report_error(cursor_.peek(), ErrorCode::MissingTypeAnnotation,
                                 "Syntax Error: Expected at least one return type after '->'.");
        }

        cursor_.consume(TokenType::LeftBrace, ErrorCode::ExpectedLeftBrace, "Expected '{' before function body.");

        std::optional<std::string> docstring = std::nullopt;
        if (cursor_.check(TokenType::DocString)) {
            docstring = cursor_.advance().lexeme;
        }

        std::vector<std::unique_ptr<Statement> > body;
        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end()) {
            body.push_back(parse_statement());
        }

        const Token &end_token = cursor_.consume(TokenType::RightBrace, ErrorCode::ExpectedRightBrace,
                                                 "Expected '}' after function body.");
        auto func_def = std::make_unique<FunctionDefinition>(std::move(modifiers), name.lexeme, std::move(params),
                                                             std::move(return_types), std::move(body),
                                                             std::move(docstring));
        func_def->span = cursor_.make_span(start_token, end_token);
        return func_def;
    }

    std::unique_ptr<TypeAnnotation> Parser::parse_type_annotation() {
        const Token &start_token = cursor_.peek();

        if (cursor_.match({TokenType::LeftParen})) {
            auto elements = parse_comma_separated_list<std::unique_ptr<TypeAnnotation> >(
                TokenType::RightParen,
                ErrorCode::ExpectedCommaSeparatorInTupleType,
                "Syntax Error: Missing comma ',' between tuple type elements.",
                [&]() {
                    return parse_type_annotation();
                }
            );

            const Token &end_token = cursor_.consume(TokenType::RightParen, ErrorCode::UnmatchedParenthesisInTuple,
                                                     "Expected ')' after tuple type elements.");
            auto tuple_type = std::make_unique<TupleTypeAnnotation>(std::move(elements));
            tuple_type->span = cursor_.make_span(start_token, end_token);
            return tuple_type;
        }

        Token name_token = cursor_.consume(TokenType::Identifier, ErrorCode::MissingTypeAnnotation,
                                           "Expected a type name.");

        std::vector<std::unique_ptr<TypeAnnotation> > generic_args;

        if (cursor_.match({TokenType::Less})) {
            generic_args = parse_comma_separated_list<std::unique_ptr<TypeAnnotation> >(
                TokenType::Greater,
                ErrorCode::ExpectedCommaSeparatorInGenericArgs,
                "Syntax Error: Missing comma ',' between generic type arguments.",
                [&]() {
                    return parse_type_annotation();
                }
            );

            if (generic_args.empty()) {
                cursor_.report_error(cursor_.peek(), ErrorCode::EmptyGenericTypeAnnotation,
                                     "Syntax Error: Expected at least one generic type argument inside '<>'.");
            }

            cursor_.consume(TokenType::Greater, ErrorCode::UnmatchedBracket,
                            "Expected '>' after generic type arguments.");
        }

        auto type_ann = std::make_unique<TypeAnnotation>(name_token.lexeme, std::move(generic_args));
        type_ann->span = cursor_.make_span(start_token, cursor_.previous());
        return type_ann;
    }
}
