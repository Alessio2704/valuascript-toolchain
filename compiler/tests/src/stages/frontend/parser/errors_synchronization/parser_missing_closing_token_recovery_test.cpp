#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    namespace {
        struct MissingConstruct {
            std::string name;
            std::string source;
            ValuascriptErrorCode expected_err;
            size_t err_col;
            std::function<void(const Program &)> verify;
        };

        struct FollowingConstruct {
            std::string name;
            std::string source;
            std::function<void(const Program &)> verify;
        };

        auto check_assignment_recovery = [](const Program &p, const std::string &target_name,
                                            const std::string &expected_mod) {
            bool found = false;
            auto check_block = [&](const std::vector<std::unique_ptr<Statement> > &block) {
                for (const auto &s: block) {
                    if (auto a = dynamic_cast<Assignment *>(s.get())) {
                        if (!a->targets.empty() && a->targets[0].first == target_name) {
                            found = true;
                            if (!expected_mod.empty()) {
                                EXPECT_EQ(a->modifiers.size(), 1) << "Expected modifier on " << target_name;
                                if (!a->modifiers.empty()) {
                                    EXPECT_EQ(a->modifiers[0].name, expected_mod);
                                }
                            } else {
                                EXPECT_TRUE(a->modifiers.empty()) << "Did not expect modifier on " << target_name;
                            }
                        }
                    }
                }
            };

            check_block(p.execution_steps);
            for (const auto &f: p.function_definitions) {
                check_block(f->body);
            }

            EXPECT_TRUE(found) << "Failed to recover assignment for " << target_name;
        };

        std::vector<MissingConstruct> missing_constructs = {
            {
                "grouping", "let a = (1 + 2\n", Err::ExpectedRightParenAfterExpression, 15,
                [](const Program &p) {
                    bool found = false;
                    for (const auto &s: p.execution_steps) {
                        if (auto a = dynamic_cast<Assignment *>(s.get())) {
                            if (!a->targets.empty() && a->targets[0].first == "a") {
                                found = true;
                                EXPECT_NE(dynamic_cast<GroupingExpression*>(a->value.get()),
                                          nullptr) << "Failed to recover value as GroupingExpression";
                            }
                        }
                    }
                    EXPECT_TRUE(found) << "Failed to recover grouping assignment 'a'";
                }
            },
            {
                "tuple", "let a = (1, 2\n", Err::ExpectedRightParenAfterTupleElements, 14,
                [](const Program &p) {
                    bool found = false;
                    for (const auto &s: p.execution_steps) {
                        if (auto a = dynamic_cast<Assignment *>(s.get())) {
                            if (!a->targets.empty() && a->targets[0].first == "a") {
                                found = true;
                                EXPECT_NE(dynamic_cast<TupleLiteral*>(a->value.get()),
                                          nullptr) << "Failed to recover value as TupleLiteral";
                            }
                        }
                    }
                    EXPECT_TRUE(found) << "Failed to recover tuple assignment 'a'";
                }
            },
            {
                "tensor", "let a = [1, 2\n", Err::UnmatchedBracketAfterTensorElements, 14,
                [](const Program &p) {
                    bool found = false;
                    for (const auto &s: p.execution_steps) {
                        if (auto a = dynamic_cast<Assignment *>(s.get())) {
                            if (!a->targets.empty() && a->targets[0].first == "a") {
                                found = true;
                                EXPECT_NE(dynamic_cast<TensorLiteral*>(a->value.get()),
                                          nullptr) << "Failed to recover value as TensorLiteral";
                            }
                        }
                    }
                    EXPECT_TRUE(found) << "Failed to recover tensor assignment 'a'";
                }
            },
            {
                "dict", "let a = { b: 1\n", Err::UnmatchedBraceInDictionaryLiteral, 15,
                [](const Program &p) {
                    bool found = false;
                    for (const auto &s: p.execution_steps) {
                        if (auto a = dynamic_cast<Assignment *>(s.get())) {
                            if (!a->targets.empty() && a->targets[0].first == "a") {
                                found = true;
                                EXPECT_NE(dynamic_cast<DictLiteral*>(a->value.get()),
                                          nullptr) << "Failed to recover value as DictLiteral";
                            }
                        }
                    }
                    EXPECT_TRUE(found) << "Failed to recover dict assignment 'a'";
                }
            },
            {
                "func", "func f() -> int { let x = 1\n", Err::ExpectedRightBraceAfterFunctionBody, 28,
                [](const Program &p) {
                    bool found = false;
                    for (const auto &f: p.function_definitions) {
                        if (f->name == "f") found = true;
                    }
                    EXPECT_TRUE(found) << "Failed to recover unclosed func 'f'";
                }
            },
            {
                "struct", "struct S { x: int\n", Err::ExpectedRightBraceAfterStructBody, 18,
                [](const Program &p) {
                    bool found = false;
                    for (const auto &s: p.struct_definitions) {
                        if (s->name == "S") found = true;
                    }
                    EXPECT_TRUE(found) << "Failed to recover unclosed struct 'S'";
                }
            },
            {
                "enum", "enum E: int { A\n", Err::ExpectedRightBraceAfterEnumBody, 16,
                [](const Program &p) {
                    bool found = false;
                    for (const auto &e: p.enum_definitions) {
                        if (e->name == "E") found = true;
                    }
                    EXPECT_TRUE(found) << "Failed to recover unclosed enum 'E'";
                }
            }
        };

        std::vector<FollowingConstruct> following_constructs = {
            {
                "func",
                "func valid_func() -> int {}\n",
                [](const Program &p) {
                    bool found = false;
                    for (const auto &f: p.function_definitions) {
                        if (f->name == "valid_func") found = true;
                    }
                    EXPECT_TRUE(found) << "Failed to recover valid_func";
                }
            },
            {
                "struct",
                "struct ValidStruct {}\n",
                [](const Program &p) {
                    bool found = false;
                    for (const auto &s: p.struct_definitions) {
                        if (s->name == "ValidStruct") found = true;
                    }
                    EXPECT_TRUE(found) << "Failed to recover ValidStruct";
                }
            },
            {
                "enum",
                "enum ValidEnum: int {}\n",
                [](const Program &p) {
                    bool found = false;
                    for (const auto &e: p.enum_definitions) {
                        if (e->name == "ValidEnum") found = true;
                    }
                    EXPECT_TRUE(found) << "Failed to recover ValidEnum";
                }
            },
            {
                "directive",
                "#valid_directive\n",
                [](const Program &p) {
                    bool found = false;
                    for (const auto &d: p.directives) {
                        if (d->name == "valid_directive") found = true;
                    }
                    EXPECT_TRUE(found) << "Failed to recover valid_directive";
                }
            },
            {
                "import",
                "import \"valid_import\"\n",
                [](const Program &p) {
                    bool found = false;
                    for (const auto &i: p.import_statements) {
                        if (i->path == "\"valid_import\"") found = true;
                    }
                    EXPECT_TRUE(found) << "Failed to recover import";
                }
            },
            {
                "let",
                "let valid_let = 1\n",
                [](const Program &p) { check_assignment_recovery(p, "valid_let", ""); }
            },
            {
                "var",
                "var valid_var = 1\n",
                [](const Program &p) { check_assignment_recovery(p, "valid_var", ""); }
            },
            {
                "modifier_func",
                "@valid_mod func valid_mod_func() -> int {}\n",
                [](const Program &p) {
                    bool found = false;
                    for (const auto &f: p.function_definitions) {
                        if (f->name == "valid_mod_func") {
                            found = true;
                            EXPECT_EQ(f->modifiers.size(), 1);
                            if (!f->modifiers.empty()) {
                                EXPECT_EQ(f->modifiers[0].name, "valid_mod");
                            }
                        }
                    }
                    EXPECT_TRUE(found) << "Failed to recover valid_mod_func with modifier";
                }
            },
            {
                "modifier_struct",
                "@valid_mod struct ValidModStruct {}\n",
                [](const Program &p) {
                    bool found = false;
                    for (const auto &s: p.struct_definitions) {
                        if (s->name == "ValidModStruct") {
                            found = true;
                            EXPECT_EQ(s->modifiers.size(), 1);
                            if (!s->modifiers.empty()) {
                                EXPECT_EQ(s->modifiers[0].name, "valid_mod");
                            }
                        }
                    }
                    EXPECT_TRUE(found) << "Failed to recover ValidModStruct with modifier";
                }
            },
            {
                "modifier_enum",
                "@valid_mod enum ValidModEnum: int {}\n",
                [](const Program &p) {
                    bool found = false;
                    for (const auto &e: p.enum_definitions) {
                        if (e->name == "ValidModEnum") {
                            found = true;
                            EXPECT_EQ(e->modifiers.size(), 1);
                            if (!e->modifiers.empty()) {
                                EXPECT_EQ(e->modifiers[0].name, "valid_mod");
                            }
                        }
                    }
                    EXPECT_TRUE(found) << "Failed to recover ValidModEnum with modifier";
                }
            },
            {
                "modifier_let",
                "@valid_mod let valid_mod_let = 1\n",
                [](const Program &p) { check_assignment_recovery(p, "valid_mod_let", "valid_mod"); }
            },
            {
                "modifier_var",
                "@valid_mod var valid_mod_var = 1\n",
                [](const Program &p) { check_assignment_recovery(p, "valid_mod_var", "valid_mod"); }
            }
        };

        std::vector<ParserErrorsSynchronizationTestCase> GenerateTestCases() {
            std::vector<ParserErrorsSynchronizationTestCase> cases;
            for (const auto &mc: missing_constructs) {
                for (const auto &fc: following_constructs) {
                    std::string test_name = mc.name + "_recovers_" + fc.name;
                    std::string source = mc.source + fc.source;

                    size_t expected_line = 1;
                    size_t expected_col = mc.err_col;

                    if (mc.name == "func") {
                        if (fc.name == "let" || fc.name == "var") {
                            expected_line = 2;
                            expected_col = 18;
                        } else if (fc.name == "modifier_let" || fc.name == "modifier_var") {
                            expected_line = 2;
                            expected_col = 33;
                        }
                    }

                    cases.push_back(ParserErrorsSynchronizationTestCase{
                        test_name,
                        source,
                        {{mc.expected_err, expected_line, expected_col}},
                        [mc_verify = mc.verify, fc_verify = fc.verify](const Program &p) {
                            mc_verify(p);
                            fc_verify(p);
                        }
                    });
                }
            }
            return cases;
        }
    }

    class MissingClosingTokenRecoveryTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(MissingClosingTokenRecoveryTest, RecoversSubsequentTopLevelDeclarations) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserMissingClosingTests,
        MissingClosingTokenRecoveryTest,
        ::testing::ValuesIn(GenerateTestCases()),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
        return info.param.test_name;
        }
    );
}
