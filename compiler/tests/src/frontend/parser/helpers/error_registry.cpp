#include "error_registry.h"

namespace valuascript::compiler::test
{
#define DEFINE_ERROR_REGISTRY(Name, Type) \
std::vector<ErrorRegistryEntry<Type>>& ErrorRegistry::Name() { \
static std::vector<ErrorRegistryEntry<Type>> vec; \
return vec; \
}

    DEFINE_ERROR_REGISTRY(imports, ImportVerifier)
    DEFINE_ERROR_REGISTRY(directives, DirectiveVerifier)
    DEFINE_ERROR_REGISTRY(functions, FuncVerifier)
    DEFINE_ERROR_REGISTRY(extensions, ExtVerifier)
    DEFINE_ERROR_REGISTRY(structs, StructVerifier)
    DEFINE_ERROR_REGISTRY(enums, EnumVerifier)
    DEFINE_ERROR_REGISTRY(aliases, AliasVerifier)
    DEFINE_ERROR_REGISTRY(assignments, AssignmentVerifier)
    DEFINE_ERROR_REGISTRY(reassignments, ReassignmentVerifier)
    DEFINE_ERROR_REGISTRY(returns, ReturnVerifier)
    DEFINE_ERROR_REGISTRY(expr_stmts, ExprStmtVerifier)
    DEFINE_ERROR_REGISTRY(expressions, ExprVerifier)
    DEFINE_ERROR_REGISTRY(modifiers, ModifierVerifier)
    DEFINE_ERROR_REGISTRY(type_annotations, TypeVerifier)
}
