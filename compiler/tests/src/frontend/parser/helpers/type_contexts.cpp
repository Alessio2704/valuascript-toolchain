#include "context_registry.h"
#include "spec_adder.h"
#include "context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_type_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                .name = ContextNames::TypeAssignmentTarget,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ctx_assign: ",
                .suffix = " = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({AssignmentTargetSpec{.name = "ctx_assign", .type_v = SpecAdder::get_v<TypeVerifier>(v)}},
                                                          IsNumber("1")));
                }
            },
            {
                .name = ContextNames::TypeMultiAssignmentTarget1,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ctx_m1: ",
                .suffix = ", ctx_m2 = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsAssignment({AssignmentTargetSpec{.name = "ctx_m1", .type_v = SpecAdder::get_v<TypeVerifier>(v)}, AssignmentTargetSpec{.name = "ctx_m2"}}, IsNumber("1")));
                }
            },
            {
                .name = ContextNames::TypeMultiAssignmentTarget2,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ctx_m1, ctx_m2: ",
                .suffix = " = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsAssignment({AssignmentTargetSpec{.name = "ctx_m1"}, AssignmentTargetSpec{.name = "ctx_m2", .type_v = SpecAdder::get_v<TypeVerifier>(v)}}, IsNumber("1")));
                }
            },
            {
                .name = ContextNames::TypeTypealiasTarget,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "typealias ctx_alias = ",
                .suffix = "\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTypeAlias("ctx_alias", {}, SpecAdder::get_v<TypeVerifier>(v)));
                }
            },
            {
                .name = ContextNames::TypeExtensionTarget,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "extension ",
                .suffix = " {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsExtensionDef({}, SpecAdder::get_v<TypeVerifier>(v)));
                }
            },
            {
                .name = ContextNames::TypeFunctionParameter,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_func_param(p: ",
                .suffix = ") -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_param", {},
                                                           {ParamSpec{.name = "p", .type_v = SpecAdder::get_v<TypeVerifier>(v)}},
                                                           {IsType("void")}));
                }
            },
            {
                .name = ContextNames::TypeFunctionMultiParameter1,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_func_param(p1: ",
                .suffix = ", p2: int) -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_param", {},
                                                           {ParamSpec{.name = "p1", .type_v = SpecAdder::get_v<TypeVerifier>(v)},
                                                            ParamSpec{.name = "p2", .type_v = IsType("int")}},
                                                           {IsType("void")}));
                }
            },
            {
                .name = ContextNames::TypeFunctionMultiParameter2,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_func_param(p1: int, p2: ",
                .suffix = ") -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_param", {},
                                                           {ParamSpec{.name = "p1", .type_v = IsType("int")},
                                                            ParamSpec{.name = "p2", .type_v = SpecAdder::get_v<TypeVerifier>(v)}},
                                                           {IsType("void")}));
                }
            },
            {
                .name = ContextNames::TypeFunctionReturn,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_func_ret() -> ",
                .suffix = " {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_ret", {}, {}, {
                                                               SpecAdder::get_v<TypeVerifier>(v)
                                                           }));
                }
            },
            {
                .name = ContextNames::TypeFunctionMultiReturn,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_func_multi_ret() -> ",
                .suffix = ", int {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                           {SpecAdder::get_v<TypeVerifier>(v), IsType("int")}));
                }
            },
            {
                .name = ContextNames::TypeFunctionMultiReturnEnd,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_func_multi_ret() -> int, ",
                .suffix = " {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                           {IsType("int"), SpecAdder::get_v<TypeVerifier>(v)}));
                }
            },
            {
                .name = ContextNames::TypeStructField,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "struct ctx_struct { f: ",
                .suffix = " }\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_struct",
                                                             FieldSpec{.name = "f", .type_v = SpecAdder::get_v<TypeVerifier>(v)}
                                                         ));
                }
            },
            {
                .name = ContextNames::TypeStructMultipleFields,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "struct ctx_struct { f1: ",
                .suffix = ", \nf2: int }\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_struct",
                                                             FieldSpec{.name = "f1", .type_v = SpecAdder::get_v<TypeVerifier>(v)},
                                                             FieldSpec{.name = "f2", .type_v = IsType("int")}
                                                         ));
                }
            },
            {
                .name = ContextNames::TypeEnumUnderlyingType,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TopLevel,
                .prefix = "enum ctx_enum: ",
                .suffix = " { A }\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_enum", {}, SpecAdder::get_v<TypeVerifier>(v), EnumCaseSpec{.name = "A"}));
                }
            },
            {
                .name = ContextNames::TypeTupleTypeStart,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TypeAnnotation,
                .prefix = "(",
                .suffix = ", int)",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsTupleType(
                            SpecAdder::get_v<TypeVerifier>(v),
                            IsType("int")
                        ));
                }
            },
            {
                .name = ContextNames::TypeTupleTypeMiddle,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TypeAnnotation,
                .prefix = "(int, ",
                .suffix = ", string)",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsTupleType(
                            IsType("int"),
                            SpecAdder::get_v<TypeVerifier>(v),
                            IsType("string")
                        ));
                }
            },
            {
                .name = ContextNames::TypeTupleTypeEnd,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TypeAnnotation,
                .prefix = "(int, string, ",
                .suffix = ")",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsTupleType(
                            IsType("int"),
                            IsType("string"),
                            SpecAdder::get_v<TypeVerifier>(v)
                        ));
                }
            },
            {
                .name = ContextNames::TypeGenericTypeStart,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TypeAnnotation,
                .prefix = "vector<",
                .suffix = ", int>",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsType("vector",
                            SpecAdder::get_v<TypeVerifier>(v),
                            IsType("int")
                        )
                    );
                }
            },
            {
                .name = ContextNames::TypeGenericTypeMiddle,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TypeAnnotation,
                .prefix = "vector<int, ",
                .suffix = ", string>",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsType("vector",
                            IsType("int"),
                            SpecAdder::get_v<TypeVerifier>(v),
                            IsType("string")
                        )
                    );
                }
            },
            {
                .name = ContextNames::TypeGenericTypeEnd,
                .input_types = {InjectableType::TypeAnnotation},
                .output_type = InjectableType::TypeAnnotation,
                .prefix = "vector<int, string, ",
                .suffix = ">",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsType("vector",
                            IsType("int"),
                            IsType("string"),
                            SpecAdder::get_v<TypeVerifier>(v)
                        )
                    );
                }
            }
        };

        return contexts;
    }
}
