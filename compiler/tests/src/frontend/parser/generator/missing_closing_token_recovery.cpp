#include <gtest/gtest.h>
#include "../errors_synchronization/parser_errors_synchronization_base.h"
#include "language_constructs_provider.h"

using namespace valuascript::shared;

namespace valuascript::compiler::test {
    namespace {
        struct MissingClosingScenario {
            std::string name;
            std::string source;
            Err expected_err;
            std::function<void(const Program &)> verify;
        };

        std::vector<MissingClosingScenario> scenarios = {
            {
                "grouping", "let a = (1 + 2\n", Err::ExpectedRightParenAfterExpression,
                [](const Program &program) {
                    auto target_stmt = find_statement(program, [](const Statement *stmt) {
                        auto assignment = dynamic_cast<const Assignment *>(stmt);
                        return assignment && !assignment->targets.empty() && assignment->targets[0].first == "a" &&
                               dynamic_cast<GroupingExpression *>(assignment->value.get()) != nullptr;
                    });
                    EXPECT_NE(target_stmt, nullptr) << "Failed to recover grouping assignment 'a'";
                }
            },
            {
                "tuple", "let a = (1, 2\n", Err::ExpectedRightParenAfterTupleElements,
                [](const Program &program) {
                    auto target_stmt = find_statement(program, [](const Statement *stmt) {
                        auto assignment = dynamic_cast<const Assignment *>(stmt);
                        return assignment && !assignment->targets.empty() && assignment->targets[0].first == "a" &&
                               dynamic_cast<TupleLiteral *>(assignment->value.get()) != nullptr;
                    });
                    EXPECT_NE(target_stmt, nullptr) << "Failed to recover tuple assignment 'a'";
                }
            },
            {
                "tensor", "let a = [1, 2\n", Err::UnmatchedBracketAfterTensorElements,
                [](const Program &program) {
                    auto target_stmt = find_statement(program, [](const Statement *stmt) {
                        auto assignment = dynamic_cast<const Assignment *>(stmt);
                        return assignment && !assignment->targets.empty() && assignment->targets[0].first == "a" &&
                               dynamic_cast<TensorLiteral *>(assignment->value.get()) != nullptr;
                    });
                    EXPECT_NE(target_stmt, nullptr) << "Failed to recover tensor assignment 'a'";
                }
            },
            {
                "dict", "let a = { b: 1\n", Err::UnmatchedBraceInDictionaryLiteral,
                [](const Program &program) {
                    auto target_stmt = find_statement(program, [](const Statement *stmt) {
                        auto assignment = dynamic_cast<const Assignment *>(stmt);
                        return assignment && !assignment->targets.empty() && assignment->targets[0].first == "a" &&
                               dynamic_cast<DictLiteral *>(assignment->value.get()) != nullptr;
                    });
                    EXPECT_NE(target_stmt, nullptr) << "Failed to recover dict assignment 'a'";
                }
            },
            {
                "func", "func f() -> int { let x = 1\n", Err::ExpectedRightBraceAfterFunctionBody,
                [](const Program &program) {
                    bool found = std::any_of(program.function_definitions.begin(), program.function_definitions.end(),
                                             [](const auto &func_def) { return func_def->name == "f"; });
                    EXPECT_TRUE(found) << "Failed to recover unclosed func 'f'";
                }
            },
            {
                "struct", "struct S { x: int\n", Err::ExpectedRightBraceAfterStructBody,
                [](const Program &program) {
                    bool found = std::any_of(program.struct_definitions.begin(), program.struct_definitions.end(),
                                             [](const auto &struct_def) { return struct_def->name == "S"; });
                    EXPECT_TRUE(found) << "Failed to recover unclosed struct 'S'";
                }
            },
            {
                "enum", "enum E: int { A\n", Err::ExpectedRightBraceAfterEnumBody,
                [](const Program &program) {
                    bool found = std::any_of(program.enum_definitions.begin(), program.enum_definitions.end(),
                                             [](const auto &enum_def) { return enum_def->name == "E"; });
                    EXPECT_TRUE(found) << "Failed to recover unclosed enum 'E'";
                }
            }
        };

        std::vector<ParserErrorsSynchronizationTestCase> GenerateSyncTests() {
            std::vector<ParserErrorsSynchronizationTestCase> test_cases;
            auto followers = LanguageConstructsProvider::build_all_test_variants();

            for (const auto &scenario: scenarios) {
                for (const auto &follow: followers) {
                    std::string test_name = scenario.name + "_syncs_to_" + follow.name;
                    std::string combined_source = scenario.source + follow.source;

                    size_t exp_line = 1;
                    size_t exp_col = scenario.source.length();

                    if (scenario.name == "func" && !follow.is_top_level_only) {
                        exp_line = 2;
                        std::string stripped = follow.source;
                        while (!stripped.empty() && stripped.back() == '\n') stripped.pop_back();
                        exp_col = stripped.length() + 1;
                    }

                    test_cases.push_back({
                        test_name, combined_source,
                        {{scenario.expected_err, exp_line, exp_col}},
                        [v_scene = scenario.verify, v_follow = follow.verify](const Program &p) {
                            v_scene(p);
                            v_follow(p);
                        }
                    });
                }
            }
            return test_cases;
        }
    }

    class MissingClosingTokenSyncTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(MissingClosingTokenSyncTest, VerifyRecovery) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserSync,
        MissingClosingTokenSyncTest,
        ::testing::ValuesIn(GenerateSyncTests()),
        [](const auto& info) {
        return info.param.test_name;
        }
    );
}
