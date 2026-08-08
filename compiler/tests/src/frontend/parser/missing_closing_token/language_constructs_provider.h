#pragma once
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include "frontend/parser/ast.h"
#include "frontend/parser/helpers/node_matchers.h"

namespace valuascript::compiler::test
{
    inline std::vector<ModifierSpec> get_expected_modifier_specs(int m)
    {
        static const std::vector<ModifierSpec> all_specs = {
            {.name = "simple"},
            {.name = "param", .args = {{.label = "id", .value_v = IsNumber("100")}}},
            {
                .name = "complex", .args = {
                    {.label = "a", .value_v = IsNumber("1")},
                    {.label = "b", .value_v = IsString("\"str\"")},
                    {.label = "c", .value_v = IsTensor(IsNumber("1"), IsNumber("2"))}
                }
            }
        };
        return {all_specs.begin(), all_specs.begin() + m};
    }

    template <typename Container, typename Verifier>
    void verify_last_node(const Container& container, const Verifier& verifier)
    {
        ASSERT_FALSE(container.empty()) << "Expected non-empty AST container.";
        verifier(container.back().get());
    }

    inline Statement* get_follower_statement(const Program& p)
    {
        if (!p.execution_steps.empty()) return p.execution_steps.back().get();
        if (!p.function_definitions.empty() && !p.function_definitions[0]->body.empty())
        {
            return p.function_definitions[0]->body.back().get();
        }
        return nullptr;
    }

    struct LanguageConstructDefinition
    {
        std::string name;
        std::string prefix;
        std::string source;
        bool supports_modifiers;
        bool is_top_level_only;
        std::function<void(const Program&, int)> verify;

        [[nodiscard]] std::string build(const std::string& mods = "") const
        {
            return prefix + mods + source;
        }
    };

    struct TestConstruct
    {
        std::string name;
        std::string source;
        bool is_top_level_only;
        std::function<void(const Program&)> verify;
    };

    class LanguageConstructsProvider
    {
    public:
        static std::vector<std::string> get_mod_samples()
        {
            static const std::vector<std::string> mod_samples{
                "@simple ",
                "@param(id: 100) ",
                "@complex(a: 1, b: \"str\", c: [1, 2]) "
            };

            return mod_samples;
        }

        static std::vector<LanguageConstructDefinition> get_raw_definitions()
        {
            return {
                {
                    .name = "func", .prefix = "", .source = "func rec_func() -> void {}\n", .supports_modifiers = true,
                    .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        verify_last_node(p.function_definitions,
                                         IsFunctionDef("rec_func", get_expected_modifier_specs(m), {},
                                                       {IsType("void")}));
                    }
                },
                {
                    .name = "struct", .prefix = "", .source = "struct RecStruct {}\n", .supports_modifiers = true,
                    .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        verify_last_node(p.struct_definitions,
                                         IsStructDef("RecStruct", get_expected_modifier_specs(m)));
                    }
                },
                {
                    .name = "enum", .prefix = "", .source = "enum RecEnum: int {}\n", .supports_modifiers = true,
                    .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        verify_last_node(p.enum_definitions,
                                         IsEnumDef("RecEnum", get_expected_modifier_specs(m), IsType("int")));
                    }
                },
                {
                    .name = "typealias", .prefix = "", .source = "typealias RecAlias = int\n",
                    .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        verify_last_node(p.type_aliases,
                                         IsTypeAlias("RecAlias", get_expected_modifier_specs(m), IsType("int")));
                    }
                },
                {
                    .name = "import", .prefix = "", .source = "import \"lib\"\n", .supports_modifiers = true,
                    .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        verify_last_node(p.import_statements, IsImport("\"lib\"", get_expected_modifier_specs(m)));
                    }
                },
                {
                    .name = "directive_no_value", .prefix = "", .source = "#dir\n", .supports_modifiers = false,
                    .is_top_level_only = true,
                    .verify = [](const Program& p, int)
                    {
                        verify_last_node(p.directives, IsDirective("dir"));
                    }
                },
                {
                    .name = "directive_value_equals", .prefix = "", .source = "#dir = directive_value\n",
                    .supports_modifiers = false, .is_top_level_only = true,
                    .verify = [](const Program& p, int)
                    {
                        verify_last_node(p.directives, IsDirective("dir", IsIdentifier("directive_value")));
                    }
                },
                {
                    .name = "directive_value_no_equals", .prefix = "", .source = "#dir directive_value\n",
                    .supports_modifiers = false, .is_top_level_only = true,
                    .verify = [](const Program& p, int)
                    {
                        verify_last_node(p.directives, IsDirective("dir", IsIdentifier("directive_value")));
                    }
                },
                {
                    .name = "func_multi", .prefix = "", .source = "func rec_func_multi() -> void {\n    let x = 1\n}\n",
                    .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        verify_last_node(p.function_definitions,
                                         IsFunctionDef("rec_func_multi", get_expected_modifier_specs(m), {},
                                                       {IsType("void")}, {
                                                           IsAssignment({{.name = "x"}}, IsNumber("1"))
                                                       }));
                    }
                },
                {
                    .name = "struct_multi", .prefix = "",
                    .source = "struct RecStructMulti {\n    x: int,\n    y: float\n}\n", .supports_modifiers = true,
                    .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        verify_last_node(p.struct_definitions,
                                         IsStructDef("RecStructMulti", get_expected_modifier_specs(m),
                                                     std::vector<FieldSpec>{
                                                         FieldSpec{.name = "x", .type_v = IsType("int")},
                                                         FieldSpec{.name = "y", .type_v = IsType("float")}
                                                     }));
                    }
                },
                {
                    .name = "enum_multi", .prefix = "", .source = "enum RecEnumMulti: int {\n    A,\n    B\n}\n",
                    .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        verify_last_node(p.enum_definitions,
                                         IsEnumDef("RecEnumMulti", get_expected_modifier_specs(m), IsType("int"),
                                                   std::vector<EnumCaseSpec>{
                                                       EnumCaseSpec{.name = "A"}, EnumCaseSpec{.name = "B"}
                                                   }));
                    }
                },
                {
                    .name = "let", .prefix = "let ", .source = "rec_let = 1\n", .supports_modifiers = true,
                    .is_top_level_only = false,
                    .verify = [](const Program& p, int m)
                    {
                        StmtVerifier(IsAssignment({{.modifiers = get_expected_modifier_specs(m), .name = "rec_let"}},
                                                  IsNumber("1")))(get_follower_statement(p));
                    }
                },
                {
                    .name = "call", .prefix = "", .source = "rec_call()\n", .supports_modifiers = false,
                    .is_top_level_only = false,
                    .verify = [](const Program& p, int)
                    {
                        StmtVerifier(IsExprStmt(IsCall(IsIdentifier("rec_call"))))(get_follower_statement(p));
                    }
                },
                {
                    .name = "reassign_id", .prefix = "", .source = "rec_x = 1\n", .supports_modifiers = false,
                    .is_top_level_only = false,
                    .verify = [](const Program& p, int)
                    {
                        StmtVerifier(IsReassignment(IsIdentifier("rec_x"), IsNumber("1")))(get_follower_statement(p));
                    }
                },
                {
                    .name = "self_dot", .prefix = "", .source = "self.prop = 1\n", .supports_modifiers = false,
                    .is_top_level_only = false,
                    .verify = [](const Program& p, int)
                    {
                        StmtVerifier(IsReassignment(IsDot(IsSelf(), "prop"), IsNumber("1")))(get_follower_statement(p));
                    }
                },
                {
                    .name = "self_bracket", .prefix = "", .source = "self[0] = 1\n", .supports_modifiers = false,
                    .is_top_level_only = false,
                    .verify = [](const Program& p, int)
                    {
                        StmtVerifier(IsReassignment(IsBracket(IsSelf(), IsNumber("0")), IsNumber("1")))(
                            get_follower_statement(p));
                    }
                },
                {
                    .name = "id_dot", .prefix = "", .source = "obj.prop = 1\n", .supports_modifiers = false,
                    .is_top_level_only = false,
                    .verify = [](const Program& p, int)
                    {
                        StmtVerifier(IsReassignment(IsDot(IsIdentifier("obj"), "prop"), IsNumber("1")))(
                            get_follower_statement(p));
                    }
                },
                {
                    .name = "id_bracket", .prefix = "", .source = "arr[0] = 1\n", .supports_modifiers = false,
                    .is_top_level_only = false,
                    .verify = [](const Program& p, int)
                    {
                        StmtVerifier(IsReassignment(IsBracket(IsIdentifier("arr"), IsNumber("0")), IsNumber("1")))(
                            get_follower_statement(p));
                    }
                }
            };
        }

        static std::vector<LanguageConstructDefinition> get_raw_broken_definitions()
        {
            auto no_op = [](const Program&, int)
            {
            };
            return {
                {
                    .name = "func_broken", .prefix = "", .source = "func b() -> void {\n    let a = *\n}\n",
                    .supports_modifiers = true, .is_top_level_only = true, .verify = no_op
                },
                {
                    .name = "struct_broken", .prefix = "", .source = "struct A {\n    a\n}\n",
                    .supports_modifiers = true, .is_top_level_only = true, .verify = no_op
                },
                {
                    .name = "enum_broken", .prefix = "", .source = "enum A {\n    *,\n    B\n}\n",
                    .supports_modifiers = true, .is_top_level_only = true, .verify = no_op
                },
                {
                    .name = "typealias_broken", .prefix = "", .source = "typealias A = *\n", .supports_modifiers = true,
                    .is_top_level_only = true, .verify = no_op
                },
                {
                    .name = "import_broken", .prefix = "", .source = "import *\n", .supports_modifiers = true,
                    .is_top_level_only = true, .verify = no_op
                },
                {
                    .name = "directive_broken_no_name", .prefix = "", .source = "#*\n", .supports_modifiers = false,
                    .is_top_level_only = true, .verify = no_op
                },
                {
                    .name = "directive_broken_value", .prefix = "", .source = "#dir = *\n", .supports_modifiers = false,
                    .is_top_level_only = true, .verify = no_op
                }
            };
        }

        static std::vector<TestConstruct> build_all_test_variants()
        {
            std::vector<TestConstruct> variants;
            std::vector<std::string> mod_samples = get_mod_samples();

            for (const auto& core : get_raw_definitions())
            {
                variants.push_back({
                    .name = core.name + "_mod0", .source = core.build(), .is_top_level_only = core.is_top_level_only,
                    .verify = [=](const Program& p) { core.verify(p, 0); }
                });

                if (core.supports_modifiers)
                {
                    variants.push_back({
                        .name = core.name + "_mod1", .source = core.build(mod_samples[0]),
                        .is_top_level_only = core.is_top_level_only,
                        .verify = [=](const Program& p) { core.verify(p, 1); }
                    });
                    variants.push_back({
                        .name = core.name + "_mod2", .source = core.build(mod_samples[0] + mod_samples[1]),
                        .is_top_level_only = core.is_top_level_only,
                        .verify = [=](const Program& p) { core.verify(p, 2); }
                    });
                    variants.push_back({
                        .name = core.name + "_mod3",
                        .source = core.build(mod_samples[0] + mod_samples[1] + mod_samples[2]),
                        .is_top_level_only = core.is_top_level_only,
                        .verify = [=](const Program& p) { core.verify(p, 3); }
                    });
                }
            }
            return variants;
        }
    };
}
