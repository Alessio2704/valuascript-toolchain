#pragma once
#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <functional>
#include <memory>
#include <algorithm>
#include "stages/frontend/parser/ast.h"

namespace valuascript::compiler::test {
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
    void check_reassign(const Program &p) {
        auto it = std::find_if(p.execution_steps.begin(), p.execution_steps.end(), [](const auto &s) {
            auto rs = dynamic_cast<Reassignment *>(s.get());
            auto target = rs ? dynamic_cast<TargetType *>(rs->target.get()) : nullptr;
            if constexpr (std::is_same_v<TargetType, IdentifierAccess>) {
                return target != nullptr;
            } else {
                return target && dynamic_cast<ReceiverType *>(target->target.get());
            }
        });
        EXPECT_NE(it, p.execution_steps.end()) << "Failed to recover reassignment for target type.";
    }

    inline std::vector<FollowingConstruct> get_all_top_level_following_constructs() {
        std::vector<std::string> mod_samples = {
            "@simple ",
            "@param(id: 100) ",
            "@complex(a: 1, b: \"str\", c: [1, 2]) "
        };

        std::vector<CoreConstruct> cores = {
            {
                "func", "func rec_func() -> void {}\n", true, [](const Program &p, int modifiers_count) {
                    auto it = std::find_if(p.function_definitions.begin(), p.function_definitions.end(),
                                           [](auto &f) { return f->name == "rec_func"; });
                    ASSERT_NE(it, p.function_definitions.end());
                    check_mods((*it)->modifiers, modifiers_count);
                }
            },
            {
                "struct", "struct RecStruct {}\n", true, [](const Program &p, int modifiers_count) {
                    auto it = std::find_if(p.struct_definitions.begin(), p.struct_definitions.end(),
                                           [](auto &s) { return s->name == "RecStruct"; });
                    ASSERT_NE(it, p.struct_definitions.end());
                    check_mods((*it)->modifiers, modifiers_count);
                }
            },
            {
                "enum", "enum RecEnum: int {}\n", true, [](const Program &p, int modifiers_count) {
                    auto it = std::find_if(p.enum_definitions.begin(), p.enum_definitions.end(),
                                           [](auto &e) { return e->name == "RecEnum"; });
                    ASSERT_NE(it, p.enum_definitions.end());
                    check_mods((*it)->modifiers, modifiers_count);
                }
            },
            {
                "typealias", "typealias RecAlias = int\n", true, [](const Program &p, int modifiers_count) {
                    auto it = std::find_if(p.type_aliases.begin(), p.type_aliases.end(),
                                           [](auto &t) { return t->name == "RecAlias"; });
                    ASSERT_NE(it, p.type_aliases.end());
                    check_mods((*it)->modifiers, modifiers_count);
                }
            },
            {
                "let", "let rec_let = 1\n", true, [](const Program &p, int modifiers_count) {
                    auto it = std::find_if(p.execution_steps.begin(), p.execution_steps.end(), [](auto &s) {
                        auto a = dynamic_cast<Assignment *>(s.get());
                        return a && !a->targets.empty() && a->targets[0].first == "rec_let";
                    });
                    ASSERT_NE(it, p.execution_steps.end());
                    check_mods(dynamic_cast<Assignment *>(it->get())->modifiers, modifiers_count);
                }
            },
            {
                "var", "var rec_var = 1\n", true, [](const Program &p, int modifiers_count) {
                    auto it = std::find_if(p.execution_steps.begin(), p.execution_steps.end(), [](auto &s) {
                        auto a = dynamic_cast<Assignment *>(s.get());
                        return a && !a->targets.empty() && a->targets[0].first == "rec_var";
                    });
                    ASSERT_NE(it, p.execution_steps.end());
                    check_mods(dynamic_cast<Assignment *>(it->get())->modifiers, modifiers_count);
                }
            },
            {
                "import", "import \"lib\"\n", false, [](const Program &p, int) {
                    EXPECT_FALSE(p.import_statements.empty());
                }
            },
            {
                "directive", "#dir\n", false, [](const Program &p, int) {
                    EXPECT_FALSE(p.directives.empty());
                }
            },
            {
                "call", "rec_call()\n", false, [](const Program &p, int) {
                    EXPECT_TRUE(std::any_of(p.execution_steps.begin(), p.execution_steps.end(), [](auto& s){
                        auto es = dynamic_cast<ExpressionStatement*>(s.get()); return es && dynamic_cast<FunctionCall*>(
                            es->expr.get());
                        }));
                }
            },
            {
                "reassign_id", "rec_x = 1\n", false, [](const Program &p, int) {
                    check_reassign<IdentifierAccess, void>(p);
                }
            },
            {
                "self_dot", "self.prop = 1\n", false, [](const Program &p, int) {
                    check_reassign<DotAccess, SelfExpression>(p);
                }
            },
            {
                "self_bracket", "self[0] = 1\n", false, [](const Program &p, int) {
                    check_reassign<BracketAccess, SelfExpression>(p);
                }
            },
            {
                "id_dot", "obj.prop = 1\n", false, [](const Program &p, int) {
                    check_reassign<DotAccess, IdentifierAccess>(p);
                }
            },
            {
                "id_bracket", "arr[0] = 1\n", false, [](const Program &p, int) {
                    check_reassign<BracketAccess, IdentifierAccess>(p);
                }
            }
        };

        std::vector<FollowingConstruct> universe;
        for (const auto &core: cores) {
            universe.push_back({core.name + "_mod0", core.source, [=](const Program &p) { core.verify(p, 0); }});

            if (core.supports_modifiers) {
                universe.push_back({
                    core.name + "_mod1", mod_samples[0] + core.source, [=](const Program &p) { core.verify(p, 1); }
                });
                universe.push_back({
                    core.name + "_mod2", mod_samples[0] + mod_samples[1] + core.source,
                    [=](const Program &p) { core.verify(p, 2); }
                });
                universe.push_back({
                    core.name + "_mod3", mod_samples[0] + mod_samples[1] + mod_samples[2] + core.source,
                    [=](const Program &p) { core.verify(p, 3); }
                });
            }
        }
        return universe;
    }
}
