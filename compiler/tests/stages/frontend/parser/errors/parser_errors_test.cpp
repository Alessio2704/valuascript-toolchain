#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/parser_stage.h"
#include "compiler_stage/compiler_context.h"
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
        {ErrorCode::ExpectedModifierName, 1, 3},
        {ErrorCode::InvalidIdentifier, 2, 5},
        {ErrorCode::ReservedKeywordAsIdentifier, 3, 5},
        {ErrorCode::ExpectedStructName, 3, 12},
        {ErrorCode::IncompleteAssignment, 4, 6}
        }
        },
        ParserMultiErrorTestCase{
        "FunctionDefinitionErrors",
        "func (a: int) -> int {}\n"
        "func test1(a int) -> int {}\n"
        "func test2(b: int) int {}\n",
        {
        {ErrorCode::MissingFunctionName, 1, 6},
        {ErrorCode::MissingColonAfterParameter, 2, 14},
        {ErrorCode::MissingArrowInFunction, 3, 20}
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
        {ErrorCode::ExpectedStructName, 1, 8},
        {ErrorCode::ExpectedBraceInStructDefinition, 2, 13},
        {ErrorCode::ExpectedColonAfterStructFieldName, 3, 18},
        {ErrorCode::ExpectedEnumName, 4, 6},
        {ErrorCode::ExpectedColonAfterEnumName, 5, 12}
        }
        },
        ParserMultiErrorTestCase{
        "DataStructureErrors",
        "let b = { \"key\" 10 }\n"
        "let b = { key 10 }\n"
        "let c = obj[]\n"
        "let d = obj.",
        {
        {ErrorCode::ExpectedDictionaryKey, 1, 11},
        {ErrorCode::ExpectedColonAfterDictionaryKey, 2, 15},
        {ErrorCode::EmptyBracketAccess, 3, 12},
        {ErrorCode::ExpectedPropertyName, 4, 13}
        }
        },
        ParserMultiErrorTestCase{
        "ControlFlowAndExpressionErrors",
        "let a = if (true) 10 else 20\n"
        "let x = 10 < 20 < 30\n"
        "let a = switch (val) { case 1 10 }\n"
        "let a = switch (val) { default -> 1 default -> 2 }\n",
        {
        {ErrorCode::MissingThenToken, 1, 19},
        {ErrorCode::ChainingNotAllowedForComparisonOperations, 2, 17},
        {ErrorCode::ExpectedEnumCaseName, 3, 29},
        {ErrorCode::MultipleDefaultCasesInSwitch, 4, 45}
        }
        },
        ParserMultiErrorTestCase{
        "DeepSynchronizationStressTest",
        "let valid1 = 100\n"
        "let broken1 = (10 + \n"
        "garbage token + - * / \n"
        "func valid_func() -> int {}\n"
        "struct { }\n"
        "let valid2 = 200\n",
        {
        {ErrorCode::ExpectedRightParen, 3, 9},
        {ErrorCode::ExpectedStructName, 5, 8}
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
        {ErrorCode::ExpectedRightParen, 1, 17},
        {ErrorCode::UnmatchedBracket, 4, 17},
        {ErrorCode::UnmatchedBraceInDictionaryLiteral, 6, 23}
        }
        },
        ParserMultiErrorTestCase{
        "Regression_1",
        "let a = (10 + 20 \n"
        "let b = { \"key\" 10 }\n"
        "let a = func_call(\n\n"
        "let b = some_other()",
        {
        {ErrorCode::ExpectedRightParen, 1, 17},
        {ErrorCode::ExpectedDictionaryKey, 2, 11},
        {ErrorCode::MissingArgumentName, 3, 19}
        }
        },
        ParserMultiErrorTestCase{
        "FunctionCallAndSignatureErrors",
        "func process(a: int b: string) -> int {}\n"
        "let result = process(a: 10 20)\n"
        "let x = missing_args(a: 1, , 3)\n",
        {
        {ErrorCode::ExpectedRightParen, 1, 21},
        {ErrorCode::ExpectedRightParen, 2, 28},
        {ErrorCode::MissingArgumentName, 3, 28}
        }
        },

        ParserMultiErrorTestCase{
        "TypeAnnotationErrors",
        "let a: = 10\n"
        "func bad_return() -> { }\n",
        {
        {ErrorCode::MissingTypeAnnotation, 1, 8},
        {ErrorCode::MissingTypeAnnotation, 2, 22}
        }
        },

        ParserMultiErrorTestCase{
        "MalformedExpressions",
        "let a = 5 +\n"
        "let b = - \n"
        "let c = 10 * / 5\n"
        "let d = (10 + 5] \n",
        {
        {ErrorCode::InvalidExpression, 1, 12},
        {ErrorCode::InvalidExpression, 2, 10},
        {ErrorCode::InvalidExpression, 3, 14},
        {ErrorCode::ExpectedRightParen, 4, 16}
        }
        },

        ParserMultiErrorTestCase{
        "AdvancedStructAndEnumErrors",
        "struct Config { host: string port: int }\n"
        "enum State: string { One Two }\n"
        "struct Empty { : int }\n"
        "enum State { One, Two }\n",
        {
        {ErrorCode::ExpectedBraceInStructDefinition, 1, 30},
        {ErrorCode::ExpectedRightBrace, 2, 26},
        {ErrorCode::ExpectedStructFieldName, 3, 16},
        {ErrorCode::ExpectedColonAfterEnumName, 4, 12}
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
        {ErrorCode::UnexpectedToken, 1, 1},
        {ErrorCode::UnexpectedToken, 2, 15}, // Tripped by the '\n' at the end of line 2
        {ErrorCode::UnexpectedToken, 4, 34} // Tripped by the '\n' at the end of line 4
        }
        }
    ),
    [](const ::testing::TestParamInfo<ParserMultiErrorTestCase>& info) {
    return info.param.test_name;
    }
);
