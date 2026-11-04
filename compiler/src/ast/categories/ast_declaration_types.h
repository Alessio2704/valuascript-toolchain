#pragma once

#include <tuple>
#include "ast/core/ast_decl.h"
#include "ast/categories/ast_category_kind.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler
{
    using AllDeclarationNodeTypes = std::tuple<
        ImportStatement,
        Directive,
        FunctionDefinition,
        StructDefinition,
        EnumDefinition,
        TypeAliasDefinition,
        ExtensionDefinition
    >;

    template <typename T>
    concept IsDeclarationNode = valuascript::shared::tuple_contains_type_v<T, AllDeclarationNodeTypes>;

    using DeclarationKind = CategoryKind<AllDeclarationNodeTypes>;
}
