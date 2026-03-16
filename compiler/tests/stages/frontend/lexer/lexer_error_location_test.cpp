#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "stages/frontend/lexer/lexer_stage.h"
#include "../../../../include/compiler_context/compiler_context.h"
#include "errors/valuascript_exception.h"

using namespace valuascript::compiler;

struct ExpectedError {
    ErrorCode code;
    size_t line;
    size_t column;
};

struct LexerMultiErrorTestCase {
    std::string test_name;
    std::string source_code;
    std::vector<ExpectedError> expected_errors;
};

class LexerMultiErrorTest : public testing::TestWithParam<LexerMultiErrorTestCase> {
protected:
    static void run_lexer_and_check_errors(const LexerMultiErrorTestCase &param) {
        auto context = std::make_shared<CompilerContext>();
        context->settings.fail_fast = false;

        std::vector<CompilerStageArtifact> artifacts = {
            {CompilerStageArtifactCode::SourceCode, param.source_code},
            {CompilerStageArtifactCode::FilePath, std::string("test_script.vs")}
        };

        LexerStage lexer;

        ASSERT_NO_THROW({
            lexer.run(context, artifacts);
            }) << "Lexer threw an exception even though fail_fast was set to false.";

        const auto &actual_errors = context->diagnostics.get_errors();

        ASSERT_EQ(actual_errors.size(), param.expected_errors.size())
            << "Mismatch in the number of collected errors.\n"
            << "Expected " << param.expected_errors.size() << ", but got " << actual_errors.size();

        for (size_t i = 0; i < actual_errors.size(); ++i) {
            const auto &actual = actual_errors[i];
            const auto &expected = param.expected_errors[i];

            EXPECT_EQ(actual.get_category(), ErrorCategory::Lexical);

            EXPECT_EQ(actual.get_code(), expected.code)
                << "Error [" << i << "] Code mismatch.\nExpected Code: " << static_cast<int>(expected.code)
                << "\nActual Code: " << static_cast<int>(actual.get_code())
                << "\nActual Message: " << actual.what();

            EXPECT_EQ(actual.get_span().line_start, expected.line)
                << "Error [" << i << "] Line mismatch for error: " << actual.what();

            EXPECT_EQ(actual.get_span().column_start, expected.column)
                << "Error [" << i << "] Column mismatch for error: " << actual.what();
        }
    }
};

TEST_P(LexerMultiErrorTest, CollectsMultipleErrorsAtCorrectLocations) {
    run_lexer_and_check_errors(GetParam());
}

INSTANTIATE_TEST_SUITE_P(
    LexerStressTests,
    LexerMultiErrorTest,
    ::testing::Values(
        LexerMultiErrorTestCase{
        "MultipleInvalidCharacters",
        "let a = $\n"
        "let b = ~\n"
        "let c = \\",
        {
        {ErrorCode::InvalidCharacter, 1, 9},
        {ErrorCode::InvalidCharacter, 2, 9},
        {ErrorCode::InvalidCharacter, 3, 9}
        }
        },
        LexerMultiErrorTestCase{
        "MixedTokenErrors",
        "let w = 12.\n"
        "let x = .5\n"
        "let y = \"unclosed string spanning to EOF",
        {
        {ErrorCode::UnterminatedDecimal, 1, 9},
        {ErrorCode::DecimalMissingLeadingZero, 2, 9},
        {ErrorCode::UnclosedString, 3, 9}
        }
        },
        LexerMultiErrorTestCase{
        "SuccessiveInvalidCharacters",
        "$$$\n"
        "~~~",
        {
        {ErrorCode::InvalidCharacter, 1, 1},
        {ErrorCode::InvalidCharacter, 1, 2},
        {ErrorCode::InvalidCharacter, 1, 3},
        {ErrorCode::InvalidCharacter, 2, 1},
        {ErrorCode::InvalidCharacter, 2, 2},
        {ErrorCode::InvalidCharacter, 2, 3}
        }
        },
        LexerMultiErrorTestCase{
        "InvalidUnderscoresInNumbers",
        "let num1 = 12__3\n"
        "let num2 = 45._\n"
        "let num3 = 100_",
        {
        {ErrorCode::InvalidCharacter, 1, 12},
        {ErrorCode::UnterminatedDecimal, 2, 12},
        {ErrorCode::InvalidCharacter, 3, 12}
        }
        },

        LexerMultiErrorTestCase{
        "LargeScaleStressTest",
        "\n"
        "\n"
        "let value = 100 \n"
        "let invalid1 = $  \n"
        "let partial = 0.\n"
        "let invalid2 = ~\n"
        "\n"
        "let unclosed = \"started"
        "\n",
        {
        {ErrorCode::InvalidCharacter, 4, 16},
        {ErrorCode::UnterminatedDecimal, 5, 15},
        {ErrorCode::InvalidCharacter, 6, 16},
        {ErrorCode::UnclosedString, 8, 16}
        }
        },
        LexerMultiErrorTestCase{
        "UnclosedDocString",
        "let doc = \"\"\"This is a docstring\n"
        "that spans multiple lines\n"
        "but never closes properly...",
        {
        {ErrorCode::UnclosedString, 1, 11}
        }
        },
        LexerMultiErrorTestCase{
        "HiddenLexicalErrorsInMath",
        "let result = 10 + .5 * 100_ - \"unclosed",
        {
        {ErrorCode::DecimalMissingLeadingZero, 1, 19},
        {ErrorCode::InvalidCharacter, 1, 24},
        {ErrorCode::UnclosedString, 1, 31}
        }
        },
        LexerMultiErrorTestCase{
        "InterleavedValidAndInvalid",
        "let valid1 = 100\n"
        "let bad1 = $\n"
        "let valid2 = \"test\"\n"
        "let bad2 = ~\n"
        "let bad3 = .99\n"
        "let valid3 = 0.99",
        {
        {ErrorCode::InvalidCharacter, 2, 12},
        {ErrorCode::InvalidCharacter, 4, 12},
        {ErrorCode::DecimalMissingLeadingZero, 5, 12}
        }
        },
        LexerMultiErrorTestCase{
        "RapidFireCorruptions",
        "\"unclosed 1\n"
        "12.\n"
        "\"unclosed 2\n"
        ".9\n"
        "1_a",
        {
        {ErrorCode::UnclosedString, 1, 1},
        {ErrorCode::UnterminatedDecimal, 2, 1},
        {ErrorCode::UnclosedString, 3, 1},
        {ErrorCode::DecimalMissingLeadingZero, 4, 1},
        {ErrorCode::InvalidCharacter, 5, 1}
        }
        },
        LexerMultiErrorTestCase{
        "WhitespaceAndIndentationTracking",
        "\n\n"
        "\t\tlet spaced =   $\n"
        "    let padded = .123\n"
        "    let carriage = 12.\n",
        {
        {ErrorCode::InvalidCharacter, 3, 18},
        {ErrorCode::DecimalMissingLeadingZero, 4, 18},
        {ErrorCode::UnterminatedDecimal, 5, 20}
        }
        },
        LexerMultiErrorTestCase{
        "PureGarbageFile",
        "?\\`\n"
        "~|\n"
        "$$$",
        {
        {ErrorCode::InvalidCharacter, 1, 1},
        {ErrorCode::InvalidCharacter, 1, 2},
        {ErrorCode::InvalidCharacter, 1, 3},
        {ErrorCode::InvalidCharacter, 2, 1},
        {ErrorCode::InvalidCharacter, 2, 2},
        {ErrorCode::InvalidCharacter, 3, 1},
        {ErrorCode::InvalidCharacter, 3, 2},
        {ErrorCode::InvalidCharacter, 3, 3}
        }
        }
    ),
    [](const ::testing::TestParamInfo<LexerMultiErrorTestCase>& info) {
    return info.param.test_name;
    }
);
