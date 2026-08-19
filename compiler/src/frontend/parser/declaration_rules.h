#pragma once
#include <vector>
#include <memory>
#include "token/token.h"
#include "ast/ast.h"

namespace valuascript::compiler
{
    struct ParameterRuleSpec
    {
        bool allow_modifiers = false;
        bool allow_type = false;
        bool require_type = false;
        bool allow_value = false;
        bool require_value = false;
        TokenType value_separator = TokenType::Assign;

        ParserErrorCode missing_name_err = ParserErrorCode::ExpectedIdentifier;
        ParserErrorCode missing_type_colon_err = ParserErrorCode::ExpectedColon;
        ParserErrorCode missing_value_separator_err = ParserErrorCode::MissingOperator;
        ParserErrorCode missing_value_err = ParserErrorCode::InvalidExpression;
        ParserErrorCode unexpected_modifier_err = ParserErrorCode::ModifiersAttachedToInvalidDeclaration;
    };

    struct GenericParameter
    {
        std::vector<Modifier> modifiers;
        Token name{.type = TokenType::Identifier, .lexeme = "<error>", .line = 0, .column = 0};
        TypeAnnPtr type;
        ExprPtr value;
        bool has_value_separator = false;
        SourceSpan span;
    };
}
