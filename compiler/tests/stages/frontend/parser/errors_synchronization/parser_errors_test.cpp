#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

class GeneralParserSynchronizationTest : public ParserErrorsSynchronizationBase {
};

TEST_P(GeneralParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
    run_parser_and_check_errors(GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    ParserExhaustiveStressTests,
    GeneralParserSynchronizationTest,
    ::testing::Values(
        ParserErrorsSynchronizationTestCase{
        "VariableDeclarationErrors",
        "@ let a = 10\n"
        "let = 20\n"
        "let struct = 30\n"
        "let d",
        {
        {ValuascriptErrorCode::ExpectedModifierName, 1, 3},
        {ValuascriptErrorCode::InvalidIdentifier, 2, 5},
        {ValuascriptErrorCode::ReservedKeywordAsIdentifier, 3, 5},
        {ValuascriptErrorCode::ExpectedStructName, 3, 12},
        {ValuascriptErrorCode::IncompleteAssignment, 4, 6}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "FunctionDefinitionErrors",
        "func (a: int) -> int {}\n"
        "func test1(a int) -> int {}\n"
        "func test2(b: int) int {}\n",
        {
        {ValuascriptErrorCode::MissingFunctionName, 1, 6},
        {ValuascriptErrorCode::MissingColonAfterParameter, 2, 14},
        {ValuascriptErrorCode::MissingArrowInFunction, 3, 20}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "DataStructureErrors",
        "let b = { \"key\" 10 }\n"
        "let b = { key 10 }\n"
        "let c = obj[]\n"
        "let d = obj.",
        {
        {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 11},
        {ValuascriptErrorCode::ExpectedColonAfterDictionaryKey, 2, 15},
        {ValuascriptErrorCode::EmptyBracketAccess, 3, 12},
        {ValuascriptErrorCode::ExpectedPropertyName, 4, 13}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "ControlFlowAndExpressionErrors",
        "let a = if (true) 10 else 20\n"
        "let x = 10 < 20 < 30\n"
        "let a = switch (val) { case 1 10 }\n"
        "let a = switch (val) { default -> 1 default -> 2 }\n",
        {
        {ValuascriptErrorCode::MissingThenToken, 1, 19},
        {ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 2, 17},
        {ValuascriptErrorCode::ExpectedEnumCaseNameAfterCase, 3, 29},
        {ValuascriptErrorCode::MultipleDefaultCasesInSwitch, 4, 45}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "DeepSynchronizationStressTest",
        "let valid1 = 100\n"
        "let broken1 = (10  2)\n"
        "func valid_func() -> int {}\n"
        "struct { }\n"
        "let valid2 = 200\n"
        "let a = { a: 1 b: 2}",
        {
        {ValuascriptErrorCode::MissingOperatorInsideGrouping, 2, 20},
        {ValuascriptErrorCode::ExpectedStructName, 4, 8},
        {ValuascriptErrorCode::ExpectedCommaSeparatorInDictionaryLiteral, 6, 16}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "UnterminatedParenthesisBracketsOrBraces",
        "let a = (10 + 20 \n"
        "let valid_1 = 100\n"
        "let valid_2 = 200\n"
        "let b = [1, 2, 3 \n"
        "let valid_3 = 300\n"
        "let c = { key: \"value\" \n"
        "let valid_4 = 400\n",
        {
        {ValuascriptErrorCode::ExpectedRightParenAfterExpression, 1, 17},
        {ValuascriptErrorCode::UnmatchedBracketAfterVectorElements, 4, 17},
        {ValuascriptErrorCode::UnmatchedBraceInDictionaryLiteral, 6, 23}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "Regression_1",
        "let b = { \"key\" 10 }\n"
        "let a = func_call(\n\n"
        "let b = some_other()",
        {
        {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 11},
        {ValuascriptErrorCode::ExpectedArgumentNameOrClosingParen, 2, 19}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "MissingOperand",
        "let res = a  (b - c)\n"
        "let res = a + (b  c)\n"
        "let res = a[1]  (b - c)\n"
        "let res = a[1] / (b  c)\n"
        "let res = a[1] / (1  c)\n"
        "let res = a[1] (1 + c)\n"
        "let res = a[1] + (b.a  c)\n"
        "let res = a[1] + (b.a  c[3].b)\n"
        "func test() -> scalar {\n"
        "return a + a.key (1 + 2)\n"
        "}\n"
        "func test() -> scalar {\n"
        "return a + a.key (1 + 2)\n"
        "}\n"
        "let res = a ([1, 2])\n"
        "let res = a ({1, 2})\n"
        "let res = a ([[1, 2], [3, 4])\n"
        "let res = a (-5)\n"
        "let a = switch (s) { case LOW -> 1  (3 + 3) case HIGH -> 3 }\n",
        {
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 1, 14},
        {ValuascriptErrorCode::MissingOperatorInsideGrouping, 2, 19},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 3, 17},
        {ValuascriptErrorCode::MissingOperatorInsideGrouping, 4, 22},
        {ValuascriptErrorCode::MissingOperatorInsideGrouping, 5, 22},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 6, 16},
        {ValuascriptErrorCode::MissingOperatorInsideGrouping, 7, 24},
        {ValuascriptErrorCode::MissingOperatorInsideGrouping, 8, 24},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 10, 18},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 13, 18},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 15, 13},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 16, 13},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 17, 13},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 18, 13},
        {ValuascriptErrorCode::MissingOperatorOrArgumentName, 19, 37},
        }
        },
        ParserErrorsSynchronizationTestCase{
        "FunctionCallAndSignatureErrors",
        "func process(a: int b: string) -> int {}\n"
        "let result = process(a: 10 20)\n"
        "let x = trailing_comma(a: 1, )\n"
        "func true() -> { return }",
        {
        {ValuascriptErrorCode::ExpectedCommaSeparatorInParameterList, 1, 21},
        {ValuascriptErrorCode::MissingOperator, 2, 28},
        {ValuascriptErrorCode::TrailingCommaInFunctionCall, 3, 28},
        {ValuascriptErrorCode::MissingFunctionName, 4, 6}
        }
        },

        ParserErrorsSynchronizationTestCase{
        "TypeAnnotationErrors",
        "let a: = 10\n"
        "func bad_return() -> { }\n",
        {
        {ValuascriptErrorCode::MissingTypeAnnotation, 1, 8},
        {ValuascriptErrorCode::MissingTypeAnnotationAfterArrow, 2, 22}
        }
        },

        ParserErrorsSynchronizationTestCase{
        "MalformedExpressions",
        "let a = 5 +\n"
        "let b = - \n"
        "let c = 10 * / 5\n"
        "let d = (10 + 5] \n",
        {
        {ValuascriptErrorCode::InvalidExpression, 1, 12},
        {ValuascriptErrorCode::InvalidExpression, 2, 10},
        {ValuascriptErrorCode::InvalidExpression, 3, 14},
        {ValuascriptErrorCode::ExpectedRightParenAfterExpression, 4, 16}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "TopLevelGarbageAndStrayTokens",
        "} \n"
        "let valid = 10\n"
        "] \n"
        "func valid2() -> int { return 0 }\n"
        "* let bad_start = 0\n",
        {
        {ValuascriptErrorCode::UnexpectedTopLevelToken, 1, 1},
        {ValuascriptErrorCode::UnexpectedTopLevelToken, 2, 15}, // Tripped by the '\n' at the end of line 2
        {ValuascriptErrorCode::UnexpectedTopLevelToken, 4, 34} // Tripped by the '\n' at the end of line 4
        }
        },
        ParserErrorsSynchronizationTestCase{
        "Regression_2",
        "func test() -> scalar { return 1\n"
        "// -- R&D Capitalization --\n"
        "let value_of_research_assets, current_year_amortization = get_rd()\n"
        "// -- WACC --\n"
        "let wacc = get_wacc()\n"
        "enum Scenario: scalar { LOW, BASE, HIGH }\n",
        {
        {ValuascriptErrorCode::TopLevelDeclarationInsideFunction, 6, 1}
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
        {ValuascriptErrorCode::TrailingSeparatorInNumberLiteral, 1, 15},
        {ValuascriptErrorCode::ExpectedArgumentNameOrClosingParen, 4, 66}
        }
        },
        ParserErrorsSynchronizationTestCase{
        "Regression_4",
        "let a = {1, 2, 3}\n",
        {
        {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 10},
        {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 13},
        {ValuascriptErrorCode::ExpectedDictionaryKey, 1, 16}
        }
        }
    ),
    [](const ::testing::TestParamInfo<ParserErrorsSynchronizationTestCase>& info) {
    return info.param.test_name;
    }
);
