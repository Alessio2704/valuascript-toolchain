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
                    return UniversalVerifier(IsAssignment({{SpecAdder::get_id(v)}}, IsNumber("1")));
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
                    return UniversalVerifier(IsAssignment({{SpecAdder::get_id(v)}, {"ctx_b"}}, IsNumber("1")));
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
                    return UniversalVerifier(IsAssignment({{"ctx_a"}, {SpecAdder::get_id(v)}}, IsNumber("1")));
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
                                                           {ParamSpec{SpecAdder::get_id(v), {}, IsType("int")}},
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
                .name = ContextNames::IdStructFieldName,
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
                .name = ContextNames::IdEnumCaseName,
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
                .name = ContextNames::IdEnumCaseNameMulti1,
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
                .name = ContextNames::IdEnumCaseNameMulti2,
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
                .name = ContextNames::IdEnumCaseNameMulti3,
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
                .name = ContextNames::IdDictKey,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "{ ",
                .suffix = ": 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsDict(DictItemSpec{SpecAdder::get_id(v), {}, IsNumber("1")}));
                }
            },
            {
                .name = ContextNames::IdSwitchCaseLabel,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { case ",
                .suffix = " -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            SwitchCaseSpec(SpecAdder::get_id(v), IsNumber("1"))
                        )
                    );
                }
            },
            {
                .name = ContextNames::IdSwitchCaseLabelMulti1,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { case ",
                .suffix = ", B, C -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            SwitchCaseSpec(SpecAdder::get_id(v), "B", "C", IsNumber("1"))
                        )
                    );
                }
            },
            {
                .name = ContextNames::IdSwitchCaseLabelMulti2,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { case A, ",
                .suffix = ", C -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            SwitchCaseSpec("A", SpecAdder::get_id(v), "C", IsNumber("1"))
                        )
                    );
                }
            },
            {
                .name = ContextNames::IdSwitchCaseLabelMulti3,
                .input_types = {InjectableType::Identifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { case A, B, ",
                .suffix = " -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            SwitchCaseSpec("A", "B", SpecAdder::get_id(v), IsNumber("1"))
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
