#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"
#include "frontend/parser/shared_following_constructs.h"

using namespace valuascript::shared;

namespace valuascript::compiler::test {
    namespace {
        struct MissingConstruct {
            std::string name;
            std::string source;
            ValuascriptErrorCode expected_err;
            size_t err_col;
            std::function<void(const Program &)> verify;
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

        std::vector<ParserErrorsSynchronizationTestCase> GenerateTestCases() {
            std::vector<ParserErrorsSynchronizationTestCase> cases;
            auto following_constructs = get_all_top_level_following_constructs();

            for (const auto &mc: missing_constructs) {
                for (const auto &fc: following_constructs) {
                    std::string test_name = mc.name + "_recovers_" + fc.name;
                    std::string source = mc.source + fc.source;

                    size_t expected_line = 1;
                    size_t expected_col = mc.err_col;
                    bool is_absorbed = false;
                    if (mc.name == "func") {
                        if (fc.name.find("func") != 0 && fc.name.find("struct") != 0 &&
                            fc.name.find("enum") != 0 && fc.name.find("typealias") != 0 &&
                            fc.name.find("import") != 0 && fc.name.find("directive") != 0) {
                            is_absorbed = true;
                            expected_line = 2;

                            std::string stripped_fc = fc.source;
                            while (!stripped_fc.empty() && stripped_fc.back() == '\n') {
                                stripped_fc.pop_back();
                            }
                            expected_col = stripped_fc.length() + 1;
                        }
                    }

                    cases.push_back(ParserErrorsSynchronizationTestCase{
                        test_name,
                        source,
                        {{mc.expected_err, expected_line, expected_col}},
                        [mc_verify = mc.verify, fc_verify = fc.verify, is_absorbed](const Program &p) {
                            mc_verify(p);

                            if (is_absorbed) {
                                Program &mutable_p = const_cast<Program &>(p);
                                for (auto &f: mutable_p.function_definitions) {
                                    for (auto &s: f->body) {
                                        mutable_p.execution_steps.push_back(std::move(s));
                                    }
                                }
                            }

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
