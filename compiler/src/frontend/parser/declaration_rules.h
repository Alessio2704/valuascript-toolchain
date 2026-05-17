#pragma once
#include <vector>
#include <memory>
#include "token/token.h"
#include "ast.h"

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

        ValuascriptErrorCode missing_name_err = ValuascriptErrorCode::ExpectedIdentifier;
        ValuascriptErrorCode missing_type_colon_err = ValuascriptErrorCode::ExpectedColon;
        ValuascriptErrorCode missing_value_separator_err = ValuascriptErrorCode::MissingOperator;
        ValuascriptErrorCode missing_value_err = ValuascriptErrorCode::InvalidExpression;
        ValuascriptErrorCode unexpected_modifier_err = ValuascriptErrorCode::ModifiersAttachedToInvalidDeclaration;
    };

    struct GenericParameter
    {
        std::vector<Modifier> modifiers;
        Token name{TokenType::Identifier, "<error>", 0, 0};
        std::unique_ptr<TypeAnnotation> type;
        std::unique_ptr<Expression> value;
        bool has_value_separator = false;
        SourceSpan span;
    };
}
