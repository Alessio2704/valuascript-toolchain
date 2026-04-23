#include "context_registry.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_modifier_contexts()
    {
        return {
            {
                "assignment", {InjectableType::Modifier}, InjectableType::StrongStatement, "", " let ctx_assign = 1\n", [
                ](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment(SpecAdder::get_v<ModifierVerifier>(v), {{"ctx_assign"}},
                                                          IsNumber("1")));
                }
            },
            {
                "function_definition", {InjectableType::Modifier}, InjectableType::TopLevel, "",
                " func ctx_func() -> void {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func", SpecAdder::get_v<ModifierVerifier>(v), {},
                                                           {IsType("void")}));
                }
            },
            {
                "function_parameter", {InjectableType::Modifier}, InjectableType::TopLevel, "func ctx_param(",
                " p: int) -> void {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_param", {},
                                                           {
                                                               ParamSpec{
                                                                   "p", SpecAdder::get_v<ModifierVerifier>(v),
                                                                   IsType("int")
                                                               }
                                                           }, {IsType("void")}));
                }
            },
            {
                "struct_definition", {InjectableType::Modifier}, InjectableType::TopLevel, "", " struct ctx_struct {}\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_struct", SpecAdder::get_v<ModifierVerifier>(v), {}));
                }
            },
            {
                "struct_field", {InjectableType::Modifier}, InjectableType::TopLevel, "struct ctx_s_field { ",
                " f: int }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_s_field", {}, {
                                                             FieldSpec{
                                                                 "f", SpecAdder::get_v<ModifierVerifier>(v),
                                                                 IsType("int")
                                                             }
                                                         }));
                }
            },
            {
                "enum_definition", {InjectableType::Modifier}, InjectableType::TopLevel, "",
                " enum ctx_enum: int { A = 1 }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_enum", SpecAdder::get_v<ModifierVerifier>(v), IsType("int"),
                                                       {{"A", {}, IsNumber("1")}}));
                }
            },
            {
                "enum_case", {InjectableType::Modifier}, InjectableType::TopLevel, "enum ctx_e_case: int { ",
                " A = 1 }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_e_case", {}, IsType("int"), {
                                                           {"A", SpecAdder::get_v<ModifierVerifier>(v), IsNumber("1")}
                                                       }));
                }
            },
            {
                "typealias_definition", {InjectableType::Modifier}, InjectableType::TopLevel, "",
                " typealias ctx_alias = int\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTypeAlias("ctx_alias", SpecAdder::get_v<ModifierVerifier>(v),
                                                         IsType("int")));
                }
            },
            {
                "dict_item", {InjectableType::Modifier}, InjectableType::Expression, "{ ", " k: 1 }",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDict({
                        DictItemSpec{"k", SpecAdder::get_v<ModifierVerifier>(v), IsNumber("1")}
                    }));
                }
            }
        };
    }
}
