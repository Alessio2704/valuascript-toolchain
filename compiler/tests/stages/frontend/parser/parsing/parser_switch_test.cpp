#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/parser/ast.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserSwitchTestBase : public testing::Test {
protected:
    static std::shared_ptr<Program> parse_code(const std::string &code) {
        LexerStage lexer;
        std::vector<CompilerStageArtifact> lexer_history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        };
        auto lexer_result = lexer.run(lexer_history);

        ParserStage parser;
        std::vector<CompilerStageArtifact> parser_history = {
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        };
        auto parser_result = parser.run(parser_history);

        return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
    }
};

struct SwitchHappyParam {
    std::string test_id;
    std::string source_code;
};

class SwitchHappyPathTest : public ParserSwitchTestBase,
                            public testing::WithParamInterface<SwitchHappyParam> {
};

TEST_P(SwitchHappyPathTest, ParsesSuccessfully) {
    const SwitchHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->execution_steps.size(), 1) << "Expected exactly 1 assignment in AST.";
        EXPECT_EQ(ast->directives.size(), 0);
        EXPECT_EQ(ast->function_definitions.size(), 0);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        EXPECT_NE(assignment->value, nullptr) << "Expected assignment to have a value expression.";
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    SwitchHappyPathTest,
    testing::Values(
        SwitchHappyParam{"standard_switch", "let a = switch (res) { case UP -> 10 case DOWN -> -15 default -> 20 }"},
        SwitchHappyParam{"multi_match_cases", "let a = switch (res) { case UP, SUS -> 10 case DOWN -> 0 }"},
        SwitchHappyParam{"no_default", "let a = switch (state) { case OPEN -> 1 case CLOSED -> 0 }"},
        SwitchHappyParam{"complex_target", "let a = switch (get_status(s: p)) { case OK -> 100 }"},
        SwitchHappyParam{"complex_result", "let a = switch (res) { case UP -> base_val * 1.5 default -> 0.0 }"},
        SwitchHappyParam{"inline_argument", "let a = calculate(p: switch (res) { case UP -> 1 default -> 0 })"},
        SwitchHappyParam{"nested_switch",
        "let a = switch (x) { case A -> switch (y) { case B -> 1 default -> 0 } default -> -1 }"}
    ),
    [](const testing::TestParamInfo<SwitchHappyParam>& info) {
    return info.param.test_id;
    }
);

struct SwitchSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class SwitchSadPathTest : public ParserSwitchTestBase,
                          public testing::WithParamInterface<SwitchSadParam> {
};

TEST_P(SwitchSadPathTest, ThrowsCorrectSyntaxError) {
    const SwitchSadParam &param = GetParam();

    try {
        parse_code(param.source_code);
        FAIL() << "Parser should have thrown an exception for test: " << param.test_id;
    } catch (const ValuaScriptException &e) {
        EXPECT_EQ(e.get_category(), ErrorCategory::Syntax)
            << "Category mismatch on test: " << param.test_id;
        EXPECT_EQ(e.get_code(), param.expected_error)
            << "Error code mismatch on test: " << param.test_id;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    SwitchSadPathTest,
    testing::Values(
        SwitchSadParam{"missing_left_paren", "let a = switch res) { case UP -> 1 }", ErrorCode::ExpectedLeftParen},
        SwitchSadParam{"missing_right_paren", "let a = switch (res { case UP -> 1 }",
        ErrorCode::ExpectedRightParen},
        SwitchSadParam{"missing_left_brace", "let a = switch (res) case UP -> 1 }", ErrorCode::ExpectedLeftBrace},
        SwitchSadParam{"missing_right_brace", "let a = switch (res) { case UP -> 1", ErrorCode::ExpectedRightBrace},
        SwitchSadParam{"number_as_case", "let a = switch (res) { case 1 -> 10 }",
        ErrorCode::ExpectedEnumCaseName},
        SwitchSadParam{"string_as_case", "let a = switch (res) { case \"UP\" -> 10 }",
        ErrorCode::ExpectedEnumCaseName},
        SwitchSadParam{"expression_as_case", "let a = switch (res) { case a + b -> 10 }",
        ErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier},
        SwitchSadParam{"missing_arrow_case", "let a = switch (res) { case UP 10 }",
        ErrorCode::ExpectedRightArrowAfterSwitchCaseIdentifier},
        SwitchSadParam{"missing_arrow_default", "let a = switch (res) { default 10 }", ErrorCode::
        ExpectedRightArrowAfterSwitchCaseIdentifier}
        ,
        SwitchSadParam{"duplicate_default", "let a = switch (res) { default -> 1 default -> 2 }",
       ErrorCode::MultipleDefaultCasesInSwitch},
        SwitchSadParam{"assignment_in_body", "let a = switch (res) { let b = 2 }",
        ErrorCode::CaseOrDefaultMissingInSwitch}

    ),
    [](const testing::TestParamInfo<SwitchSadParam>& info) {
    return info.param.test_id;
    }
);
