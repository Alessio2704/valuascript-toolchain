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
    DEFINE_REGISTRY(extensions, ExtVerifier)
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

    std::vector<RegistryEntry<FuncVerifier>>& ConstructRegistry::modified_functions()
    {
        static std::vector<RegistryEntry<FuncVerifier>> vec = {
            {"func_mod1", "@mod1 func f_m1() -> void {}\n", IsFunctionDef("f_m1", {{"mod1", {}}}, {}, {IsType("void")})},
            {"func_mod2", "@mod2() func f_m2() -> void {}\n", IsFunctionDef("f_m2", {{"mod2", {}}}, {}, {IsType("void")})},
            {"func_mod3", "@mod3(p: 1) func f_m3() -> void {}\n", IsFunctionDef("f_m3", {{"mod3", {{"p", IsNumber("1")}}}}, {}, {IsType("void")})}
        };
        return vec;
    }

    std::vector<RegistryEntry<ExtVerifier>>& ConstructRegistry::modified_extensions()
    {
        static std::vector<RegistryEntry<ExtVerifier>> vec = {
            {"ext_mod1", "@mod1 extension target {}\n", IsExtensionDef({{"mod1", {}}}, IsType("target"), {})},
            {"ext_mod2", "@mod2() extension target {}\n", IsExtensionDef({{"mod2", {}}}, IsType("target"), {})},
            {"ext_mod3", "@mod3(p: 1) extension target {}\n", IsExtensionDef({{"mod3", {{"p", IsNumber("1")}}}}, IsType("target"), {})}
        };
        return vec;
    }

    std::vector<RegistryEntry<StructVerifier>>& ConstructRegistry::modified_structs()
    {
        static std::vector<RegistryEntry<StructVerifier>> vec = {
            {"struct_mod1", "@mod1 struct s_m1 {}\n", IsStructDef("s_m1", {{"mod1", {}}})},
            {"struct_mod2", "@mod2() struct s_m2 {}\n", IsStructDef("s_m2", {{"mod2", {}}})},
            {"struct_mod3", "@mod3(p: 1) struct s_m3 {}\n", IsStructDef("s_m3", {{"mod3", {{"p", IsNumber("1")}}}})}
        };
        return vec;
    }

    std::vector<RegistryEntry<EnumVerifier>>& ConstructRegistry::modified_enums()
    {
        static std::vector<RegistryEntry<EnumVerifier>> vec = {
            {"enum_mod1", "@mod1 enum e_m1: int {}\n", IsEnumDef("e_m1", {{"mod1", {}}}, IsType("int"))},
            {"enum_mod2", "@mod2() enum e_m2: int {}\n", IsEnumDef("e_m2", {{"mod2", {}}}, IsType("int"))},
            {"enum_mod3", "@mod3(p: 1) enum e_m3: int {}\n", IsEnumDef("e_m3", {{"mod3", {{"p", IsNumber("1")}}}}, IsType("int"))}
        };
        return vec;
    }

    std::vector<RegistryEntry<AliasVerifier>>& ConstructRegistry::modified_aliases()
    {
        static std::vector<RegistryEntry<AliasVerifier>> vec = {
            {"alias_mod1", "@mod1 typealias a_m1 = int\n", IsTypeAlias("a_m1", {{"mod1", {}}}, IsType("int"))},
            {"alias_mod2", "@mod2() typealias a_m2 = int\n", IsTypeAlias("a_m2", {{"mod2", {}}}, IsType("int"))},
            {"alias_mod3", "@mod3(p: 1) typealias a_m3 = int\n", IsTypeAlias("a_m3", {{"mod3", {{"p", IsNumber("1")}}}}, IsType("int"))}
        };
        return vec;
    }

    std::vector<RegistryEntry<ImportVerifier>>& ConstructRegistry::modified_imports()
    {
        static std::vector<RegistryEntry<ImportVerifier>> vec = {
            {"import_mod1", "@mod1 import \"lib\"\n", IsImport("\"lib\"", {{"mod1", {}}})},
            {"import_mod2", "@mod2() import \"lib\"\n", IsImport("\"lib\"", {{"mod2", {}}})},
            {"import_mod3", "@mod3(p: 1) import \"lib\"\n", IsImport("\"lib\"", {{"mod3", {{"p", IsNumber("1")}}}})}
        };
        return vec;
    }

    std::vector<RegistryEntry<AssignmentVerifier>>& ConstructRegistry::modified_assignments()
    {
        static std::vector<RegistryEntry<AssignmentVerifier>> vec = {
            {"assign_mod1", "@mod1 let a_m1 = 1\n", IsAssignment({{ {{"mod1", {}}}, "a_m1" }}, IsNumber("1"))},
            {"assign_mod2", "@mod2() let a_m2 = 1\n", IsAssignment({{ {{"mod2", {}}}, "a_m2" }}, IsNumber("1"))},
            {"assign_mod3", "@mod3(p: 1) let a_m3 = 1\n", IsAssignment({{ {{"mod3", {{"p", IsNumber("1")}}}}, "a_m3" }}, IsNumber("1"))}
        };
        return vec;
    }

    std::vector<RegistryEntry<ReturnVerifier>>& ConstructRegistry::modified_returns()
    {
        static std::vector<RegistryEntry<ReturnVerifier>> vec = {
            {"return_mod1", "@mod1 return 1\n", IsReturn({{"mod1", {}}}, {IsNumber("1")})},
            {"return_mod2", "@mod2() return 1\n", IsReturn({{"mod2", {}}}, {IsNumber("1")})},
            {"return_mod3", "@mod3(p: 1) return 1\n", IsReturn({{"mod3", {{"p", IsNumber("1")}}}}, {IsNumber("1")})}
        };
        return vec;
    }
}
