#pragma once

#include <tuple>
#include "ast/core/ast_stmt.h"
#include "ast/categories/ast_category_kind.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    using AllStatementNodeTypes = std::tuple<
        Assignment,
        Reassignment,
        ExpressionStatement,
        ReturnStatement
    >;

    template <typename T>
    concept IsStatementNode = valuascript::shared::tuple_contains_type_v<T, AllStatementNodeTypes>;

    using StatementKind = CategoryKind<AllStatementNodeTypes>;
}
