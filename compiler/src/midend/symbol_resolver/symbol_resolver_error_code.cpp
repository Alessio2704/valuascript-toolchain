#include "symbol_resolver_error_code.h"

namespace valuascript::compiler
{
    std::string_view get_error_template(SymbolResolverErrorCode code)
    {
        switch (code)
        {
        case SymbolResolverErrorCode::RedeclarationInSameScope:
            return "Semantic Error: Symbol '{}' is already declared in this scope.";
        case SymbolResolverErrorCode::UndeclaredIdentifier:
            return "Semantic Error: Use of undeclared identifier '{}'.";
        case SymbolResolverErrorCode::ShadowingBuiltin:
            return "Semantic Error: Declaration '{}' shadows built-in type.";
        }
        return "Unknown Semantic Error";
    }
}
