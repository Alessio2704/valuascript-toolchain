#pragma once

#include "context_names.h"
#include "context_registry.h"
#include <array>
#include <string_view>
#include <vector>

namespace valuascript::compiler::test::ContextNames
{
#define VALUASCRIPT_MAKE_STRING_VIEW(name, str) std::string_view{name},
    inline constexpr std::array AllContexts = {
        VALUASCRIPT_ALL_CONTEXTS(VALUASCRIPT_MAKE_STRING_VIEW)
    };

#undef VALUASCRIPT_MAKE_STRING_VIEW

    inline std::vector<std::string_view> all()
    {
        return {AllContexts.begin(), AllContexts.end()};
    }

    inline const std::vector<std::string_view>& all_nested_expressions()
    {
        return ContextRegistry::get_nested_expression_context_names();
    }

    inline std::vector<std::string_view> all_nested_swallowing_tensor_contexts()
    {
        return {
            ExprTensorStart,
            ExprTensorMiddle,
            ExprTupleStart,
            ExprTupleMiddle
        };
    }

    inline std::vector<std::string_view> all_nested_swallowing_type_contexts()
    {
        return {
            TypeTupleTypeStart,
            TypeTupleTypeMiddle,
            TypeGenericTypeStart,
            TypeGenericTypeMiddle
        };
    }

    inline std::vector<std::string_view> all_nested_swallowing_dict_contexts()
    {
        return {
            ExprDictValueStart,
            ExprDictValueMiddle,
            ExprCallArgStart,
            ExprCallArgMiddle
        };
    }

    inline std::vector<std::string_view> all_nested_swallowing_bracket_contexts()
    {
        return {
            ExprTensorStart,
            ExprTensorMiddle,
            ExprTupleStart,
            ExprTupleMiddle,
            ExprDictValueStart,
            ExprDictValueMiddle,
            ExprCallArgStart,
            ExprCallArgMiddle
        };
    }

    inline std::vector<std::string_view> all_nested_swallowing_tuple_contexts()
    {
        return {
            ExprTupleStart,
            ExprTupleMiddle,
            ExprTupleEnd,
            ExprTensorStart,
            ExprTensorMiddle,
            ExprDictValueStart,
            ExprDictValueMiddle,
            ExprCallArgStart,
            ExprCallArgMiddle,
            ExprCallArgEnd,
            ExprCallArgSingle,
            ExprGrouping,
            ExprUnaryGrouping,
            ExprBinaryLhs,
            ExprAsCallTarget,
            ExprAsDotTarget,
            ExprAsBracketTarget,
            ExprAsSliceTarget
        };
    }

    inline std::vector<std::string_view> all_nested_swallowing_grouping_contexts()
    {
        return {
            ExprTupleStart,
            ExprTupleMiddle,
            ExprTupleEnd,
            ExprTensorStart,
            ExprTensorMiddle,
            ExprDictValueStart,
            ExprDictValueMiddle,
            ExprCallArgStart,
            ExprCallArgMiddle,
            ExprCallArgEnd,
            ExprCallArgSingle,
            ExprGrouping,
            ExprUnaryGrouping,
            ExprBinaryLhs,
            ExprAsCallTarget,
            ExprAsDotTarget,
            ExprAsBracketTarget,
            ExprAsSliceTarget
        };
    }
}
