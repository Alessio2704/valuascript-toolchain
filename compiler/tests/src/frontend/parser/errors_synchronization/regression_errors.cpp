#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    class GeneralParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(GeneralParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserExhaustiveStressTests,
        GeneralParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "Regression_1",
                .source_code = "let a = func_call(\n\nlet b = some_other()",
                .expected_errors = {
                    {.code = Err::ExpectedArgumentNameOrClosingParen, .line = 1, .column = 19}
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "Regression_3",
                .source_code = "#iterations = 10_000_\n"
                               "\n"
                               "// -- R&D Capitalization --\n"
                               "let value_of_research_assets, current_year_amortization = get_rd(\n"
                               "let a = 10\n",
                .expected_errors = {
                    {.code = LexerErrorCode::TrailingSeparatorInNumberLiteral, .line = 1, .column = 15},
                    {.code = Err::ExpectedArgumentNameOrClosingParen, .line = 4, .column = 66}
                }
            }
        ),
        TestNameGenerator{}
    );
}
