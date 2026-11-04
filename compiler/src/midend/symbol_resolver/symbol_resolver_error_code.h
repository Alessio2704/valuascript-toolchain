#pragma once
#include <string_view>

namespace valuascript::compiler
{
    enum class SymbolResolverErrorCode
    {
        RedeclarationInSameScope = 5001,
        UndeclaredIdentifier,
        ShadowingBuiltin
    };

    std::string_view get_error_template(SymbolResolverErrorCode code);
}
