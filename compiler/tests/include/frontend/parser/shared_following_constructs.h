#pragma once
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler::test {
    template<typename Predicate>
    const Statement *find_statement(const Program &program, Predicate pred) {
        for (const auto &stmt: program.execution_steps) {
            if (pred(stmt.get())) return stmt.get();
        }
        for (const auto &func_def: program.function_definitions) {
            for (const auto &stmt: func_def->body) {
                if (pred(stmt.get())) return stmt.get();
            }
        }
        return nullptr;
    }

    struct FollowingConstruct {
        std::string name;
        std::string source;
        std::function<void(const Program &)> verify;
    };

    struct CoreConstruct {
        std::string name;
        std::string source;
        bool supports_modifiers;
        std::function<void(const Program &, int)> verify;
    };

    inline void check_mods(const std::vector<Modifier> &mods, int expected_count) {
        EXPECT_EQ(mods.size(), expected_count) << "Modifier count mismatch.";
        if (expected_count > 0)
            EXPECT_EQ(mods[0].name, "simple");
        if (expected_count > 1)
            EXPECT_EQ(mods[1].name, "param");
        if (expected_count > 2)
            EXPECT_EQ(mods[2].name, "complex");
    }

    template<typename TargetType, typename ReceiverType>
    void check_reassign(const Program &program) {
        auto target_statement = find_statement(program, [](const Statement *stmt) {
            auto reassignment = dynamic_cast<const Reassignment *>(stmt);
            auto target = reassignment ? dynamic_cast<TargetType *>(reassignment->target.get()) : nullptr;
            if constexpr (std::is_same_v<TargetType, IdentifierAccess>) {
                return target != nullptr;
            } else {
                return target && dynamic_cast<ReceiverType *>(target->target.get());
            }
        });
        EXPECT_NE(target_statement, nullptr) << "Failed to recover reassignment for target type.";
    }

    inline std::vector<FollowingConstruct> get_all_top_level_following_constructs() {
        std::vector<std::string> mod_samples = {
            "@simple ",
            "@param(id: 100) ",
            "@complex(a: 1, b: \"str\", c: [1, 2]) "
        };

        std::vector<CoreConstruct> cores = {
            {
                "func", "func rec_func() -> void {}\n", true, [](const Program &program, int modifiers_count) {
                    auto it = std::find_if(program.function_definitions.begin(), program.function_definitions.end(),
                                           [](const auto &func_def) { return func_def->name == "rec_func"; });
                    ASSERT_NE(it, program.function_definitions.end());
                    check_mods((*it)->modifiers, modifiers_count);
                }
            },
            {
                "struct", "struct RecStruct {}\n", true, [](const Program &program, int modifiers_count) {
                    auto it = std::find_if(program.struct_definitions.begin(), program.struct_definitions.end(),
                                           [](const auto &struct_def) { return struct_def->name == "RecStruct"; });
                    ASSERT_NE(it, program.struct_definitions.end());
                    check_mods((*it)->modifiers, modifiers_count);
                }
            },
            {
                "enum", "enum RecEnum: int {}\n", true, [](const Program &program, int modifiers_count) {
                    auto it = std::find_if(program.enum_definitions.begin(), program.enum_definitions.end(),
                                           [](const auto &enum_def) { return enum_def->name == "RecEnum"; });
                    ASSERT_NE(it, program.enum_definitions.end());
                    check_mods((*it)->modifiers, modifiers_count);
                }
            },
            {
                "typealias", "typealias RecAlias = int\n", true, [](const Program &program, int modifiers_count) {
                    auto it = std::find_if(program.type_aliases.begin(), program.type_aliases.end(),
                                           [](const auto &type_alias) { return type_alias->name == "RecAlias"; });
                    ASSERT_NE(it, program.type_aliases.end());
                    check_mods((*it)->modifiers, modifiers_count);
                }
            },
            {
                "let", "let rec_let = 1\n", true, [](const Program &program, int modifiers_count) {
                    auto target_stmt = find_statement(program, [](const Statement *stmt) {
                        auto assignment = dynamic_cast<const Assignment *>(stmt);
                        return assignment && !assignment->targets.empty() && assignment->targets[0].first == "rec_let";
                    });
                    ASSERT_NE(target_stmt, nullptr);
                    check_mods(dynamic_cast<const Assignment *>(target_stmt)->modifiers, modifiers_count);
                }
            },
            {
                "var", "var rec_var = 1\n", true, [](const Program &program, int modifiers_count) {
                    auto target_stmt = find_statement(program, [](const Statement *stmt) {
                        auto assignment = dynamic_cast<const Assignment *>(stmt);
                        return assignment && !assignment->targets.empty() && assignment->targets[0].first == "rec_var";
                    });
                    ASSERT_NE(target_stmt, nullptr);
                    check_mods(dynamic_cast<const Assignment *>(target_stmt)->modifiers, modifiers_count);
                }
            },
            {
                "import", "import \"lib\"\n", false, [](const Program &program, int) {
                    EXPECT_FALSE(program.import_statements.empty());
                }
            },
            {
                "directive", "#dir\n", false, [](const Program &program, int) {
                    EXPECT_FALSE(program.directives.empty());
                }
            },
            {
                "call", "rec_call()\n", false, [](const Program &program, int) {
                    auto target_stmt = find_statement(program, [](const Statement *stmt) {
                        auto expr_stmt = dynamic_cast<const ExpressionStatement *>(stmt);
                        return expr_stmt && dynamic_cast<const FunctionCall *>(expr_stmt->expr.get());
                    });
                    EXPECT_NE(target_stmt, nullptr);
                }
            },
            {
                "reassign_id", "rec_x = 1\n", false,
                [](const Program &program, int) { check_reassign<IdentifierAccess, void>(program); }
            },
            {
                "self_dot", "self.prop = 1\n", false,
                [](const Program &program, int) { check_reassign<DotAccess, SelfExpression>(program); }
            },
            {
                "self_bracket", "self[0] = 1\n", false,
                [](const Program &program, int) { check_reassign<BracketAccess, SelfExpression>(program); }
            },
            {
                "id_dot", "obj.prop = 1\n", false,
                [](const Program &program, int) { check_reassign<DotAccess, IdentifierAccess>(program); }
            },
            {
                "id_bracket", "arr[0] = 1\n", false,
                [](const Program &program, int) { check_reassign<BracketAccess, IdentifierAccess>(program); }
            }
        };

        std::vector<FollowingConstruct> universe;
        for (const auto &core_construct: cores) {
            universe.push_back({
                core_construct.name + "_mod0", core_construct.source,
                [=](const Program &program) { core_construct.verify(program, 0); }
            });

            if (core_construct.supports_modifiers) {
                universe.push_back({
                    core_construct.name + "_mod1", mod_samples[0] + core_construct.source,
                    [=](const Program &program) { core_construct.verify(program, 1); }
                });
                universe.push_back({
                    core_construct.name + "_mod2", mod_samples[0] + mod_samples[1] + core_construct.source,
                    [=](const Program &program) { core_construct.verify(program, 2); }
                });
                universe.push_back({
                    core_construct.name + "_mod3",
                    mod_samples[0] + mod_samples[1] + mod_samples[2] + core_construct.source,
                    [=](const Program &program) { core_construct.verify(program, 3); }
                });
            }
        }
        return universe;
    }
}
