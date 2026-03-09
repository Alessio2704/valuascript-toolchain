#include <gtest/gtest.h>
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/lexer/token.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct HappyLexerParam {
    std::string test_id;
    std::string source_code;
    size_t expected_token_count;
};

class LexerHappyPathTest : public testing::TestWithParam<HappyLexerParam> {
protected:
    static std::vector<Token> tokenize(const std::string& code) {
        LexerStage lexer;
        std::vector<CompilerStageArtifact> history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        };
        auto result = lexer.run(history);
        return std::any_cast<std::vector<Token>>(result.data);
    }
};

TEST_P(LexerHappyPathTest, TokenizesSuccessfully) {
    const HappyLexerParam& param = GetParam();
    
    std::vector<Token> tokens;
    EXPECT_NO_THROW({
        tokens = tokenize(param.source_code);
    }) << "Lexer threw an exception on test: " << param.test_id;

    if (!tokens.empty()) {
        EXPECT_EQ(tokens.size(), param.expected_token_count) 
            << "Token count mismatch on test: " << param.test_id;
        EXPECT_EQ(tokens.back().type, TokenType::EndOfFile) 
            << "Missing EndOfFile token on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    LexerStageTest,
    LexerHappyPathTest,
    testing::Values(
        // Assignments
        HappyLexerParam{"number", "let a = 1000", 5},
        HappyLexerParam{"number_percentage_1", "let a = 5%", 5},
        HappyLexerParam{"number_percentage_2", "let a = 0.5%", 5},
        HappyLexerParam{"number_percentage_3", "let a = 1_000.5%", 5},
        HappyLexerParam{"string", "let a = \"string\"", 5},
        HappyLexerParam{"vector_literal", "let a = [1,2,3]", 11},
        HappyLexerParam{"boolean", "let a = true", 5},
        HappyLexerParam{"cname_1", "let _a = false", 5},
        HappyLexerParam{"identifier_containing_keyword", "let ifthenelse = 1", 5},
        HappyLexerParam{"function_call", "let a = some_func()", 7},
        HappyLexerParam{"multi_assignment", "let a, b = some_func()", 9},
        HappyLexerParam{"conditional_expression", "let a = if true then 10 else 4", 10},
        
        // Operators
        HappyLexerParam{"or", "let a = x or y", 7},
        HappyLexerParam{"and", "let a = x and y", 7},
        HappyLexerParam{"not", "let a = not x", 6},
        HappyLexerParam{"eq", "let a = x == y", 7},
        HappyLexerParam{"neq", "let a = x != y", 7},
        HappyLexerParam{"gt", "let a = x > y", 7},
        HappyLexerParam{"lt", "let a = x < y", 7},
        HappyLexerParam{"gte", "let a = x >= y", 7},
        HappyLexerParam{"lte", "let a = x <= y", 7},
        HappyLexerParam{"pow", "let a = x^y", 7},
        
        // Vectors & Parenthesis
        HappyLexerParam{"access_vector_element", "let a = x[1]", 8},
        HappyLexerParam{"delete_vector_element", "let a = x[:1]", 9},
        HappyLexerParam{"parenthesis_in_assignment", "let a = (x + y) * z", 11},
        
        // Directives
        HappyLexerParam{"iterations", "#iterations = 1000", 5},
        HappyLexerParam{"output", "#output = my_var", 5},
        HappyLexerParam{"output_file", "#output_file = \"results.csv\"", 5},
        HappyLexerParam{"module", "#module", 3},

        // Import
        HappyLexerParam{"import", "import \"file/path\"", 3},

        // Structs
        HappyLexerParam{"struct", "struct Assumption { cagr: Decimal }", 8},

        // Member Access
        HappyLexerParam{"member_access", "model.cagr", 4},

        // Floats and Separators
        HappyLexerParam{"at_separator", "let a = 1_000", 5},
        HappyLexerParam{"at_float", "let a = 1.3242", 5},
        HappyLexerParam{"at_float_and_separator", "let a = 1_103.3_242", 5},
        HappyLexerParam{"at_float_signed_1", "let a = +1_103.3_242", 6}, // Lexer splits + and Number
        HappyLexerParam{"at_float_signed_2", "let a = -1_103.3_242", 6}, // Lexer splits - and Number
        
        // Functions
        HappyLexerParam{"func_basic", "func test() -> scalar { return 1 }", 11},
        HappyLexerParam{"func_params", "func test(a: scalar) -> scalar { return 1 }", 14},
        HappyLexerParam{"func_tuple_return", "func test() -> (scalar, vector) { return 1 }", 15},
        HappyLexerParam{"func_docstring", "func test() -> scalar { \"\"\"Docs\"\"\" \n return 1 }", 12},
        
        // File structure
        HappyLexerParam{"empty_file", "", 1},
        HappyLexerParam{"only_comment", "// comment", 1},
        HappyLexerParam{"tabs_and_spaces", "\t\t   ", 1}
    ),
    [](const testing::TestParamInfo<HappyLexerParam>& info) {
        return info.param.test_id;
    }
);