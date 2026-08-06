#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        auto ExpectNoStructs()
        {
            return [](const Program& ast)
            {
                ASSERT_EQ(ast.struct_definitions.size(), 0);
                ASSERT_EQ(ast.execution_steps.size(), 1);
            };
        }
    }

    class StructParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(StructParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        StructStressTest,
        StructParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
            .test_name = "no_left_brace_struct_empty_ast",
            .source_code = "struct Test id: int }\nlet a = 1\n",
            .expected_errors = { {.code = Err::ExpectedBraceInStructDefinition, .line = 1, .column = 13} },
            .verify_ast = ExpectNoStructs()
            }
        ),
        TestNameGenerator{}
    );
}
