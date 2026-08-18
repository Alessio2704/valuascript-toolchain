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
            {.test_name = "func_mod1", .code = "@mod1 func f_m1() -> void {}\n", .verifier = IsFunctionDef("f_m1", {{.name="mod1", .args={}}}, {}, {IsType("void")})},
            {.test_name = "func_mod2", .code = "@mod2() func f_m2() -> void {}\n", .verifier = IsFunctionDef("f_m2", {{.name="mod2", .args={}}}, {}, {IsType("void")})},
            {.test_name = "func_mod3", .code = "@mod3(p: 1) func f_m3() -> void {}\n", .verifier = IsFunctionDef("f_m3", {{.name="mod3", .args={{.label="p", .value_v=IsNumber("1")}}}}, {}, {IsType("void")})}
        };
        return vec;
    }

    std::vector<RegistryEntry<ExtVerifier>>& ConstructRegistry::modified_extensions()
    {
        static std::vector<RegistryEntry<ExtVerifier>> vec = {
            {.test_name = "ext_mod1", .code = "@mod1 extension target {}\n", .verifier = IsExtensionDef({{.name="mod1", .args={}}}, IsType("target"), {})},
            {.test_name = "ext_mod2", .code = "@mod2() extension target {}\n", .verifier = IsExtensionDef({{.name="mod2", .args={}}}, IsType("target"), {})},
            {.test_name = "ext_mod3", .code = "@mod3(p: 1) extension target {}\n", .verifier = IsExtensionDef({{.name="mod3", .args={{.label="p", .value_v=IsNumber("1")}}}}, IsType("target"), {})}
        };
        return vec;
    }

    std::vector<RegistryEntry<StructVerifier>>& ConstructRegistry::modified_structs()
    {
        static std::vector<RegistryEntry<StructVerifier>> vec = {
            {.test_name = "struct_mod1", .code = "@mod1 struct s_m1 {}\n", .verifier = IsStructDef("s_m1", {{.name="mod1", .args={}}})},
            {.test_name = "struct_mod2", .code = "@mod2() struct s_m2 {}\n", .verifier = IsStructDef("s_m2", {{.name="mod2", .args={}}})},
            {.test_name = "struct_mod3", .code = "@mod3(p: 1) struct s_m3 {}\n", .verifier = IsStructDef("s_m3", {{.name="mod3", .args={{.label="p", .value_v=IsNumber("1")}}}})}
        };
        return vec;
    }

    std::vector<RegistryEntry<EnumVerifier>>& ConstructRegistry::modified_enums()
    {
        static std::vector<RegistryEntry<EnumVerifier>> vec = {
            {.test_name = "enum_mod1", .code = "@mod1 enum e_m1: int {}\n", .verifier = IsEnumDef("e_m1", {{.name="mod1", .args={}}}, IsType("int"))},
            {.test_name = "enum_mod2", .code = "@mod2() enum e_m2: int {}\n", .verifier = IsEnumDef("e_m2", {{.name="mod2", .args={}}}, IsType("int"))},
            {.test_name = "enum_mod3", .code = "@mod3(p: 1) enum e_m3: int {}\n", .verifier = IsEnumDef("e_m3", {{.name="mod3", .args={{.label="p", .value_v=IsNumber("1")}}}}, IsType("int"))}
        };
        return vec;
    }

    std::vector<RegistryEntry<AliasVerifier>>& ConstructRegistry::modified_aliases()
    {
        static std::vector<RegistryEntry<AliasVerifier>> vec = {
            {.test_name = "alias_mod1", .code = "@mod1 typealias a_m1 = int\n", .verifier = IsTypeAlias("a_m1", {{.name="mod1", .args={}}}, IsType("int"))},
            {.test_name = "alias_mod2", .code = "@mod2() typealias a_m2 = int\n", .verifier = IsTypeAlias("a_m2", {{.name="mod2", .args={}}}, IsType("int"))},
            {.test_name = "alias_mod3", .code = "@mod3(p: 1) typealias a_m3 = int\n", .verifier = IsTypeAlias("a_m3", {{.name="mod3", .args={{.label="p", .value_v=IsNumber("1")}}}}, IsType("int"))}
        };
        return vec;
    }

    std::vector<RegistryEntry<ImportVerifier>>& ConstructRegistry::modified_imports()
    {
        static std::vector<RegistryEntry<ImportVerifier>> vec = {
            {.test_name = "import_mod1", .code = "@mod1 import \"lib\"\n", .verifier = IsImport("\"lib\"", {{.name="mod1", .args={}}})},
            {.test_name = "import_mod2", .code = "@mod2() import \"lib\"\n", .verifier = IsImport("\"lib\"", {{.name="mod2", .args={}}})},
            {.test_name = "import_mod3", .code = "@mod3(p: 1) import \"lib\"\n", .verifier = IsImport("\"lib\"", {{.name="mod3", .args={{.label="p", .value_v=IsNumber("1")}}}})}
        };
        return vec;
    }

    std::vector<RegistryEntry<AssignmentVerifier>>& ConstructRegistry::modified_assignments()
    {
        static std::vector<RegistryEntry<AssignmentVerifier>> vec = {
            {.test_name = "assign_mod1", .code = "@mod1 let a_m1 = 1\n", .verifier = IsAssignment({AssignmentTargetSpec{.modifiers = {{.name="mod1", .args={}}}, .name = "a_m1"}}, IsNumber("1"))},
            {.test_name = "assign_mod2", .code = "@mod2() let a_m2 = 1\n", .verifier = IsAssignment({AssignmentTargetSpec{.modifiers = {{.name="mod2", .args={}}}, .name = "a_m2"}}, IsNumber("1"))},
            {.test_name = "assign_mod3", .code = "@mod3(p: 1) let a_m3 = 1\n", .verifier = IsAssignment({AssignmentTargetSpec{.modifiers = {{.name="mod3", .args={{.label="p", .value_v=IsNumber("1")}}}}, .name = "a_m3"}}, IsNumber("1"))}
        };
        return vec;
    }

    std::vector<RegistryEntry<ReturnVerifier>>& ConstructRegistry::modified_returns()
    {
        static std::vector<RegistryEntry<ReturnVerifier>> vec = {
            {.test_name = "return_mod1", .code = "@mod1 return 1\n", .verifier = IsReturn({{.name="mod1", .args={}}}, {IsNumber("1")})},
            {.test_name = "return_mod2", .code = "@mod2() return 1\n", .verifier = IsReturn({{.name="mod2", .args={}}}, {IsNumber("1")})},
            {.test_name = "return_mod3", .code = "@mod3(p: 1) return 1\n", .verifier = IsReturn({{.name="mod3", .args={{.label="p", .value_v=IsNumber("1")}}}}, {IsNumber("1")})}
        };
        return vec;
    }
}
