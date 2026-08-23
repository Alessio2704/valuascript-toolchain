#pragma once

#include <tuple>
#include "ast/core/ast_type.h"
#include "ast/categories/ast_category_kind.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    using AllTypeAnnotationNodeTypes = std::tuple<
        TypeAnnotation,
        TupleTypeAnnotation
    >;

    template <typename T>
    concept IsTypeAnnotationNode = valuascript::shared::tuple_contains_type_v<T, AllTypeAnnotationNodeTypes>;

    using TypeAnnotationKind = CategoryKind<AllTypeAnnotationNodeTypes>;
}
