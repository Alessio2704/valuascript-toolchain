#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    class AssignmentParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(AssignmentParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        AssignmentErrors,
        AssignmentParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "reassignment_self_missing_property_name",
                .source_code = "self. = 42\nlet c = 2\n",
                .expected_errors = {
                    {.code = Err::ExpectedPropertyName, .line = 1, .column = 6}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 2);
                    auto assign_c = dynamic_cast<Assignment*>(ast.execution_steps[1].get());
                    ASSERT_NE(assign_c, nullptr);
                    EXPECT_EQ(assign_c->targets[0].name, "c");
                }
            }
        ),
        TestNameGenerator{}
    );
}