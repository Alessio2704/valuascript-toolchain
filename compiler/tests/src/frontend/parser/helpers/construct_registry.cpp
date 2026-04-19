#include "construct_registry.h"

namespace valuascript::compiler::test
{
#define DEFINE_REGISTRY(Name, Type) \
std::vector<RegistryEntry<Type>>& ConstructRegistry::Name() { \
static std::vector<RegistryEntry<Type>> vec; \
return vec; \
}

    DEFINE_REGISTRY(imports, ImportVerifier)
    DEFINE_REGISTRY(directives, DirectiveVerifier)
    DEFINE_REGISTRY(functions, FuncVerifier)
    DEFINE_REGISTRY(structs, StructVerifier)
    DEFINE_REGISTRY(enums, EnumVerifier)
    DEFINE_REGISTRY(aliases, AliasVerifier)
    DEFINE_REGISTRY(assignments, AssignmentVerifier)
    DEFINE_REGISTRY(reassignments, ReassignmentVerifier)
    DEFINE_REGISTRY(returns, ReturnVerifier)
    DEFINE_REGISTRY(expr_stmts, ExprStmtVerifier)
    DEFINE_REGISTRY(expressions, ExprVerifier)
    DEFINE_REGISTRY(modifiers, ModifierVerifier)
    DEFINE_REGISTRY(type_annotations, TypeVerifier)
}
