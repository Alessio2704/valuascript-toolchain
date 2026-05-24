#pragma once
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include "frontend/parser/ast.h"

namespace valuascript::compiler::test
{
    inline SourceSpan calculate_expected_span(const std::string& code, size_t start_line, size_t start_col)
    {
        size_t last_non_ws = code.find_last_not_of(" \t\r\n");
        if (last_non_ws == std::string::npos) return {start_line, start_col, start_line, start_col, ""};

        size_t current_line = start_line;
        size_t current_col = start_col;
        size_t end_line = start_line;
        size_t end_col = start_col;

        for (size_t i = 0; i <= last_non_ws; ++i)
        {
            if (i == last_non_ws)
            {
                end_line = current_line;
                end_col = current_col + 1;
            }
            if (code[i] == '\n')
            {
                current_line++;
                current_col = 1;
            }
            else
            {
                current_col++;
            }
        }
        return {start_line, start_col, end_line, end_col, ""};
    }

    template <typename Predicate>
    const Statement* find_statement(const Program& program, Predicate pred)
    {
        for (const auto& stmt : program.execution_steps)
        {
            if (pred(stmt.get())) return stmt.get();
        }
        for (const auto& func_def : program.function_definitions)
        {
            for (const auto& stmt : func_def->body)
            {
                if (pred(stmt.get())) return stmt.get();
            }
        }
        return nullptr;
    }

    template <typename TargetType, typename ReceiverType>
    void verify_reassignment(const Program& program)
    {
        auto target_statement = find_statement(program, [](const Statement* stmt)
        {
            auto reassignment = dynamic_cast<const Reassignment*>(stmt);
            auto target = reassignment ? dynamic_cast<TargetType*>(reassignment->target.get()) : nullptr;
            if constexpr (std::is_same_v<TargetType, IdentifierAccess>)
            {
                return target != nullptr;
            }
            else
            {
                return target && dynamic_cast<ReceiverType*>(target->target.get());
            }
        });
        EXPECT_NE(target_statement, nullptr) << "Failed to find reassignment in AST.";
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
                    "func", "", "func rec_func() -> void {}\n", true, true,
                    [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.function_definitions.begin(), p.function_definitions.end(),
                                               [](auto& f) { return f->name == "rec_func"; });
                        ASSERT_NE(it, p.function_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                    }
                },
                {
                    "struct", "", "struct RecStruct {}\n", true, true,
                    [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.struct_definitions.begin(), p.struct_definitions.end(),
                                               [](auto& s) { return s->name == "RecStruct"; });
                        ASSERT_NE(it, p.struct_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                    }
                },
                {
                    "enum", "", "enum RecEnum: int {}\n", true, true,
                    [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.enum_definitions.begin(), p.enum_definitions.end(),
                                               [](auto& e) { return e->name == "RecEnum"; });
                        ASSERT_NE(it, p.enum_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                    }
                },
                {
                    "typealias", "", "typealias RecAlias = int\n", true, true,
                    [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.type_aliases.begin(), p.type_aliases.end(),
                                               [](auto& a) { return a->name == "RecAlias"; });
                        ASSERT_NE(it, p.type_aliases.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                    }
                },
                {
                    "import", "", "import \"lib\"\n", true, true,
                    [](const Program& p, int m)
                    {
                        ASSERT_FALSE(p.import_statements.empty());
                        EXPECT_EQ(p.import_statements[0]->modifiers.size(), m);
                    }
                },
                {
                    "directive_no_value", "", "#dir\n", false, true,
                    [](const Program& p, int) { EXPECT_FALSE(p.directives.empty()); }
                },
                {
                    "directive_value_equals", "", "#dir = directive_value\n", false, true,
                    [](const Program& p, int) { EXPECT_FALSE(p.directives.empty()); }
                },
                {
                    "directive_value_no_equals", "", "#dir directive_value\n", false, true,
                    [](const Program& p, int) { EXPECT_FALSE(p.directives.empty()); }
                },
                {
                    "func_multi", "", "func rec_func_multi() -> void {\n    let x = 1\n}\n", true, true,
                    [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.function_definitions.begin(), p.function_definitions.end(),
                                               [](auto& f) { return f->name == "rec_func_multi"; });
                        ASSERT_NE(it, p.function_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                        EXPECT_EQ((*it)->body.size(), 1);
                    }
                },
                {
                    "struct_multi", "", "struct RecStructMulti {\n    x: int,\n    y: float\n}\n", true, true,
                    [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.struct_definitions.begin(), p.struct_definitions.end(),
                                               [](auto& s) { return s->name == "RecStructMulti"; });
                        ASSERT_NE(it, p.struct_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                        EXPECT_EQ((*it)->fields.size(), 2);
                    }
                },
                {
                    "enum_multi", "", "enum RecEnumMulti: int {\n    A,\n    B\n}\n", true, true,
                    [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.enum_definitions.begin(), p.enum_definitions.end(),
                                               [](auto& e) { return e->name == "RecEnumMulti"; });
                        ASSERT_NE(it, p.enum_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                        EXPECT_EQ((*it)->cases.size(), 2);
                    }
                },
                {
                    "let", "let ", "rec_let = 1\n", true, false,
                    [](const Program& p, int m)
                    {
                        auto s = find_statement(p, [](const Statement* st)
                        {
                            auto a = dynamic_cast<const Assignment*>(st);
                            return a && !a->targets.empty() && a->targets[0].name == "rec_let";
                        });
                        ASSERT_NE(s, nullptr);
                        EXPECT_EQ(dynamic_cast<const Assignment*>(s)->targets[0].modifiers.size(), m);
                    }
                },
                {
                    "call", "", "rec_call()\n", false, false, [](const Program& p, int)
                    {
                        EXPECT_NE(find_statement(p,[](const Statement* st) {
                                      auto e = dynamic_cast<const ExpressionStatement*>(st);
                                      return e && dynamic_cast<const FunctionCall*>(e->expr.get());
                                      }), nullptr);
                    }
                },
                {
                    "reassign_id", "", "rec_x = 1\n", false, false,
                    [](const Program& p, int) { verify_reassignment<IdentifierAccess, void>(p); }
                },
                {
                    "self_dot", "", "self.prop = 1\n", false, false,
                    [](const Program& p, int) { verify_reassignment<DotAccess, SelfExpression>(p); }
                },
                {
                    "self_bracket", "", "self[0] = 1\n", false, false,
                    [](const Program& p, int) { verify_reassignment<BracketAccess, SelfExpression>(p); }
                },
                {
                    "id_dot", "", "obj.prop = 1\n", false, false,
                    [](const Program& p, int) { verify_reassignment<DotAccess, IdentifierAccess>(p); }
                },
                {
                    "id_bracket", "", "arr[0] = 1\n", false, false,
                    [](const Program& p, int) { verify_reassignment<BracketAccess, IdentifierAccess>(p); }
                }
            };
        }

        static std::vector<LanguageConstructDefinition> get_raw_broken_definitions()
        {
            auto no_op = [](const Program&, int)
            {
            };
            return {
                {"func_broken", "", "func b() -> void {\n    let a = *\n}\n", true, true, no_op},
                {"struct_broken", "", "struct A {\n    a\n}\n", true, true, no_op},
                {"enum_broken", "", "enum A {\n    *,\n    B\n}\n", true, true, no_op},
                {"typealias_broken", "", "typealias A = *\n", true, true, no_op},
                {"import_broken", "", "import *\n", true, true, no_op},
                {"directive_broken_no_name", "", "#*\n", false, true, no_op},
                {"directive_broken_value", "", "#dir = *\n", false, true, no_op}
            };
        }

        static std::vector<TestConstruct> build_all_test_variants()
        {
            std::vector<TestConstruct> variants;
            std::vector<std::string> mod_samples = get_mod_samples();

            for (const auto& core : get_raw_definitions())
            {
                variants.push_back({
                    core.name + "_mod0", core.build(), core.is_top_level_only,
                    [=](const Program& p) { core.verify(p, 0); }
                });

                if (core.supports_modifiers)
                {
                    variants.push_back({
                        core.name + "_mod1", core.build(mod_samples[0]), core.is_top_level_only,
                        [=](const Program& p) { core.verify(p, 1); }
                    });
                    variants.push_back({
                        core.name + "_mod2", core.build(mod_samples[0] + mod_samples[1]), core.is_top_level_only,
                        [=](const Program& p) { core.verify(p, 2); }
                    });
                    variants.push_back({
                        core.name + "_mod3", core.build(mod_samples[0] + mod_samples[1] + mod_samples[2]),
                        core.is_top_level_only, [=](const Program& p) { core.verify(p, 3); }
                    });
                }
            }
            return variants;
        }

        static std::vector<TestConstruct> build_all_broken_test_variants()
        {
            std::vector<TestConstruct> variants;
            std::vector<std::string> mod_samples = get_mod_samples();

            for (const auto& core : get_raw_broken_definitions())
            {
                variants.push_back({
                    core.name + "_mod0", core.build(), core.is_top_level_only,
                    [=](const Program& p) { core.verify(p, 0); }
                });

                if (core.supports_modifiers)
                {
                    variants.push_back({
                        core.name + "_mod1", core.build(mod_samples[0]), core.is_top_level_only,
                        [=](const Program& p) { core.verify(p, 1); }
                    });
                    variants.push_back({
                        core.name + "_mod2", core.build(mod_samples[0] + mod_samples[1]), core.is_top_level_only,
                        [=](const Program& p) { core.verify(p, 2); }
                    });
                    variants.push_back({
                        core.name + "_mod3", core.build(mod_samples[0] + mod_samples[1] + mod_samples[2]),
                        core.is_top_level_only, [=](const Program& p) { core.verify(p, 3); }
                    });
                }
            }
            return variants;
        }
    };
}
