#include <gtest/gtest.h>
#include "frontend/parser/parser_errors_synchronization_base.h"
#include "frontend/parser/shared_following_constructs.h"

using namespace valuascript::shared;

namespace valuascript::compiler::test {
    namespace {
        struct MissingConstruct {
            std::string name;
            std::string source;
            ValuascriptErrorCode expected_err;
            std::function<void(const Program &)> verify;
        };

        std::vector<MissingConstruct> all_missing_constructs = {
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

        std::vector<ParserErrorsSynchronizationTestCase> GenerateTestCases() {
            std::vector<ParserErrorsSynchronizationTestCase> test_cases;
            auto all_following_constructs = get_all_top_level_following_constructs();

            for (const auto &missing_construct: all_missing_constructs) {
                for (const auto &following_construct: all_following_constructs) {
                    std::string test_name = missing_construct.name + "_recovers_" + following_construct.name;
                    std::string combined_source = missing_construct.source + following_construct.source;

                    size_t expected_line = 1;
                    size_t expected_column = missing_construct.source.length();

                    if (missing_construct.name == "func") {
                        if (following_construct.name.find("func") != 0 && following_construct.name.find("struct") != 0
                            &&
                            following_construct.name.find("enum") != 0 && following_construct.name.find("typealias") !=
                            0 &&
                            following_construct.name.find("import") != 0 && following_construct.name.find("directive")
                            != 0) {
                            expected_line = 2;

                            std::string stripped_following_source = following_construct.source;
                            while (!stripped_following_source.empty() && stripped_following_source.back() == '\n') {
                                stripped_following_source.pop_back();
                            }
                            expected_column = stripped_following_source.length() + 1;
                        }
                    }

                    test_cases.push_back(ParserErrorsSynchronizationTestCase{
                        test_name,
                        combined_source,
                        {{missing_construct.expected_err, expected_line, expected_column}},
                        [verify_missing_construct = missing_construct.verify,
                            verify_following_construct = following_construct.verify](const Program &program) {
                            verify_missing_construct(program);
                            verify_following_construct(program);
                        }
                    });
                }
            }
            return test_cases;
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
