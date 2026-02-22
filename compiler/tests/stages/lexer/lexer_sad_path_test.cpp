#include <gtest/gtest.h>
#include "stages/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct SadLexerParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class LexerSadPathTest : public testing::TestWithParam<SadLexerParam> {
protected:
    static void run_lexer(const std::string& code) {
        LexerStage lexer;
        std::vector<CompilerStageArtifact> history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        };
        lexer.run(history);
    }
};

TEST_P(LexerSadPathTest, ThrowsCorrectLexicalError) {
    const SadLexerParam& param = GetParam();

    try {
        run_lexer(param.source_code);
        FAIL() << "Lexer should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException& e) {
        EXPECT_EQ(e.get_category(), ErrorCategory::Lexical)
            << "Error category mismatch on test: " << param.test_id;
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    } catch (...) {
        FAIL() << "Lexer threw an unknown exception type for test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    LexerStageTest,
    LexerSadPathTest,
    testing::Values(
        // Invalid standalone characters
        SadLexerParam{"invalid_char_dollar", "let a = $", ErrorCode::InvalidCharacter},
        SadLexerParam{"invalid_char_ampersand", "let a = &", ErrorCode::InvalidCharacter},

        // Malformed Numbers
        SadLexerParam{"at_wrong_identifier_1", "let a = 1.", ErrorCode::InvalidCharacter},
        SadLexerParam{"at_wrong_identifier_2", "let a = .5", ErrorCode::InvalidCharacter},
        SadLexerParam{"at_wrong_identifier_3", "let a = 1_", ErrorCode::InvalidCharacter},
        SadLexerParam{"at_double_underscore_invalid", "let a = 1__000", ErrorCode::InvalidCharacter},

        // Unclosed Strings
        SadLexerParam{"unclosed_string_1", "let a = \"hello", ErrorCode::UnclosedString},
        SadLexerParam{"unclosed_docstring", "func test() -> scalar { \"\"\"\"\" return 1 }", ErrorCode::UnclosedString}
    ),
    [](const testing::TestParamInfo<SadLexerParam>& info) {
        return info.param.test_id;
    }
);