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
                if (!modifiers.empty()) {
                    cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                }

                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::UnexpectedTopLevelToken);
        }
    }

    std::unique_ptr<ImportStatement> Parser::parse_import_statement() {
        const Token &start_token = cursor_.consume(TokenType::Import, ValuascriptErrorCode::ExpectedImportToken);
        const Token &path = cursor_.consume(TokenType::String, ValuascriptErrorCode::MissingImportPathString);
        auto stmt = std::make_unique<ImportStatement>(path.lexeme);
        stmt->span = cursor_.make_span(start_token, path);
        return stmt;
    }

    std::unique_ptr<Directive> Parser::parse_directive() {
        const Token &start_token = cursor_.consume(TokenType::Hash, ValuascriptErrorCode::ExpectedHashToken);
        const Token &name_token = consume_identifier(ValuascriptErrorCode::MissingDirectiveName);
        std::string directive_name = name_token.lexeme;

        std::unique_ptr<Expression> value = nullptr;

        if (cursor_.match({TokenType::Assign})) {
            if (cursor_.is_at_end() || is_top_level_token(cursor_.peek().type)) {
                cursor_.report_error(cursor_.previous(), ValuascriptErrorCode::MissingValueAfterEquals);
            }
            value = parse_expression();
        } else if (!cursor_.is_at_end() && !is_top_level_token(cursor_.peek().type)) {
            value = parse_expression();
        }

        verify_statement_end();

        auto dir = std::make_unique<Directive>(directive_name, std::move(value));
        dir->span = cursor_.make_span(start_token, cursor_.previous());
        return dir;
    }

    std::vector<Modifier> Parser::parse_modifiers() {
        std::vector<Modifier> modifiers;

        while (cursor_.match({TokenType::At})) {
            const Token &start_token = cursor_.previous();
            Token name_token = consume_identifier(ValuascriptErrorCode::ExpectedModifierName);
            std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

            if (cursor_.match({TokenType::LeftParen})) {
                arguments = parse_key_value_list(
                    TokenType::RightParen,
                    ValuascriptErrorCode::MissingArgumentNameInModifier,
                    ValuascriptErrorCode::MissingColonAfterArgument,
                    ValuascriptErrorCode::MissingCommaSeparatorForArgumentsInModifier,
                    ValuascriptErrorCode::TrailingCommaInModifier);
                cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::UnmatchedParenthesisAfterModifierArgs);
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
        const Token &start_token = cursor_.consume(TokenType::Struct, ValuascriptErrorCode::ExpectedStructToken);

        Token name_token = consume_identifier(ValuascriptErrorCode::ExpectedStructName);

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedBraceInStructDefinition);

        auto fields = parse_comma_separated_list<std::pair<std::string, std::unique_ptr<TypeAnnotation> > >(
            TokenType::RightBrace,
            std::nullopt,
            ValuascriptErrorCode::ExpectedCommaSeparatorInStruct,
            {},
            [&]() {
                Token field_name = consume_identifier(ValuascriptErrorCode::ExpectedStructFieldName);
                cursor_.consume(TokenType::Colon, ValuascriptErrorCode::ExpectedColonAfterStructFieldName);
                return std::make_pair(field_name.lexeme, parse_type_annotation());
            }
        );

        const Token &end_token = cursor_.consume(TokenType::RightBrace,
                                                 ValuascriptErrorCode::ExpectedRightBraceAfterStructBody);
        auto struct_def = std::make_unique<
            StructDefinition>(std::move(modifiers), name_token.lexeme, std::move(fields));

        struct_def->span = cursor_.make_span(start_token, end_token);

        return struct_def;
    }

    std::unique_ptr<EnumDefinition> Parser::parse_enum_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Enum, ValuascriptErrorCode::ExpectedEnumToken);

        Token name_token = consume_identifier(ValuascriptErrorCode::ExpectedEnumName);

        cursor_.consume(TokenType::Colon, ValuascriptErrorCode::ExpectedColonAfterEnumName);

        auto underlying_type = parse_type_annotation();

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeEnumBody);

        auto cases = parse_comma_separated_list<std::pair<std::string, std::unique_ptr<Expression> > >(
            TokenType::RightBrace,
            std::nullopt,
            ValuascriptErrorCode::ExpectedCommaSeparatorInEnum,
            {},
            [&]() {
                Token case_name = consume_identifier(ValuascriptErrorCode::ExpectedEnumCaseName);
                std::unique_ptr<Expression> raw_value = nullptr;
                if (cursor_.match({TokenType::Assign})) {
                    raw_value = parse_expression();

                    if (is_expression_start(cursor_.peek().type) && cursor_.peek(1).type != TokenType::Assign) {
                        cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingOperator);
                    }
                }
                return std::make_pair(case_name.lexeme, std::move(raw_value));
            }
        );

        const Token &end_token = cursor_.consume(TokenType::RightBrace,
                                                 ValuascriptErrorCode::ExpectedRightBraceAfterEnumBody);

        auto enum_def = std::make_unique<EnumDefinition>(std::move(modifiers), name_token.lexeme,
                                                         std::move(underlying_type), std::move(cases));

        enum_def->span = cursor_.make_span(start_token, end_token);

        return enum_def;
    }

    std::unique_ptr<FunctionDefinition> Parser::parse_function_definition(std::vector<Modifier> modifiers) {
        const Token &start_token = cursor_.consume(TokenType::Func, ValuascriptErrorCode::ExpectedFuncToken);

        const Token &name = consume_identifier(ValuascriptErrorCode::MissingFunctionName);

        cursor_.consume(TokenType::LeftParen, ValuascriptErrorCode::ExpectedLeftParenAfterFunctionName);

        auto params = parse_comma_separated_list<FunctionParameter>(
            TokenType::RightParen,
            ValuascriptErrorCode::TrailingCommaInFunctionCall,
            ValuascriptErrorCode::ExpectedCommaSeparatorInParameterList,
            {},
            [&]() {
                auto mods = parse_modifiers();
                if (!mods.empty()) {
                    cursor_.report_error_no_panic(cursor_.previous(),
                                                  ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration);
                }
                const Token &param_name = consume_identifier(ValuascriptErrorCode::MissingParameterName);
                cursor_.consume(TokenType::Colon, ValuascriptErrorCode::MissingColonAfterParameter);
                return FunctionParameter{param_name.lexeme, parse_type_annotation()};
            }
        );

        cursor_.consume(TokenType::RightParen, ValuascriptErrorCode::ExpectedRightParenAfterParameters);
        cursor_.consume(TokenType::Arrow, ValuascriptErrorCode::MissingArrowInFunction);

        auto return_types = parse_comma_separated_list<std::unique_ptr<TypeAnnotation> >(
            TokenType::LeftBrace,
            ValuascriptErrorCode::TrailingComma,
            ValuascriptErrorCode::ExpectedCommaSeparatorInReturnTypeList,
            {},
            [&]() {
                return parse_type_annotation();
            }
        );

        if (return_types.empty()) {
            cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::MissingTypeAnnotationAfterArrow);
        }

        cursor_.consume(TokenType::LeftBrace, ValuascriptErrorCode::ExpectedLeftBraceBeforeFunctionBody);

        std::optional<std::string> docstring = std::nullopt;
        if (cursor_.check(TokenType::DocString)) {
            docstring = cursor_.advance().lexeme;
        }

        std::vector<std::unique_ptr<Statement> > body;
        while (!cursor_.check(TokenType::RightBrace) && !cursor_.is_at_end()) {
            body.push_back(parse_function_body_statements());
        }

        const Token &end_token = cursor_.consume(TokenType::RightBrace,
                                                 ValuascriptErrorCode::ExpectedRightBraceAfterFunctionBody);

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
                ValuascriptErrorCode::SingleElementTuplesNotAllowed,
                ValuascriptErrorCode::ExpectedCommaSeparatorInTupleType,
                {},
                [&]() {
                    return parse_type_annotation();
                }
            );

            const Token &end_token = cursor_.consume(TokenType::RightParen,
                                                     ValuascriptErrorCode::UnmatchedParenthesisInTuple);

            auto tuple_type_annotation = std::make_unique<TupleTypeAnnotation>(std::move(elements));
            tuple_type_annotation->span = cursor_.make_span(start_token, end_token);
            return tuple_type_annotation;
        }

        Token name_token = consume_identifier(ValuascriptErrorCode::MissingTypeAnnotation);
        std::vector<std::unique_ptr<TypeAnnotation> > generic_args;

        if (cursor_.match({TokenType::Less})) {
            generic_args = parse_comma_separated_list<std::unique_ptr<TypeAnnotation> >(
                TokenType::Greater,
                ValuascriptErrorCode::TrailingCommaInGenericArgument,
                ValuascriptErrorCode::ExpectedCommaSeparatorInGenericArgs,
                {},
                [&]() {
                    return parse_type_annotation();
                }
            );
            if (generic_args.empty()) {
                cursor_.report_error(cursor_.peek(), ValuascriptErrorCode::EmptyGenericTypeAnnotation);
            }

            cursor_.consume(TokenType::Greater, ValuascriptErrorCode::UnmatchedBracketAfterGenericArgs);
        }

        auto type_ann = std::make_unique<TypeAnnotation>(name_token.lexeme, std::move(generic_args));
        type_ann->span = cursor_.make_span(start_token, cursor_.previous());
        return type_ann;
    }
}
