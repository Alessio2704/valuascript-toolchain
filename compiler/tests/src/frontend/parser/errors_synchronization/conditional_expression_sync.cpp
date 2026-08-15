#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const ConditionalExpression* GetAssignedConditional(const Program& ast)
        {
            EXPECT_EQ(ast.execution_steps.size(), 1) << "Expected one assignment statement to survive.";
            if (ast.execution_steps.empty()) return nullptr;

            const auto* assign = dynamic_cast<const Assignment*>(ast.execution_steps.front().get());
            EXPECT_NE(assign, nullptr) << "Expected statement to be an Assignment.";
            if (!assign) return nullptr;

            const auto* cond = dynamic_cast<const ConditionalExpression*>(assign->value.get());
            EXPECT_NE(cond, nullptr) << "Expected assigned value to be a ConditionalExpression.";
            return cond;
        }
    }

    class ConditionalParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(ConditionalParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserExhaustiveStressTests,
        ConditionalParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "bare_if_token_at_eof_discards_gracefully",
                .source_code = "let a = if\n",
                .expected_errors = {
                    {.code = Err::InvalidExpression, .line = 1, .column = 11},
                    {.code = Err::MissingThenToken, .line = 1, .column = 11},
                    {.code = Err::MissingElseToken, .line = 1, .column = 11},
                },
                .verify_ast = [](const Program& ast) {
                    const auto* cond = GetAssignedConditional(ast);
                    ASSERT_NE(cond, nullptr);
                }
            }
        ),
        TestNameGenerator{}
    );
}
