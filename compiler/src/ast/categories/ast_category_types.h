#pragma once

#include <tuple>
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_type.h"

namespace valuascript::compiler
{
    using AstCategoryTypes = std::tuple<
        Statement,
        Expression,
        TypeAnnotation
    >;
}
