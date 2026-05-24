#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
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
            "let @* a = 10\n"
            "let = 20\n"
            "let struct = 30\n"
            "let d",
            {
            {Err::ExpectedModifierName, 1, 6},
            {Err::ExpectedIdentifier, 2, 5},
            {Err::ReservedKeywordAsIdentifier, 3, 5},
            {Err::IncompleteAssignment, 4, 6}
            }
            },
            ParserErrorsSynchronizationTestCase{
            "FunctionDefinitionErrors",
            "func (a: int) -> int {}\n"
            "func test1(a int) -> int {}\n"
            "func test2(b: int) int {}\n",
            {
            {Err::MissingFunctionName, 1, 6},
            {Err::MissingColonAfterParameter, 2, 14},
            {Err::MissingArrowInFunction, 3, 20}
            }
            },
            ParserErrorsSynchronizationTestCase{
            "DataStructureErrors",
            "let b = { \"key\" 10 }\n"
            "let b = { key 10 }\n"
            "let c = obj[]\n"
            "let d = obj.",
            {
            {Err::ExpectedDictionaryKey, 1, 11},
            {Err::ExpectedColonAfterDictionaryKey, 2, 15},
            {Err::EmptyBracketAccess, 3, 12},
            {Err::ExpectedPropertyName, 4, 13}
            }
            },
            ParserErrorsSynchronizationTestCase{
            "ControlFlowAndExpressionErrors",
            "let a = if (true) 10 else 20\n"
            "let x = 10 < 20 < 30\n"
            "let a = switch (val) { case 1 10 }\n"
            "let a = switch (val) { default -> 1 default -> 2 }\n",
            {
            {Err::MissingThenToken, 1, 19},
            {Err::ChainingNotAllowedForComparisonOperations, 2, 17},
            {Err::ExpectedEnumCaseNameAfterCase, 3, 29},
            {Err::ExpectedRightArrowAfterSwitchCaseIdentifier, 3, 34},
            {Err::MultipleDefaultCasesInSwitch, 4, 37}
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
            {Err::MissingOperatorInsideGrouping, 2, 20},
            {Err::ExpectedStructName, 4, 8},
            {Err::ExpectedCommaSeparatorInDictionaryLiteral, 6, 16}
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
            {Err::ExpectedRightParenAfterExpression, 1, 17},
            {Err::UnmatchedBracketAfterTensorElements, 4, 17},
            {Err::UnmatchedBraceInDictionaryLiteral, 6, 23}
            }
            },
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
            {Err::MissingOperatorOrArgumentName, 1, 14},
            {Err::MissingOperatorInsideGrouping, 2, 19},
            {Err::MissingOperatorOrArgumentName, 3, 17},
            {Err::MissingOperatorInsideGrouping, 4, 22},
            {Err::MissingOperatorInsideGrouping, 5, 22},
            {Err::MissingOperatorOrArgumentName, 6, 16},
            {Err::MissingOperatorInsideGrouping, 7, 24},
            {Err::MissingOperatorInsideGrouping, 8, 24},
            {Err::MissingOperatorOrArgumentName, 10, 18},
            {Err::MissingOperatorOrArgumentName, 13, 18},
            {Err::MissingOperatorOrArgumentName, 15, 13},
            {Err::MissingOperatorOrArgumentName, 16, 13},
            {Err::MissingOperatorOrArgumentName, 17, 13},
            {Err::MissingOperatorOrArgumentName, 18, 13},
            {Err::MissingOperatorOrArgumentName, 19, 37},
            }
            },
            ParserErrorsSynchronizationTestCase{
            "FunctionCallAndSignatureErrors",
            "func process(a: int b: string) -> int {}\n"
            "let result = process(a: 10 20)\n"
            "let x = trailing_comma(a: 1, )\n"
            "func true() -> void { return 1}",
            {
            {Err::ExpectedCommaSeparatorInParameterList, 1, 21},
            {Err::MissingOperator, 2, 28},
            {Err::TrailingCommaInFunctionCall, 3, 28},
            {Err::ReservedKeywordAsIdentifier, 4, 6}
            }
            },

            ParserErrorsSynchronizationTestCase{
            "TypeAnnotationErrors",
            "let a: = 10\n"
            "func bad_return() -> { }\n",
            {
            {Err::MissingTypeAnnotation, 1, 8},
            {Err::MissingTypeAnnotationAfterArrow, 2, 22}
            }
            },

            ParserErrorsSynchronizationTestCase{
            "MalformedExpressions",
            "let a = 5 +\n"
            "let b = - \n"
            "let c = 10 * / 5\n"
            "let d = (10 + 5] \n",
            {
            {Err::InvalidExpression, 1, 11},
            {Err::InvalidExpression, 2, 9},
            {Err::InvalidExpression, 3, 14},
            {Err::ExpectedRightParenAfterExpression, 4, 16}
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
            {Err::InvalidExpression, 1, 1},
            {Err::InvalidExpression, 3, 1},
            {Err::InvalidExpression, 5, 1}
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
            {Err::ExpectedRightBraceAfterFunctionBody, 5, 22}
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
