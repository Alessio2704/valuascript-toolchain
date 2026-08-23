#pragma once

#include <tuple>
#include "ast/core/ast_core.h"
#include "ast/core/ast_decl.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_expr.h"
#include "ast/categories/ast_category_kind.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    using AllInnerNodeTypes = std::tuple<
        FunctionParameter,
        StructField,
        EnumCase,
        SwitchCase,
        AssignmentTarget,
        Modifier,
        CallArgument,
        DictItem,
        Comment
    >;

    template <typename T>
    concept IsInnerNode = valuascript::shared::tuple_contains_type_v<T, AllInnerNodeTypes>;

    using InnerKind = CategoryKind<AllInnerNodeTypes>;
}
