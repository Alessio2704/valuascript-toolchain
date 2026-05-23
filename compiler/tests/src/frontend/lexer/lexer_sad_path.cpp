#include <gtest/gtest.h>

#include "lexer_tests_utils.h"
#include "frontend/lexer/lexer_stage.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    using E = LexerErrorCode;

    struct SadLexerParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class LexerSadPathTest : public testing::TestWithParam<SadLexerParam>
    {
    };

    TEST_P(LexerSadPathTest, ThrowsCorrectLexicalError)
    {
        const SadLexerParam& param = GetParam();

        try
        {
            test::tokenize_code(param.source_code);
            FAIL() << "Lexer should have thrown an exception for test: " << param.test_name;
        }
        catch (const ValuaScriptException& e)
        {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Lexical)
            << "Error category mismatch on test: " << param.test_name;
            EXPECT_TRUE(e.is_error(param.expected_error))
            << "Error code mismatch on test: " << param.test_name;
        }
        catch (...)
        {
            FAIL() << "Lexer threw an unknown exception type for test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        LexerStageTest,
        LexerSadPathTest,
        testing::Values(
            SadLexerParam{"invalid_char_dollar", "let a = $", E::InvalidCharacter},
            SadLexerParam{"invalid_char_ampersand", "let a = &", E::InvalidCharacter},

            SadLexerParam{"percentage_before_number", "x = %1", E::InvalidCharacter},
            SadLexerParam{"unterminated_decimal_1", "let a = 1.", E::UnterminatedDecimal},
            SadLexerParam{"unterminated_decimal_2", "let a = 1_230.", E::UnterminatedDecimal},
            SadLexerParam{"unterminated_number_after_separator", "let a = 1_", E::
            TrailingSeparatorInNumberLiteral},
            SadLexerParam{"at_double_underscore_invalid", "let a = 1__000", E::
            TrailingSeparatorInNumberLiteral},

            SadLexerParam{"unclosed_string_1", "let a = \"hello", E::UnclosedString},
            SadLexerParam{"unclosed_string_2", "let a = \"hello \n \n", E::UnclosedString},
            SadLexerParam{"unclosed_string_3", "\"hello", E::UnclosedString},
            SadLexerParam{"unclosed_string_import", "import \"file/path", E::UnclosedString},
            SadLexerParam{"unclosed_docstring_1", "func test() -> scalar { \"\"\"\"\" return 1 }", E
            ::
            UnclosedString},
            SadLexerParam{"unclosed_docstring_2", "\"\"\"\"\" return 1 }", E::UnclosedString}
        ),
        [](const testing::TestParamInfo<SadLexerParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
