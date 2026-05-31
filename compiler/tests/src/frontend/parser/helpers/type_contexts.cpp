#include "context_registry.h"
#include "spec_adder.h"
#include "context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_type_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                ContextNames::TypeAssignmentTarget, {InjectableType::TypeAnnotation}, InjectableType::StrongStatement,
                "let ctx_assign: ", " = 1\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({{"ctx_assign", SpecAdder::get_v<TypeVerifier>(v)}},
                                                          IsNumber("1")));
                }
            },
            {
                ContextNames::TypeMultiAssignmentTarget1, {InjectableType::TypeAnnotation},
                InjectableType::StrongStatement,
                "let ctx_m1: ", ", ctx_m2 = 1\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsAssignment({{"ctx_m1", SpecAdder::get_v<TypeVerifier>(v)}, {"ctx_m2"}}, IsNumber("1")));
                }
            },
            {
                ContextNames::TypeMultiAssignmentTarget2, {InjectableType::TypeAnnotation},
                InjectableType::StrongStatement,
                "let ctx_m1, ctx_m2: ", " = 1\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsAssignment({{"ctx_m1"}, {"ctx_m2", SpecAdder::get_v<TypeVerifier>(v)}}, IsNumber("1")));
                }
            },
            {
                ContextNames::TypeTypealiasTarget, {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "typealias ctx_alias = ", "\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTypeAlias("ctx_alias", {}, SpecAdder::get_v<TypeVerifier>(v)));
                }
            },
            {
                ContextNames::TypeFunctionParameter, {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "func ctx_func_param(p: ", ") -> void {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_param", {},
                                                           {ParamSpec{"p", {}, SpecAdder::get_v<TypeVerifier>(v)}},
                                                           {IsType("void")}));
                }
            },
            {
                ContextNames::TypeFunctionReturn, {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "func ctx_func_ret() -> ", " {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_ret", {}, {}, {
                                                               SpecAdder::get_v<TypeVerifier>(v)
                                                           }));
                }
            },
            {
                ContextNames::TypeFunctionMultiReturn, {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "func ctx_func_multi_ret() -> ", ", int {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                           {SpecAdder::get_v<TypeVerifier>(v), IsType("int")}));
                }
            },
            {
                ContextNames::TypeStructField, {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "struct ctx_struct { f: ",
                " }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_struct", {}, {
                                                             FieldSpec{"f", {}, SpecAdder::get_v<TypeVerifier>(v)}
                                                         }));
                }
            },
            {
                ContextNames::TypeEnumUnderlyingType, {InjectableType::TypeAnnotation}, InjectableType::TopLevel,
                "enum ctx_enum: ",
                " { A }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_enum", {}, SpecAdder::get_v<TypeVerifier>(v), {{"A"}}));
                }
            },
            {
                ContextNames::TypeTupleTypeStart, {InjectableType::TypeAnnotation}, InjectableType::TypeAnnotation, "(",
                ", int)", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsTupleType({
                            SpecAdder::get_v<TypeVerifier>(v),
                            IsType("int")
                        }));
                }
            },
            {
                ContextNames::TypeTupleTypeMiddle, {InjectableType::TypeAnnotation}, InjectableType::TypeAnnotation,
                "(int, ",
                ", string)", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsTupleType({
                            IsType("int"),
                            SpecAdder::get_v<TypeVerifier>(v),
                            IsType("string"),
                        }));
                }
            },
            {
                ContextNames::TypeTupleTypeEnd, {InjectableType::TypeAnnotation}, InjectableType::TypeAnnotation,
                "(int, string, ",
                ")", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsTupleType({
                            IsType("int"),
                            IsType("string"),
                            SpecAdder::get_v<TypeVerifier>(v)
                        }));
                }
            },
            {
                ContextNames::TypeGenericTypeStart, {InjectableType::TypeAnnotation}, InjectableType::TypeAnnotation,
                "vector<",
                ", int>", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsType("vector", {
                                   SpecAdder::get_v<TypeVerifier>(v),
                                   IsType("int")
                               }
                        )
                    );
                }
            },
            {
                ContextNames::TypeGenericTypeMiddle, {InjectableType::TypeAnnotation}, InjectableType::TypeAnnotation,
                "vector<int, ",
                ", string>", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsType("vector", {
                                   IsType("int"),
                                   SpecAdder::get_v<TypeVerifier>(v),
                                   IsType("string")
                               }
                        )
                    );
                }
            },
            {
                ContextNames::TypeGenericTypeEnd, {InjectableType::TypeAnnotation}, InjectableType::TypeAnnotation,
                "vector<int, string, ",
                ">", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsType("vector", {
                                   IsType("int"),
                                   IsType("string"),
                                   SpecAdder::get_v<TypeVerifier>(v)
                               }
                        )
                    );
                }
            }
        };

        return contexts;
    }
}
