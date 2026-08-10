#pragma once

#include "context_names.h"
#include <array>
#include <string_view>

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
}
