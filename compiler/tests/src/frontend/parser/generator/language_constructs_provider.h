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
        if (last_non_ws == std::string::npos) return SourceSpan{.line_start = start_line, .column_start = start_col, .line_end = start_line, .column_end = start_col, .file_path = std::make_shared<const std::string>("")};

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
        return SourceSpan{.line_start = start_line, .column_start = start_col, .line_end = end_line, .column_end = end_col, .file_path = std::make_shared<const std::string>("")};
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
                    .name = "func", .prefix = "", .source = "func rec_func() -> void {}\n", .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.function_definitions.begin(), p.function_definitions.end(),
                                               [](auto& f) { return f->name == "rec_func"; });
                        ASSERT_NE(it, p.function_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                    }
                },
                {
                    .name = "struct", .prefix = "", .source = "struct RecStruct {}\n", .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.struct_definitions.begin(), p.struct_definitions.end(),
                                               [](auto& s) { return s->name == "RecStruct"; });
                        ASSERT_NE(it, p.struct_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                    }
                },
                {
                    .name = "enum", .prefix = "", .source = "enum RecEnum: int {}\n", .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.enum_definitions.begin(), p.enum_definitions.end(),
                                               [](auto& e) { return e->name == "RecEnum"; });
                        ASSERT_NE(it, p.enum_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                    }
                },
                {
                    .name = "typealias", .prefix = "", .source = "typealias RecAlias = int\n", .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.type_aliases.begin(), p.type_aliases.end(),
                                               [](auto& a) { return a->name == "RecAlias"; });
                        ASSERT_NE(it, p.type_aliases.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                    }
                },
                {
                    .name = "import", .prefix = "", .source = "import \"lib\"\n", .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        ASSERT_FALSE(p.import_statements.empty());
                        EXPECT_EQ(p.import_statements[0]->modifiers.size(), m);
                    }
                },
                {
                    .name = "directive_no_value", .prefix = "", .source = "#dir\n", .supports_modifiers = false, .is_top_level_only = true,
                    .verify = [](const Program& p, int) { EXPECT_FALSE(p.directives.empty()); }
                },
                {
                    .name = "directive_value_equals", .prefix = "", .source = "#dir = directive_value\n", .supports_modifiers = false, .is_top_level_only = true,
                    .verify = [](const Program& p, int) { EXPECT_FALSE(p.directives.empty()); }
                },
                {
                    .name = "directive_value_no_equals", .prefix = "", .source = "#dir directive_value\n", .supports_modifiers = false, .is_top_level_only = true,
                    .verify = [](const Program& p, int) { EXPECT_FALSE(p.directives.empty()); }
                },
                {
                    .name = "func_multi", .prefix = "", .source = "func rec_func_multi() -> void {\n    let x = 1\n}\n", .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.function_definitions.begin(), p.function_definitions.end(),
                                               [](auto& f) { return f->name == "rec_func_multi"; });
                        ASSERT_NE(it, p.function_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                        EXPECT_EQ((*it)->body.size(), 1);
                    }
                },
                {
                    .name = "struct_multi", .prefix = "", .source = "struct RecStructMulti {\n    x: int,\n    y: float\n}\n", .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.struct_definitions.begin(), p.struct_definitions.end(),
                                               [](auto& s) { return s->name == "RecStructMulti"; });
                        ASSERT_NE(it, p.struct_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                        EXPECT_EQ((*it)->fields.size(), 2);
                    }
                },
                {
                    .name = "enum_multi", .prefix = "", .source = "enum RecEnumMulti: int {\n    A,\n    B\n}\n", .supports_modifiers = true, .is_top_level_only = true,
                    .verify = [](const Program& p, int m)
                    {
                        auto it = std::find_if(p.enum_definitions.begin(), p.enum_definitions.end(),
                                               [](auto& e) { return e->name == "RecEnumMulti"; });
                        ASSERT_NE(it, p.enum_definitions.end());
                        EXPECT_EQ((*it)->modifiers.size(), m);
                        EXPECT_EQ((*it)->cases.size(), 2);
                    }
                },
                {
                    .name = "let", .prefix = "let ", .source = "rec_let = 1\n", .supports_modifiers = true, .is_top_level_only = false,
                    .verify = [](const Program& p, int m)
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
                    .name = "call", .prefix = "", .source = "rec_call()\n", .supports_modifiers = false, .is_top_level_only = false,
                    .verify = [](const Program& p, int)
                    {
                        EXPECT_NE(find_statement(p,[](const Statement* st) {
                                      auto e = dynamic_cast<const ExpressionStatement*>(st);
                                      return e && dynamic_cast<const FunctionCall*>(e->expr.get());
                                      }), nullptr);
                    }
                },
                {
                    .name = "reassign_id", .prefix = "", .source = "rec_x = 1\n", .supports_modifiers = false, .is_top_level_only = false,
                    .verify = [](const Program& p, int) { verify_reassignment<IdentifierAccess, void>(p); }
                },
                {
                    .name = "self_dot", .prefix = "", .source = "self.prop = 1\n", .supports_modifiers = false, .is_top_level_only = false,
                    .verify = [](const Program& p, int) { verify_reassignment<DotAccess, SelfExpression>(p); }
                },
                {
                    .name = "self_bracket", .prefix = "", .source = "self[0] = 1\n", .supports_modifiers = false, .is_top_level_only = false,
                    .verify = [](const Program& p, int) { verify_reassignment<BracketAccess, SelfExpression>(p); }
                },
                {
                    .name = "id_dot", .prefix = "", .source = "obj.prop = 1\n", .supports_modifiers = false, .is_top_level_only = false,
                    .verify = [](const Program& p, int) { verify_reassignment<DotAccess, IdentifierAccess>(p); }
                },
                {
                    .name = "id_bracket", .prefix = "", .source = "arr[0] = 1\n", .supports_modifiers = false, .is_top_level_only = false,
                    .verify = [](const Program& p, int) { verify_reassignment<BracketAccess, IdentifierAccess>(p); }
                }
            };
        }

        static std::vector<LanguageConstructDefinition> get_raw_broken_definitions()
        {
            auto no_op = [](const Program&, int)
            {
            };
            return {
                {.name = "func_broken", .prefix = "", .source = "func b() -> void {\n    let a = *\n}\n", .supports_modifiers = true, .is_top_level_only = true, .verify = no_op},
                {.name = "struct_broken", .prefix = "", .source = "struct A {\n    a\n}\n", .supports_modifiers = true, .is_top_level_only = true, .verify = no_op},
                {.name = "enum_broken", .prefix = "", .source = "enum A {\n    *,\n    B\n}\n", .supports_modifiers = true, .is_top_level_only = true, .verify = no_op},
                {.name = "typealias_broken", .prefix = "", .source = "typealias A = *\n", .supports_modifiers = true, .is_top_level_only = true, .verify = no_op},
                {.name = "import_broken", .prefix = "", .source = "import *\n", .supports_modifiers = true, .is_top_level_only = true, .verify = no_op},
                {.name = "directive_broken_no_name", .prefix = "", .source = "#*\n", .supports_modifiers = false, .is_top_level_only = true, .verify = no_op},
                {.name = "directive_broken_value", .prefix = "", .source = "#dir = *\n", .supports_modifiers = false, .is_top_level_only = true, .verify = no_op}
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
                        .name = core.name + "_mod1", .source = core.build(mod_samples[0]), .is_top_level_only = core.is_top_level_only,
                        .verify = [=](const Program& p) { core.verify(p, 1); }
                    });
                    variants.push_back({
                        .name = core.name + "_mod2", .source = core.build(mod_samples[0] + mod_samples[1]), .is_top_level_only = core.is_top_level_only,
                        .verify = [=](const Program& p) { core.verify(p, 2); }
                    });
                    variants.push_back({
                        .name = core.name + "_mod3", .source = core.build(mod_samples[0] + mod_samples[1] + mod_samples[2]),
                        .is_top_level_only = core.is_top_level_only, .verify = [=](const Program& p) { core.verify(p, 3); }
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
                    .name = core.name + "_mod0", .source = core.build(), .is_top_level_only = core.is_top_level_only,
                    .verify = [=](const Program& p) { core.verify(p, 0); }
                });

                if (core.supports_modifiers)
                {
                    variants.push_back({
                        .name = core.name + "_mod1", .source = core.build(mod_samples[0]), .is_top_level_only = core.is_top_level_only,
                        .verify = [=](const Program& p) { core.verify(p, 1); }
                    });
                    variants.push_back({
                        .name = core.name + "_mod2", .source = core.build(mod_samples[0] + mod_samples[1]), .is_top_level_only = core.is_top_level_only,
                        .verify = [=](const Program& p) { core.verify(p, 2); }
                    });
                    variants.push_back({
                        .name = core.name + "_mod3", .source = core.build(mod_samples[0] + mod_samples[1] + mod_samples[2]),
                        .is_top_level_only = core.is_top_level_only, .verify = [=](const Program& p) { core.verify(p, 3); }
                    });
                }
            }
            return variants;
        }
    };
}
