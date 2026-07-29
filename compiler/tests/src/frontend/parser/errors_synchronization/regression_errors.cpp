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
            "Regression_1",
            "let b = { \"key\" 10 }\n"
            "let a = func_call(\n\n"
            "let b = some_other()",
            {
            {Err::ExpectedDictionaryKey, 1, 11},
            {Err::ExpectedArgumentNameOrClosingParen, 2, 19}
            }
            },
            ParserErrorsSynchronizationTestCase{
            "Regression_2",
            "func test() -> scalar { return 1\n"
            "// -- R&D Capitalization --\n"
            "let value_of_research_assets, current_year_amortization = get_rd()\n"
            "// -- WACC --\n"
            "let wacc = get_wacc()\n"
            "enum Scenario: scalar { LOW, BASE, HIGH}\n"
            "}\n",
            {
            {Err::TopLevelDeclarationNotAllowedHere, 6, 1}
            }
            },
            ParserErrorsSynchronizationTestCase{
            "Regression_2_a",
            "func test() -> scalar { return 1\n"
            "// -- R&D Capitalization --\n"
            "let value_of_research_assets, current_year_amortization = get_rd()\n"
            "// -- WACC --\n"
            "let wacc = get_wacc()\n"
            "enum Scenario: scalar { LOW, BASE, HIGH }\n",
            {
            {Err::ExpectedRightBraceAfterFunctionBody, 5, 21}
            }
            },
            ParserErrorsSynchronizationTestCase{
            "Regression_3",
            "#iterations = 10_000_\n"
            "\n"
            "// -- R&D Capitalization --\n"
            "let value_of_research_assets, current_year_amortization = get_rd(\n"
            "let a = 10\n",
            {
            {LexerErrorCode::TrailingSeparatorInNumberLiteral, 1, 15},
            {Err::ExpectedArgumentNameOrClosingParen, 4, 66}
            }
            },
            ParserErrorsSynchronizationTestCase{
            "Regression_4",
            "let a = {1, 2, 3}\n",
            {
            {Err::ExpectedDictionaryKey, 1, 10},
            {Err::ExpectedDictionaryKey, 1, 13},
            {Err::ExpectedDictionaryKey, 1, 16}
            }
            }
        ),
        [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& test_info) {
        return test_info.param.test_name;
        }
    );
}
