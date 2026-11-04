#pragma once

#include <tuple>
#include "ast/core/ast_expr.h"
#include "ast/categories/ast_category_kind.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    using AllReassignmentTargetNodeTypes = std::tuple<
        IdentifierAccess,
        DotAccess,
        BracketAccess
    >;

    template <typename T>
    concept IsReassignmentTarget = valuascript::shared::tuple_contains_type_v<T, AllReassignmentTargetNodeTypes>;

    using ReassignmentTargetKind = CategoryKind<AllReassignmentTargetNodeTypes>;
}
