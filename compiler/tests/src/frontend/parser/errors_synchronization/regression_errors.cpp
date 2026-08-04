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
                .source_code = "let b = { \"key\" 10 }\nlet a = func_call(\n\nlet b = some_other()",
                .expected_errors = {
                    {.code = Err::ExpectedDictionaryKey, .line = 1, .column = 11},
                    {.code = Err::ExpectedArgumentNameOrClosingParen, .line = 2, .column = 19}
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "Regression_2",
                .source_code = "func test() -> scalar { return 1\n"
                               "// -- R&D Capitalization --\n"
                               "let value_of_research_assets, current_year_amortization = get_rd()\n"
                               "// -- WACC --\n"
                               "let wacc = get_wacc()\n"
                               "enum Scenario: scalar { LOW, BASE, HIGH}\n"
                               "}\n",
                .expected_errors = {
                    {.code = Err::InvalidConstructPlacement, .line = 6, .column = 1}
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "Regression_2_a",
                .source_code = "func test() -> scalar { return 1\n"
                               "// -- R&D Capitalization --\n"
                               "let value_of_research_assets, current_year_amortization = get_rd()\n"
                               "// -- WACC --\n"
                               "let wacc = get_wacc()\n"
                               "enum Scenario: scalar { LOW, BASE, HIGH }\n",
                .expected_errors = {
                    {.code = Err::ExpectedRightBraceAfterFunctionBody, .line = 5, .column = 21}
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
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "Regression_4",
                .source_code = "let a = {1, 2, 3}\n",
                .expected_errors = {
                    {.code = Err::ExpectedDictionaryKey, .line = 1, .column = 10},
                    {.code = Err::ExpectedDictionaryKey, .line = 1, .column = 13},
                    {.code = Err::ExpectedDictionaryKey, .line = 1, .column = 16}
                }
            }
        ),
        TestNameGenerator{}
    );
}
