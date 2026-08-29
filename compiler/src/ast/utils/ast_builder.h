#pragma once

#include <memory>
#include <utility>
#include <type_traits>

#include "token/source_span.h"
#include "utils/traits/tuple_traits.h"
#include "ast/core/ast_core.h"
#include "ast/categories/ast_inner_types.h"

namespace valuascript::compiler
{
    struct AstBuilder
    {
        template <typename T, typename... Args>
        static auto build(Args&&... args)
        {
            if constexpr (valuascript::shared::tuple_contains_type_v<T, AllInnerNodeTypes>)
            {
                return T(std::forward<Args>(args)...);
            }
            else
            {
                return std::make_unique<T>(std::forward<Args>(args)...);
            }
        }

        template <typename T, typename... Args>
        static auto build_with_span(const SourceSpan& span, Args&&... args)
        {
            if constexpr (valuascript::shared::tuple_contains_type_v<T, AllInnerNodeTypes>)
            {
                auto node = T(std::forward<Args>(args)...);
                node.span = span;
                return node;
            }
            else
            {
                auto node = std::make_unique<T>(std::forward<Args>(args)...);
                node->span = span;
                return node;
            }
        }
    };
}
