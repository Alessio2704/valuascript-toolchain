#include "context_registry.h"
#include "spec_adder.h"
#include "context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_modifier_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                ContextNames::ModBeforeLetSingle, {InjectableType::Modifier}, InjectableType::StrongStatement, "",
                " let ctx_assign = 1\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsAssignment(
                            {
                                {SpecAdder::get_v<ModifierVerifier>(v), "ctx_assign"}
                            },
                            IsNumber("1")
                        )
                    );
                }
            },
            {
                ContextNames::ModBeforeLetMultiple, {InjectableType::Modifier}, InjectableType::StrongStatement, "",
                " let ctx_a, ctx_b = 1\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsAssignment(
                            {
                                AssignmentTargetSpec(
                                    SpecAdder::get_v<ModifierVerifier>(v), "ctx_a"),
                                AssignmentTargetSpec(
                                    SpecAdder::get_v<ModifierVerifier>(v), "ctx_b")
                            }, IsNumber("1")
                        )
                    );
                }
            },
            {
                ContextNames::ModBeforeLetMultipleWithInner, {InjectableType::Modifier},
                InjectableType::StrongStatement,
                "",
                " let ctx_a, @test ctx_b = 1\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    auto mods_a = SpecAdder::get_v<ModifierVerifier>(v);
                    auto mods_b = mods_a;
                    mods_b.push_back({"test"});

                    return UniversalVerifier(
                        IsAssignment(
                            {
                                AssignmentTargetSpec(mods_a, "ctx_a"),
                                AssignmentTargetSpec(mods_b, "ctx_b")
                            }, IsNumber("1")
                        )
                    );
                }
            },
            {
                ContextNames::ModBeforeLetMultipleWithBothInner, {InjectableType::Modifier},
                InjectableType::StrongStatement, "",
                " let @test1 ctx_a, @test2 ctx_b = 1\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    auto injected_mods = SpecAdder::get_v<ModifierVerifier>(v);

                    auto mods_a = injected_mods;
                    mods_a.push_back({"test1"});

                    auto mods_b = injected_mods;
                    mods_b.push_back({"test2"});

                    return UniversalVerifier(
                        IsAssignment(
                            {
                                AssignmentTargetSpec(mods_a, "ctx_a"),
                                AssignmentTargetSpec(mods_b, "ctx_b")
                            }, IsNumber("1")
                        )
                    );
                }
            },
            {
                ContextNames::ModAssignment, {InjectableType::Modifier}, InjectableType::StrongStatement, "let ",
                " ctx_assign = 1\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({{SpecAdder::get_v<ModifierVerifier>(v), "ctx_assign"}},
                                                          IsNumber("1")));
                }
            },
            {
                ContextNames::ModMultiAssignment, {InjectableType::Modifier}, InjectableType::StrongStatement,
                "let ctx_a, ",
                " ctx_b = 1\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({
                                                              AssignmentTargetSpec("ctx_a"),
                                                              AssignmentTargetSpec(
                                                                  SpecAdder::get_v<ModifierVerifier>(v), "ctx_b")
                                                          }, IsNumber("1")));
                }
            },
            {
                ContextNames::ModImportStatement, {InjectableType::Modifier}, InjectableType::TopLevel, "",
                " import \"ctx_lib.vs\"\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsImport("\"ctx_lib.vs\"", SpecAdder::get_v<ModifierVerifier>(v)));
                }
            },
            {
                ContextNames::ModReturnStatement, {InjectableType::Modifier}, InjectableType::WeakStatement, "",
                " return 1\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        ReturnVerifier(IsReturn(SpecAdder::get_v<ModifierVerifier>(v), {IsNumber("1")})));
                }
            },
            {
                ContextNames::ModSwitchCase, {InjectableType::Modifier}, InjectableType::Expression, "switch(1) { ",
                " case A -> 1 }",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsSwitch(IsNumber("1"), {
                                                          SwitchCaseSpec(
                                                              SpecAdder::get_v<ModifierVerifier>(v), {"A"},
                                                              IsNumber("1"))
                                                      }));
                }
            },
            {
                ContextNames::ModSwitchDefault, {InjectableType::Modifier}, InjectableType::Expression, "switch(1) { ",
                " default -> 1 }",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsSwitch(IsNumber("1"), {}, SpecAdder::get_v<ModifierVerifier>(v),
                                                      IsNumber("1")));
                }
            },
            {
                ContextNames::ModFunctionDefinition, {InjectableType::Modifier}, InjectableType::TopLevel, "",
                " func ctx_func() -> void {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func", SpecAdder::get_v<ModifierVerifier>(v), {},
                                                           {IsType("void")}));
                }
            },
            {
                ContextNames::ModFunctionParameter, {InjectableType::Modifier}, InjectableType::TopLevel,
                "func ctx_param(",
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
                ContextNames::ModFunctionParameterWithDefault, {InjectableType::Modifier}, InjectableType::TopLevel,
                "func ctx_param(",
                " p: int = 1) -> void {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_param", {},
                                                           {
                                                               ParamSpec{
                                                                   "p", SpecAdder::get_v<ModifierVerifier>(v),
                                                                   IsType("int"),
                                                                   IsNumber("1")
                                                               }
                                                           }, {IsType("void")}));
                }
            },
            {
                ContextNames::ModStructDefinition, {InjectableType::Modifier}, InjectableType::TopLevel, "",
                " struct ctx_struct {}\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_struct", SpecAdder::get_v<ModifierVerifier>(v), {}));
                }
            },
            {
                ContextNames::ModStructField, {InjectableType::Modifier}, InjectableType::TopLevel,
                "struct ctx_s_field { ",
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
                ContextNames::ModEnumDefinition, {InjectableType::Modifier}, InjectableType::TopLevel, "",
                " enum ctx_enum: int { A = 1 }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_enum", SpecAdder::get_v<ModifierVerifier>(v), IsType("int"),
                                                       {{"A", {}, IsNumber("1")}}));
                }
            },
            {
                ContextNames::ModEnumCase, {InjectableType::Modifier}, InjectableType::TopLevel,
                "enum ctx_e_case: int { ",
                " A = 1 }\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_e_case", {}, IsType("int"), {
                                                           {"A", SpecAdder::get_v<ModifierVerifier>(v), IsNumber("1")}
                                                       }));
                }
            },
            {
                ContextNames::ModTypealiasDefinition, {InjectableType::Modifier}, InjectableType::TopLevel, "",
                " typealias ctx_alias = int\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTypeAlias("ctx_alias", SpecAdder::get_v<ModifierVerifier>(v),
                                                         IsType("int")));
                }
            },
            {
                ContextNames::ModDictItem, {InjectableType::Modifier}, InjectableType::Expression, "{ ", " k: 1 }",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDict({
                        DictItemSpec{"k", SpecAdder::get_v<ModifierVerifier>(v), IsNumber("1")}
                    }));
                }
            }
        };

        return contexts;
    }
}
