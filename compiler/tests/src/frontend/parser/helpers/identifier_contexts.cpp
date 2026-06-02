#include "context_registry.h"
#include "spec_adder.h"
#include "context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_identifier_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                ContextNames::IdLetTarget, {InjectableType::Identifier}, InjectableType::StrongStatement,
                "let ", " = 1\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsAssignment({{SpecAdder::get_id(v)}}, IsNumber("1")));
                }
            },
            {
                ContextNames::IdMultiLetTarget1, {InjectableType::Identifier}, InjectableType::StrongStatement,
                "let ", ", ctx_b = 1\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsAssignment({{SpecAdder::get_id(v)}, {"ctx_b"}}, IsNumber("1")));
                }
            },
            {
                ContextNames::IdMultiLetTarget2, {InjectableType::Identifier}, InjectableType::StrongStatement,
                "let ctx_a, ", " = 1\n",
                [](const UniversalVerifier& v)
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
                ContextNames::IdFuncDefName, {InjectableType::Identifier}, InjectableType::TopLevel,
                "func ", "() -> void {}\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsFunctionDef(SpecAdder::get_id(v), {}, {}, {IsType("void")}));
                }
            },
            {
                ContextNames::IdFuncParamName, {InjectableType::Identifier}, InjectableType::TopLevel,
                "func ctx_f(", ": int) -> void {}\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsFunctionDef("ctx_f", {},
                                                           {ParamSpec{SpecAdder::get_id(v), {}, IsType("int")}},
                                                           {IsType("void")}));
                }
            },
            {
                ContextNames::IdStructDefName, {InjectableType::Identifier}, InjectableType::TopLevel,
                "struct ", " {}\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsStructDef(SpecAdder::get_id(v)));
                }
            },
            {
                ContextNames::IdStructFieldName, {InjectableType::Identifier}, InjectableType::TopLevel,
                "struct ctx_s { ", ": int }\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsStructDef("ctx_s", {}, {{SpecAdder::get_id(v), {}, IsType("int")}}));
                }
            },
            {
                ContextNames::IdEnumDefName, {InjectableType::Identifier}, InjectableType::TopLevel,
                "enum ", ": int { A = 1 }\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef(SpecAdder::get_id(v), {}, IsType("int"),
                                                       {{"A", {}, IsNumber("1")}}));
                }
            },
            {
                ContextNames::IdEnumCaseName, {InjectableType::Identifier}, InjectableType::TopLevel,
                "enum ctx_e: int { ", " = 1 }\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef("ctx_e", {}, IsType("int"), {
                                                           {SpecAdder::get_id(v), {}, IsNumber("1")}
                                                       }));
                }
            },
            {
                ContextNames::IdEnumCaseNameMulti1, {InjectableType::Identifier}, InjectableType::TopLevel,
                "enum ctx_e: int { ", " = 1, B = 2, C = 3 }\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef("ctx_e", {}, IsType("int"), {
                                                           {SpecAdder::get_id(v), {}, IsNumber("1")},
                                                           {"B", {}, IsNumber("2")},
                                                           {"C", {}, IsNumber("3")}
                                                       }));
                }
            },
            {
                ContextNames::IdEnumCaseNameMulti2, {InjectableType::Identifier}, InjectableType::TopLevel,
                "enum ctx_e: int { A = 1, ", " = 2, C = 3 }\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef("ctx_e", {}, IsType("int"), {
                                                           {"A", {}, IsNumber("1")},
                                                           {SpecAdder::get_id(v), {}, IsNumber("2")},
                                                           {"C", {}, IsNumber("3")}
                                                       }));
                }
            },
            {
                ContextNames::IdEnumCaseNameMulti3, {InjectableType::Identifier}, InjectableType::TopLevel,
                "enum ctx_e: int { A = 1, B = 2, ", " = 3 }\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsEnumDef("ctx_e", {}, IsType("int"), {
                                                           {"A", {}, IsNumber("1")},
                                                           {"B", {}, IsNumber("2")},
                                                           {SpecAdder::get_id(v), {}, IsNumber("3")}
                                                       }));
                }
            },
            {
                ContextNames::IdTypeAliasName, {InjectableType::Identifier}, InjectableType::TopLevel,
                "typealias ", " = int\n",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsTypeAlias(SpecAdder::get_id(v), {}, IsType("int")));
                }
            },
            {
                ContextNames::IdTypeAnnotation, {InjectableType::Identifier}, InjectableType::TypeAnnotation,
                "", "",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsType(SpecAdder::get_id(v)));
                }
            },
            {
                ContextNames::IdDotAccessProperty, {InjectableType::Identifier}, InjectableType::Expression,
                "(ctx_obj).", "",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsDot(IsGrouping(IsIdentifier("ctx_obj")), SpecAdder::get_id(v)));
                }
            },
            {
                ContextNames::IdDictKey, {InjectableType::Identifier}, InjectableType::Expression,
                "{ ", ": 1 }",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsDict({{SpecAdder::get_id(v), {}, IsNumber("1")}}));
                }
            },
            {
                ContextNames::IdSwitchCaseLabel, {InjectableType::Identifier}, InjectableType::Expression,
                "switch(1) { case ", " -> 1 }",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            {SwitchCaseSpec{{SpecAdder::get_id(v)}, IsNumber("1")}}
                        )
                    );
                }
            },
            {
                ContextNames::IdSwitchCaseLabelMulti1, {InjectableType::Identifier}, InjectableType::Expression,
                "switch(1) { case ", ", B, C -> 1 }",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            {SwitchCaseSpec{{SpecAdder::get_id(v), "B", "C"}, IsNumber("1")}}
                        )
                    );
                }
            },
            {
                ContextNames::IdSwitchCaseLabelMulti2, {InjectableType::Identifier}, InjectableType::Expression,
                "switch(1) { case A, ", ", C -> 1 }",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            {SwitchCaseSpec{{"A", SpecAdder::get_id(v), "C"}, IsNumber("1")}}
                        )
                    );
                }
            },
            {
                ContextNames::IdSwitchCaseLabelMulti3, {InjectableType::Identifier}, InjectableType::Expression,
                "switch(1) { case A, B, ", " -> 1 }",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(
                        IsSwitch(
                            IsNumber("1"),
                            {SwitchCaseSpec{{"A", "B", SpecAdder::get_id(v)}, IsNumber("1")}}
                        )
                    );
                }
            },
            {
                ContextNames::IdModifierName, {InjectableType::Identifier}, InjectableType::Modifier,
                "@", "() ",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(std::vector<ModifierSpec>{{SpecAdder::get_id(v), {}}});
                }
            },
            {
                ContextNames::IdModifierArgLabel, {InjectableType::Identifier}, InjectableType::Modifier,
                "@ctx_mod(", ": 1) ",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(std::vector<ModifierSpec>{
                        {"ctx_mod", {{SpecAdder::get_id(v), IsNumber("1")}}}
                    });
                }
            },
            {
                ContextNames::IdCallArgLabel, {InjectableType::Identifier}, InjectableType::Expression,
                "ctx_f(", ": 1)",
                [](const UniversalVerifier& v)
                {
                    return UniversalVerifier(IsCall(IsIdentifier("ctx_f"), {{SpecAdder::get_id(v), IsNumber("1")}}));
                }
            }
        };
        return contexts;
    }
}
