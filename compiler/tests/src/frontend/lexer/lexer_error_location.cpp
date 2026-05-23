#include <gtest/gtest.h>
#include <memory>
#include <vector>

#include "frontend/lexer/lexer_stage.h"
#include "core/compiler_context.h"
#include "core/valuascript_exception.h"
#include "utils/parametrised_test_name_helper.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    using E = LexerErrorCode;

    struct LexerExpectedError
    {
        E code;
        size_t line;
        size_t column;
    };

    struct LexerMultiErrorTestCase
    {
        std::string test_name;
        std::string source_code;
        std::vector<LexerExpectedError> expected_errors;
    };

    class LexerMultiErrorTest : public testing::TestWithParam<LexerMultiErrorTestCase>
    {
    protected:
        static void run_lexer_and_check_errors(const LexerMultiErrorTestCase& param)
        {
            auto context = std::make_shared<CompilerContext>();
            context->settings.fail_fast = false;

            std::vector<CompilerStageArtifact> artifacts = {
                {CompilerStageArtifactCode::SourceCode, param.source_code},
                {CompilerStageArtifactCode::FilePath, std::string("test_script.vs")}
            };

            LexerStage lexer;

            ASSERT_NO_THROW({
                lexer.run(*context, artifacts);
                }) << "Lexer threw an exception even though fail_fast was set to false.";

            const auto& actual_errors = context->diagnostics.get_errors();

            ASSERT_EQ(actual_errors.size(), param.expected_errors.size())
            << "Mismatch in the number of collected errors.\n"
            << "Expected " << param.expected_errors.size() << ", but got " << actual_errors.size();

            for (size_t i = 0; i < actual_errors.size(); ++i)
            {
                const auto& actual = actual_errors[i];
                const auto& expected = param.expected_errors[i];

                EXPECT_EQ(actual.get_category(), ValuascriptErrorCategory::Lexical);

                EXPECT_TRUE(actual.is_error(expected.code))
                << "Error [" << i << "] Code mismatch."
                << "\nExpected Code: " << static_cast<int>(expected.code)
                << "\nActual Code: " << actual.get_error_number()
                << "\nActual Message: " << actual.what();

                EXPECT_EQ(actual.get_span().line_start, expected.line)
                << "Error [" << i << "] Line mismatch for error: " << actual.what();

                EXPECT_EQ(actual.get_span().column_start, expected.column)
                << "Error [" << i << "] Column mismatch for error: " << actual.what();
            }
        }
    };

    TEST_P(LexerMultiErrorTest, CollectsMultipleErrorsAtCorrectLocations)
    {
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
            {E::InvalidCharacter, 1, 9},
            {E::InvalidCharacter, 2, 9},
            {E::InvalidCharacter, 3, 9}
            }
            },
            LexerMultiErrorTestCase{
            "MixedTokenErrors",
            "let w = 12.\n"
            "let x = .5\n"
            "let y = \"unclosed string spanning to EOF",
            {
            {E::UnterminatedDecimal, 1, 9},
            {E::UnclosedString, 3, 9}
            }
            },
            LexerMultiErrorTestCase{
            "SuccessiveInvalidCharacters",
            "$$$\n"
            "~~~",
            {
            {E::InvalidCharacter, 1, 1},
            {E::InvalidCharacter, 1, 2},
            {E::InvalidCharacter, 1, 3},
            {E::InvalidCharacter, 2, 1},
            {E::InvalidCharacter, 2, 2},
            {E::InvalidCharacter, 2, 3}
            }
            },
            LexerMultiErrorTestCase{
            "InvalidUnderscoresInNumbers",
            "let num1 = 12__3\n"
            "let num2 = 45._\n"
            "let num3 = 100_",
            {
            {E::TrailingSeparatorInNumberLiteral, 1, 12},
            {E::UnterminatedDecimal, 2, 12},
            {E::TrailingSeparatorInNumberLiteral, 3, 12}
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
            {E::InvalidCharacter, 4, 16},
            {E::UnterminatedDecimal, 5, 15},
            {E::InvalidCharacter, 6, 16},
            {E::UnclosedString, 8, 16}
            }
            },
            LexerMultiErrorTestCase{
            "UnclosedDocString",
            "let doc = \"\"\"This is a docstring\n"
            "that spans multiple lines\n"
            "but never closes properly...",
            {
            {E::UnclosedString, 1, 11}
            }
            },
            LexerMultiErrorTestCase{
            "HiddenLexicalErrorsInMath",
            "let result = 10 + .5 * 100_ - \"unclosed",
            {
            {E::TrailingSeparatorInNumberLiteral, 1, 24},
            {E::UnclosedString, 1, 31}
            }
            },
            LexerMultiErrorTestCase{
            "InterleavedValidAndInvalid",
            "let valid1 = 100\n"
            "let bad1 = $\n"
            "let valid2 = \"test\"\n"
            "let bad2 = ~\n"
            "let valid3 = .99\n"
            "let valid4 = 0.99",
            {
            {E::InvalidCharacter, 2, 12},
            {E::InvalidCharacter, 4, 12},
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
            {E::UnclosedString, 1, 1},
            {E::UnterminatedDecimal, 2, 1},
            {E::UnclosedString, 3, 1},
            {E::TrailingSeparatorInNumberLiteral, 5, 1}
            }
            },
            LexerMultiErrorTestCase{
            "WhitespaceAndIndentationTracking",
            "\n\n"
            "\t\tlet spaced =   $\n"
            "    let padded = .123\n"
            "    let carriage = 12.\n",
            {
            {E::InvalidCharacter, 3, 18},
            {E::UnterminatedDecimal, 5, 20}
            }
            },
            LexerMultiErrorTestCase{
            "PureGarbageFile",
            "?\\`\n"
            "~|\n"
            "$$$",
            {
            {E::InvalidCharacter, 1, 1},
            {E::InvalidCharacter, 1, 2},
            {E::InvalidCharacter, 1, 3},
            {E::InvalidCharacter, 2, 1},
            {E::InvalidCharacter, 2, 2},
            {E::InvalidCharacter, 3, 1},
            {E::InvalidCharacter, 3, 2},
            {E::InvalidCharacter, 3, 3}
            }
            }
        ),
        [](const ::testing::TestParamInfo<LexerMultiErrorTestCase>& test_info) {
        return test_info.param.test_name;
        }
    );
}
