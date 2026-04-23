#include "context_registry.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_type_contexts()
    {
        return {
            {
                "assignment_target", {InjectableType::TypeAnnotation}, InjectableType::StrongStatement,
                "let ctx_assign: ", " = 1\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({}, {{"ctx_assign", SpecAdder::get_v<TypeVerifier>(v)}},
                                                          IsNumber("1")));
                }
            },
            {
                "multi_assignment_target_1", {InjectableType::TypeAnnotation}, InjectableType::StrongStatement,
                "let ctx_m1: ", ", ctx_m2 = 1\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsAssignment({}, {{"ctx_m1", SpecAdder::get_v<TypeVerifier>(v)}, {"ctx_m2"}}, IsNumber("1")));
                }
            },
            {
                "multi_assignment_target_2", {InjectableType::TypeAnnotation}, InjectableType::StrongStatement,
                "let ctx_m1, ctx_m2: ", " = 1\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsAssignment({}, {{"ctx_m1"}, {"ctx_m2", SpecAdder::get_v<TypeVerifier>(v)}}, IsNumber("1")));
                }
            },
            {
                "typealias_target", {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "typealias ctx_alias = ", "\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTypeAlias("ctx_alias", {}, SpecAdder::get_v<TypeVerifier>(v)));
                }
            },
            {
                "function_parameter", {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "func ctx_func_param(p: ", ") -> void {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_param", {},
                                                           {ParamSpec{"p", {}, SpecAdder::get_v<TypeVerifier>(v)}},
                                                           {IsType("void")}));
                }
            },
            {
                "function_return", {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "func ctx_func_ret() -> ", " {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_ret", {}, {}, {
                                                               SpecAdder::get_v<TypeVerifier>(v)
                                                           }));
                }
            },
            {
                "function_multi_return", {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "func ctx_func_multi_ret() -> ", ", int {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                           {SpecAdder::get_v<TypeVerifier>(v), IsType("int")}));
                }
            },
            {
                "struct_field", {InjectableType::TypeAnnotation}, InjectableType::TopLevel, "struct ctx_struct { f: ",
                " }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_struct", {}, {
                                                             FieldSpec{"f", {}, SpecAdder::get_v<TypeVerifier>(v)}
                                                         }));
                }
            },
            {
                "enum_underlying_type", {InjectableType::TypeAnnotation}, InjectableType::TopLevel, "enum ctx_enum: ",
                " { A }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_enum", {}, SpecAdder::get_v<TypeVerifier>(v), {{"A"}}));
                }
            }
        };
    }
}
