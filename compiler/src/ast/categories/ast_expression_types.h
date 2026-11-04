#pragma once

#include <tuple>
#include "ast/core/ast_expr.h"
#include "ast/categories/ast_category_kind.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    using AllExpressionNodeTypes = std::tuple<
        NumberLiteral,
        PercentageLiteral,
        StringLiteral,
        BooleanLiteral,
        IdentifierAccess,
        SelfExpression,
        BinaryExpression,
        UnaryExpression,
        GroupingExpression,
        ConditionalExpression,
        FunctionCall,
        DictLiteral,
        TensorLiteral,
        TupleLiteral,
        BracketAccess,
        DotAccess,
        SwitchExpression
    >;

    template <typename T>
    concept IsExpressionNode = valuascript::shared::tuple_contains_type_v<T, AllExpressionNodeTypes>;

    using ExpressionKind = CategoryKind<AllExpressionNodeTypes>;
}
