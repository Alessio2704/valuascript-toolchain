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
                .name = "grouping", .source = "let a = (1 + 2\n", .expected_err = Err::ExpectedRightParenAfterExpression,
                .verify = [](const Program &program) {
                    ASSERT_FALSE(program.execution_steps.empty());
                    StmtVerifier(IsAssignment({{.name = "a"}}, IsGrouping(IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")))))(program.execution_steps[0].get());
                }
            },
            {
                .name = "tuple", .source = "let a = (1, 2\n", .expected_err = Err::ExpectedRightParenAfterTupleElements,
                .verify = [](const Program &program) {
                    ASSERT_FALSE(program.execution_steps.empty());
                    StmtVerifier(IsAssignment({{.name = "a"}}, IsTuple(IsNumber("1"), IsNumber("2"))))(program.execution_steps[0].get());
                }
            },
            {
                .name = "tensor", .source = "let a = [1, 2\n", .expected_err = Err::UnmatchedBracketAfterTensorElements,
                .verify = [](const Program &program) {
                    ASSERT_FALSE(program.execution_steps.empty());
                    StmtVerifier(IsAssignment({{.name = "a"}}, IsTensor(IsNumber("1"), IsNumber("2"))))(program.execution_steps[0].get());
                }
            },
            {
                .name = "dict", .source = "let a = { b: 1\n", .expected_err = Err::UnmatchedBraceInDictionaryLiteral,
                .verify = [](const Program &program) {
                    ASSERT_FALSE(program.execution_steps.empty());
                    StmtVerifier(IsAssignment({{.name = "a"}}, IsDict(DictItemSpec{.key = "b", .value_v = IsNumber("1")})))(program.execution_steps[0].get());
                }
            },
            {
                .name = "func", .source = "func f() -> int { let x = 1\n", .expected_err = Err::ExpectedRightBraceAfterFunctionBody,
                .verify = [](const Program &program) {
                    ASSERT_FALSE(program.function_definitions.empty());
                    if (auto f = ExpectNode<FunctionDefinition>(program.function_definitions[0].get())) {
                        EXPECT_EQ(f->name, "f");
                    }
                }
            },
            {
                .name = "struct", .source = "struct S { x: int\n", .expected_err = Err::ExpectedRightBraceAfterStructBody,
                .verify = [](const Program &program) {
                    ASSERT_FALSE(program.struct_definitions.empty());
                    IsStructDef("S", {}, FieldSpec{.name = "x", .type_v = IsType("int")})(program.struct_definitions[0].get());
                }
            },
            {
                .name = "enum", .source = "enum E: int { A\n", .expected_err = Err::ExpectedRightBraceAfterEnumBody,
                .verify = [](const Program &program) {
                    ASSERT_FALSE(program.enum_definitions.empty());
                    IsEnumDef("E", {}, IsType("int"), EnumCaseSpec{.name = "A"})(program.enum_definitions[0].get());
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
                    size_t exp_col = scenario.source.find_last_not_of(" \t\n\r") + 1;

                    if (scenario.name == "func" && !follow.is_top_level_only) {
                        exp_line = 2;
                        std::string stripped = follow.source;
                        while (!stripped.empty() && stripped.back() == '\n') stripped.pop_back();
                        exp_col = stripped.find_last_not_of(" \t\n\r") + 1;
                    }

                    test_cases.push_back(ParserErrorsSynchronizationTestCase{
                        .test_name = test_name,
                        .source_code = combined_source,
                        .expected_errors = {{.code = scenario.expected_err, .line = exp_line, .column = exp_col}},
                        .verify_ast = [v_scene = scenario.verify, v_follow = follow.verify](const Program &p) {
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
        [](const auto& test_info) {
        return test_info.param.test_name;
        }
    );
}
