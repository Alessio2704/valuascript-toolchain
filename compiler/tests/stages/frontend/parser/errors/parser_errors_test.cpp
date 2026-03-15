#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/parser_stage.h"
#include "../../../../../include/compiler_context/compiler_context.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::compiler;

struct ExpectedParserError {
    ErrorCode code;
    size_t line;
    size_t column;
};

struct ParserMultiErrorTestCase {
    std::string test_name;
    std::string source_code;
    std::vector<ExpectedParserError> expected_errors;
};

class ParserMultiErrorTest : public testing::TestWithParam<ParserMultiErrorTestCase> {
protected:
    static void run_parser_and_check_errors(const ParserMultiErrorTestCase &param) {
        auto context = std::make_shared<CompilerContext>();
        context->settings.fail_fast = false;

        std::vector<CompilerStageArtifact> initial_artifacts = {
            {CompilerStageArtifactCode::SourceCode, param.source_code},
            {CompilerStageArtifactCode::FilePath, std::string("test_script.vs")}
        };

        LexerStage lexer;
        ParserStage parser;

        auto lexer_artifacts = initial_artifacts;
        lexer_artifacts.push_back(lexer.run(context, initial_artifacts));

        ASSERT_NO_THROW({
            parser.run(context, lexer_artifacts);
            }) << "Parser threw an exception even though fail_fast was set to false.";

        const auto &actual_errors = context->diagnostics.get_errors();

        ASSERT_EQ(actual_errors.size(), param.expected_errors.size())
            << "Mismatch in the number of collected errors.\n"
            << "Expected " << param.expected_errors.size() << ", but got " << actual_errors.size();

        for (size_t i = 0; i < actual_errors.size(); ++i) {
            const auto &actual = actual_errors[i];
            const auto &expected = param.expected_errors[i];
            std::cout << actual.what();

            EXPECT_EQ(actual.get_category(), ErrorCategory::Syntax);
            EXPECT_EQ(actual.get_code(), expected.code)
                << "Error [" << i << "] Code mismatch.\nExpected Code: " << static_cast<int>(expected.code)
                << "\nActual Code: " << static_cast<int>(actual.get_code())
                << "\nActual Message: " << actual.what();

            EXPECT_EQ(actual.get_location().line_start, expected.line)
                << "Error [" << i << "] Line mismatch for error: " << actual.what();

            EXPECT_EQ(actual.get_location().column_end, expected.column)
                << "Error [" << i << "] Column mismatch for error: " << actual.what();
        }
    }
};

TEST_P(ParserMultiErrorTest, CollectsMultipleSyntaxErrorsAtCorrectLocations) {
    run_parser_and_check_errors(GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    ParserExhaustiveStressTests,
    ParserMultiErrorTest,
    ::testing::Values(
        ParserMultiErrorTestCase{
        "VariableDeclarationErrors",
        "@ let a = 10\n"
        "let = 20\n"
        "let struct = 30\n"
        "let d",
        {
        {ErrorCode::ExpectedModifierName, 1, 6},
        {ErrorCode::InvalidIdentifier, 2, 6},
        {ErrorCode::ReservedKeywordAsIdentifier, 3, 11},
        {ErrorCode::ExpectedStructName, 3, 13},
        {ErrorCode::IncompleteAssignment, 4, 7}
        }
        },
        ParserMultiErrorTestCase{
        "FunctionDefinitionErrors",
        "func (a: int) -> int {}\n"
        "func test1(a int) -> int {}\n"
        "func test2(b: int) int {}\n",
        {
        {ErrorCode::MissingFunctionName, 1, 7},
        {ErrorCode::MissingColonAfterParameter, 2, 17},
        {ErrorCode::MissingArrowInFunction, 3, 23}
        }
        },
        ParserMultiErrorTestCase{
        "StructAndEnumErrors",
        "struct { id: int }\n"
        "struct User id: int }\n"
        "struct Point { x int }\n"
        "enum { A }\n"
        "enum Color { A, }\n",
        {
        {ErrorCode::ExpectedStructName, 1, 9},
        {ErrorCode::ExpectedBraceInStructDefinition, 2, 15},
        {ErrorCode::ExpectedColonAfterStructFieldName, 3, 21},
        {ErrorCode::ExpectedEnumName, 4, 7},
        {ErrorCode::ExpectedColonAfterEnumName, 5, 13}
        }
        },
        ParserMultiErrorTestCase{
        "DataStructureErrors",
        "let b = { \"key\" 10 }\n"
        "let b = { key 10 }\n"
        "let c = obj[]\n"
        "let d = obj.",
        {
        {ErrorCode::ExpectedDictionaryKey, 1, 16},
        {ErrorCode::ExpectedColonAfterDictionaryKey, 2, 17},
        {ErrorCode::EmptyBracketAccess, 3, 13},
        {ErrorCode::ExpectedPropertyName, 4, 14}
        }
        },
        ParserMultiErrorTestCase{
        "ControlFlowAndExpressionErrors",
        "let a = if (true) 10 else 20\n"
        "let x = 10 < 20 < 30\n"
        "let a = switch (val) { case 1 10 }\n"
        "let a = switch (val) { default -> 1 default -> 2 }\n",
        {
        {ErrorCode::MissingThenToken, 1, 21},
        {ErrorCode::ChainingNotAllowedForComparisonOperations, 2, 18},
        {ErrorCode::ExpectedEnumCaseNameAfterCase, 3, 30},
        {ErrorCode::MultipleDefaultCasesInSwitch, 4, 47}
        }
        },
        ParserMultiErrorTestCase{
        "DeepSynchronizationStressTest",
        "let valid1 = 100\n"
        "let broken1 = (10  2)\n"
        "func valid_func() -> int {}\n"
        "struct { }\n"
        "let valid2 = 200\n"
        "let a = { a: 1 b: 2}",
        {
        {ErrorCode::MissingOperatorInsideGrouping, 2, 21},
        {ErrorCode::ExpectedStructName, 4, 9},
        {ErrorCode::ExpectedCommaSeparatorInDictionaryLiteral, 6, 17}
        }
        },
        ParserMultiErrorTestCase{
        "UnterminatedParenthesisBracketsOrBraces",
        "let a = (10 + 20 \n"
        "let valid_1 = 100\n"
        "let valid_2 = 200\n"
        "let b = [1, 2, 3 \n"
        "let valid_3 = 300\n"
        "let c = { key: \"value\" \n"
        "let valid_4 = 400\n",
        {
        {ErrorCode::ExpectedRightParenAfterExpression, 1, 18},
        {ErrorCode::UnmatchedBracketAfterVectorElements, 4, 18},
        {ErrorCode::UnmatchedBraceInDictionaryLiteral, 6, 24}
        }
        },
        ParserMultiErrorTestCase{
        "Regression_1",
        "let b = { \"key\" 10 }\n"
        "let a = func_call(\n\n"
        "let b = some_other()",
        {
        {ErrorCode::ExpectedDictionaryKey, 1, 16},
        {ErrorCode::ExpectedArgumentNameOrClosingParen, 2, 20}
        }
        },
        ParserMultiErrorTestCase{
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
        {ErrorCode::MissingOperatorOrArgumentName, 1, 15},
        {ErrorCode::MissingOperatorInsideGrouping, 2, 20},
        {ErrorCode::MissingOperatorOrArgumentName, 3, 18},
        {ErrorCode::MissingOperatorInsideGrouping, 4, 23},
        {ErrorCode::MissingOperatorInsideGrouping, 5, 23},
        {ErrorCode::MissingOperatorOrArgumentName, 6, 17},
        {ErrorCode::MissingOperatorInsideGrouping, 7, 25},
        {ErrorCode::MissingOperatorInsideGrouping, 8, 25},
        {ErrorCode::MissingOperatorOrArgumentName, 10, 19},
        {ErrorCode::MissingOperatorOrArgumentName, 13, 19},
        {ErrorCode::MissingOperatorOrArgumentName, 15, 14},
        {ErrorCode::MissingOperatorOrArgumentName, 16, 14},
        {ErrorCode::MissingOperatorOrArgumentName, 17, 14},
        {ErrorCode::MissingOperatorOrArgumentName, 18, 14},
        {ErrorCode::MissingOperatorOrArgumentName, 19, 38},
        }
        },
        ParserMultiErrorTestCase{
        "FunctionCallAndSignatureErrors",
        "func process(a: int b: string) -> int {}\n"
        "let result = process(a: 10 20)\n"
        "let x = trailing_comma(a: 1, )\n"
        "func true() -> { return }",
        {
        {ErrorCode::ExpectedCommaSeparatorInParameterList, 1, 22},
        {ErrorCode::MissingOperator, 2, 30},
        {ErrorCode::TrailingCommaInFunctionCall, 3, 29},
        {ErrorCode::MissingFunctionName, 4, 10}
        }
        },

        ParserMultiErrorTestCase{
        "TypeAnnotationErrors",
        "let a: = 10\n"
        "func bad_return() -> { }\n",
        {
        {ErrorCode::MissingTypeAnnotation, 1, 9},
        {ErrorCode::MissingTypeAnnotationAfterArrow, 2, 23}
        }
        },

        ParserMultiErrorTestCase{
        "MalformedExpressions",
        "let a = 5 +\n"
        "let b = - \n"
        "let c = 10 * / 5\n"
        "let d = (10 + 5] \n",
        {
        {ErrorCode::InvalidExpression, 1, 13},
        {ErrorCode::InvalidExpression, 2, 11},
        {ErrorCode::InvalidExpression, 3, 15},
        {ErrorCode::ExpectedRightParenAfterExpression, 4, 17}
        }
        },

        ParserMultiErrorTestCase{
        "AdvancedStructAndEnumErrors",
        "struct Config { host: string port: int }\n"
        "enum State: string { One Two }\n"
        "struct Empty { : int }\n"
        "enum State { One, Two }\n",
        {
        {ErrorCode::ExpectedCommaSeparatorInStruct, 1, 34},
        {ErrorCode::ExpectedCommaSeparatorInEnum, 2, 29},
        {ErrorCode::ExpectedStructFieldName, 3, 17},
        {ErrorCode::ExpectedColonAfterEnumName, 4, 13}
        }
        },

        ParserMultiErrorTestCase{
        "TopLevelGarbageAndStrayTokens",
        "} \n"
        "let valid = 10\n"
        "] \n"
        "func valid2() -> int { return 0 }\n"
        "* let bad_start = 0\n",
        {
        {ErrorCode::UnexpectedTopLevelToken, 1, 2},
        {ErrorCode::UnexpectedTopLevelToken, 2, 16}, // Tripped by the '\n' at the end of line 2
        {ErrorCode::UnexpectedTopLevelToken, 4, 35} // Tripped by the '\n' at the end of line 4
        }
        },
        ParserMultiErrorTestCase{
        "Regression_2",
        "func test() -> scalar { return 1\n"
        "// -- R&D Capitalization --\n"
        "let value_of_research_assets, current_year_amortization = get_rd()\n"
        "// -- WACC --\n"
        "let wacc = get_wacc()\n"
        "enum Scenario: scalar { LOW, BASE, HIGH }\n",
        {
        {ErrorCode::TopLevelDeclarationInsideFunction, 6, 5}
        }
        }
    ),
    [](const ::testing::TestParamInfo<ParserMultiErrorTestCase>& info) {
    return info.param.test_name;
    }
);
