#include "context_registry.h"
#include "spec_adder.h"
#include "context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_modifier_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                .name = ContextNames::ModBeforeLetSingle,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "",
                .suffix = " let ctx_assign = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
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
                .name = ContextNames::ModBeforeLetMultiple,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "",
                .suffix = " let ctx_a, ctx_b = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
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
                .name = ContextNames::ModBeforeLetMultipleWithInner,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "",
                .suffix = " let ctx_a, @test ctx_b = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
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
                .name = ContextNames::ModBeforeLetMultipleWithBothInner,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "",
                .suffix = " let @test1 ctx_a, @test2 ctx_b = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
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
                .name = ContextNames::ModAssignment,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ",
                .suffix = " ctx_assign = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({{SpecAdder::get_v<ModifierVerifier>(v), "ctx_assign"}},
                                                          IsNumber("1")));
                }
            },
            {
                .name = ContextNames::ModMultiAssignment,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ctx_a, ",
                .suffix = " ctx_b = 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({
                                                              AssignmentTargetSpec("ctx_a"),
                                                              AssignmentTargetSpec(
                                                                  SpecAdder::get_v<ModifierVerifier>(v), "ctx_b")
                                                          }, IsNumber("1")));
                }
            },
            {
                .name = ContextNames::ModImportStatement,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "",
                .suffix = " import \"ctx_lib.vs\"\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsImport("\"ctx_lib.vs\"", SpecAdder::get_v<ModifierVerifier>(v)));
                }
            },
            {
                .name = ContextNames::ModReturnStatement,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::WeakStatement,
                .prefix = "",
                .suffix = " return 1\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        ReturnVerifier(IsReturn(SpecAdder::get_v<ModifierVerifier>(v), {IsNumber("1")})));
                }
            },
            {
                .name = ContextNames::ModSwitchCase,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { ",
                .suffix = " case A -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsSwitch(IsNumber("1"),
                                                          SwitchCaseSpec(
                                                              SpecAdder::get_v<ModifierVerifier>(v), "A",
                                                              IsNumber("1"))
                                                      ));
                }
            },
            {
                .name = ContextNames::ModSwitchDefault,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::Expression,
                .prefix = "switch(1) { ",
                .suffix = " default -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsSwitch(IsNumber("1"), {}, SpecAdder::get_v<ModifierVerifier>(v),
                                                      IsNumber("1")));
                }
            },
            {
                .name = ContextNames::ModFunctionDefinition,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "",
                .suffix = " func ctx_func() -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func", SpecAdder::get_v<ModifierVerifier>(v), {},
                                                           {IsType("void")}));
                }
            },
            {
                .name = ContextNames::ModFunctionParameter,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_param(",
                .suffix = " p: int) -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
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
                .name = ContextNames::ModFunctionParameterWithDefault,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_param(",
                .suffix = " p: int = 1) -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
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
                .name = ContextNames::ModStructDefinition,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "",
                .suffix = " struct ctx_struct {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_struct", SpecAdder::get_v<ModifierVerifier>(v), {}));
                }
            },
            {
                .name = ContextNames::ModStructField,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "struct ctx_s_field { ",
                .suffix = " f: int }\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsStructDef("ctx_s_field",
                                                             FieldSpec{
                                                                 "f", SpecAdder::get_v<ModifierVerifier>(v),
                                                                 IsType("int")
                                                             }
                                                         ));
                }
            },
            {
                .name = ContextNames::ModEnumDefinition,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "",
                .suffix = " enum ctx_enum: int { A = 1 }\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_enum", SpecAdder::get_v<ModifierVerifier>(v), IsType("int"),
                                                       EnumCaseSpec{"A", IsNumber("1")}));
                }
            },
            {
                .name = ContextNames::ModEnumCase,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "enum ctx_e_case: int { ",
                .suffix = " A = 1 }\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("ctx_e_case", {}, IsType("int"),
                                                           EnumCaseSpec{"A", SpecAdder::get_v<ModifierVerifier>(v), IsNumber("1")}
                                                       ));
                }
            },
            {
                .name = ContextNames::ModTypealiasDefinition,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "",
                .suffix = " typealias ctx_alias = int\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTypeAlias("ctx_alias", SpecAdder::get_v<ModifierVerifier>(v),
                                                         IsType("int")));
                }
            },
            {
                .name = ContextNames::ModExtensionDefinition,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::TopLevel,
                .prefix = "",
                .suffix = " extension ctx_target {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsExtensionDef(SpecAdder::get_v<ModifierVerifier>(v), IsType("ctx_target")));
                }
            },
            {
                .name = ContextNames::ModDictItem,
                .input_types = {InjectableType::Modifier},
                .output_type = InjectableType::Expression,
                .prefix = "{ ",
                .suffix = " k: 1 }",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDict(
                        DictItemSpec{"k", SpecAdder::get_v<ModifierVerifier>(v), IsNumber("1")}
                    ));
                }
            }
        };

        return contexts;
    }
}
