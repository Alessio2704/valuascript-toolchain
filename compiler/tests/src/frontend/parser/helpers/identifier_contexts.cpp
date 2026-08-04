#include "context_registry.h"
#include "spec_adder.h"
#include "context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_identifier_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                .name = ContextNames::IdLetTarget,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ",
                .suffix = " = 1\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsAssignment({AssignmentTargetSpec{.name = SpecAdder::get_id(v)}}, IsNumber("1")));
                }
            },
            {
                .name = ContextNames::IdMultiLetTarget1,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ",
                .suffix = ", ctx_b = 1\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsAssignment({AssignmentTargetSpec{.name = SpecAdder::get_id(v)}, AssignmentTargetSpec{.name = "ctx_b"}}, IsNumber("1")));
                }
            },
            {
                .name = ContextNames::IdMultiLetTarget2,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ctx_a, ",
                .suffix = " = 1\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsAssignment({AssignmentTargetSpec{.name = "ctx_a"}, AssignmentTargetSpec{.name = SpecAdder::get_id(v)}}, IsNumber("1")));
                }
            },
            // {
            //     ContextNames::IdReassignmentTarget, {InjectableType::Identifier}, InjectableType::StrongStatement,
            //     "", " = 1\n",
            //     [](const UniversalVerifier& v)
            //     {
            //         return UniversalVerifier(IsReassignment(IsIdentifier(SpecAdder::get_id(v)), IsNumber("1")));
            //     }
            // },
            {
                .name = ContextNames::IdFuncDefName,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ",
                .suffix = "() -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsFunctionDef(SpecAdder::get_id(v), {}, {}, {IsType("void")}));
                }
            },
            {
                .name = ContextNames::IdFuncParamName,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_f(",
                .suffix = ": int) -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsFunctionDef("ctx_f", {},
                                                           {ParamSpec{.name = SpecAdder::get_id(v), .type_v = IsType("int")}},
                                                           {IsType("void")}));
                }
            },
            {
                .name = ContextNames::IdStructDefName,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "struct ",
                .suffix = " {}\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsStructDef(SpecAdder::get_id(v)));
                }
            },
            {
                .name = ContextNames::IdStructFieldNameStart,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "struct ctx_s { ",
                .suffix = ": int, b: string, c: bool }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsStructDef("ctx_s", {}, {{SpecAdder::get_id(v), {}, IsType("int")}, {"b", {}, IsType("string")}, {"c", {}, IsType("bool")}}));
                }
            },
            {
                .name = ContextNames::IdStructFieldNameMiddle,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "struct ctx_s { a: int, ",
                .suffix = ": string, c: bool }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsStructDef("ctx_s", {}, {{"a", {}, IsType("int")}, {SpecAdder::get_id(v), {}, IsType("string")}, {"c", {}, IsType("bool")}}));
                }
            },
            {
                .name = ContextNames::IdStructFieldNameEnd,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "struct ctx_s { a: int, b: string, ",
                .suffix = ": bool }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsStructDef("ctx_s", {}, {{"a", {}, IsType("int")}, {"b", {}, IsType("string")}, {SpecAdder::get_id(v), {}, IsType("bool")}}));
                }
            },
            {
                .name = ContextNames::IdStructFieldNameSingle,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "struct ctx_s { ",
                .suffix = ": int }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsStructDef("ctx_s", {}, {{SpecAdder::get_id(v), {}, IsType("int")}}));
                }
            },
            {
                .name = ContextNames::IdEnumDefName,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "enum ",
                .suffix = ": int { A = 1 }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef(SpecAdder::get_id(v), {}, IsType("int"),
                                                       {{"A", {}, IsNumber("1")}}));
                }
            },
            {
                .name = ContextNames::IdEnumCaseStart,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "enum ctx_e: int { ",
                .suffix = " = 1, B = 2, C = 3 }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef("ctx_e", {}, IsType("int"), {
                                                           {SpecAdder::get_id(v), {}, IsNumber("1")},
                                                           {"B", {}, IsNumber("2")},
                                                           {"C", {}, IsNumber("3")}
                                                       }));
                }
            },
            {
                .name = ContextNames::IdEnumCaseMiddle,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "enum ctx_e: int { A = 1, ",
                .suffix = " = 2, C = 3 }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef("ctx_e", {}, IsType("int"), {
                                                           {"A", {}, IsNumber("1")},
                                                           {SpecAdder::get_id(v), {}, IsNumber("2")},
                                                           {"C", {}, IsNumber("3")}
                                                       }));
                }
            },
            {
                .name = ContextNames::IdEnumCaseEnd,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "enum ctx_e: int { A = 1, B = 2, ",
                .suffix = " = 3 }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef("ctx_e", {}, IsType("int"), {
                                                           {"A", {}, IsNumber("1")},
                                                           {"B", {}, IsNumber("2")},
                                                           {SpecAdder::get_id(v), {}, IsNumber("3")}
                                                       }));
                }
            },
            {
                .name = ContextNames::IdEnumCaseSingle,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "enum ctx_e: int { ",
                .suffix = " = 1 }\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef("ctx_e", {}, IsType("int"), {
                                                           {SpecAdder::get_id(v), {}, IsNumber("1")}
                                                       }));
                }
            },
            {
                .name = ContextNames::IdTypeAliasName,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "typealias ",
                .suffix = " = int\n",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsTypeAlias(SpecAdder::get_id(v), {}, IsType("int")));
                }
            },
            {
                .name = ContextNames::IdTypeAnnotation,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::TypeAnnotation,
                .prefix = "",
                .suffix = "",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsType(SpecAdder::get_id(v)));
                }
            },
            {
                .name = ContextNames::IdDotAccessProperty,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "(ctx_obj).",
                .suffix = "",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsDot(IsGrouping(IsIdentifier("ctx_obj")), SpecAdder::get_id(v)));
                }
            },
            {
                .name = ContextNames::IdDictKeyStart,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "{ ",
                .suffix = ": 1, b: 2, c: 3 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsDict(DictItemSpec{.key = SpecAdder::get_id(v), .value_v = IsNumber("1")}, DictItemSpec{.key = "b", .value_v = IsNumber("2")}, DictItemSpec{.key = "c", .value_v = IsNumber("3")}));
                }
            },
            {
                .name = ContextNames::IdDictKeyMiddle,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "{ a: 1, ",
                .suffix = ": 2, c: 3 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsDict(DictItemSpec{.key = "a", .value_v = IsNumber("1")}, DictItemSpec{.key = SpecAdder::get_id(v), .value_v = IsNumber("2")}, DictItemSpec{.key = "c", .value_v = IsNumber("3")}));
                }
            },
            {
                .name = ContextNames::IdDictKeyEnd,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "{ a: 1, b: 2, ",
                .suffix = ": 3 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsDict(DictItemSpec{.key = "a", .value_v = IsNumber("1")}, DictItemSpec{.key = "b", .value_v = IsNumber("2")}, DictItemSpec{.key = SpecAdder::get_id(v), .value_v = IsNumber("3")}));
                }
            },
            {
                .name = ContextNames::IdDictKeySingle,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "{ ",
                .suffix = ": 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsDict(DictItemSpec{.key = SpecAdder::get_id(v), .value_v = IsNumber("1")}));
                }
            },
            {
                .name = ContextNames::IdSwitchCaseLabelStart,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { case ",
                .suffix = ", B, C -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            SwitchCaseSpec{.labels = {SpecAdder::get_id(v), "B", "C"}, .result_v = IsNumber("1")}
                        )
                    );
                }
            },
            {
                .name = ContextNames::IdSwitchCaseLabelMiddle,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { case A, ",
                .suffix = ", C -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            SwitchCaseSpec{.labels = {"A", SpecAdder::get_id(v), "C"}, .result_v = IsNumber("1")}
                        )
                    );
                }
            },
            {
                .name = ContextNames::IdSwitchCaseLabelEnd,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { case A, B, ",
                .suffix = " -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            SwitchCaseSpec{.labels = {"A", "B", SpecAdder::get_id(v)}, .result_v = IsNumber("1")}
                        )
                    );
                }
            },
            {
                .name = ContextNames::IdSwitchCaseLabelSingle,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { case ",
                .suffix = " -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            SwitchCaseSpec{.labels = {SpecAdder::get_id(v)}, .result_v = IsNumber("1")}
                        )
                    );
                }
            },
            {
                .name = ContextNames::IdModifierName,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Modifier,
                .prefix = "@",
                .suffix = "() ",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(std::vector<ModifierSpec>{{SpecAdder::get_id(v), {}}});
                }
            },
            {
                .name = ContextNames::IdModifierArgLabel,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Modifier,
                .prefix = "@ctx_mod(",
                .suffix = ": 1) ",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(std::vector<ModifierSpec>{
                        {"ctx_mod", {{SpecAdder::get_id(v), IsNumber("1")}}}
                    });
                }
            },
            {
                .name = ContextNames::IdCallArgLabel,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "ctx_f(",
                .suffix = ": 1)",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsCall(IsIdentifier("ctx_f"), {{SpecAdder::get_id(v), IsNumber("1")}}));
                }
            }
        };
        return contexts;
    }
}
