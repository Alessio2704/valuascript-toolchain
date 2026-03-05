#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_stage.h"
#include "stages/frontend/lexer/lexer_stage.h"
#include "stages/frontend/parser/ast.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

class ParserEnumDefinitionTest : public testing::Test {
protected:
    std::shared_ptr<Program> parse_code(const std::string &code) {
        LexerStage lexer;
        auto lexer_result = lexer.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            {CompilerStageArtifactCode::SourceCode, code}
        });

        ParserStage parser;
        auto parser_result = parser.run({
            {CompilerStageArtifactCode::FilePath, std::string("test.vs")},
            lexer_result
        });

        return std::any_cast<std::shared_ptr<Program> >(parser_result.data);
    }
};

struct EnumHappyParam {
    std::string test_id;
    std::string source_code;
};

class EnumHappyTest : public ParserEnumDefinitionTest,
                      public testing::WithParamInterface<EnumHappyParam> {
};

TEST_P(EnumHappyTest, ParsesSuccessfully) {
    auto param = GetParam();
    std::shared_ptr<Program> ast;

    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser choked on valid enum syntax: " << param.test_id << "\nCode: " << param.source_code;

    ASSERT_NE(ast, nullptr);
    EXPECT_EQ(ast->enum_definitions.size(), 1) << "Expected exactly 1 enum definition to be parsed.";
}

INSTANTIATE_TEST_SUITE_P(
    ValidEnumDefinitions,
    EnumHappyTest,
    testing::Values(
        EnumHappyParam{"implicit_values", "enum OptionType: string { call, put }"},
        EnumHappyParam{"explicit_strings", "enum Direction: string { UP = \"up\", DOWN = \"down\" }"},
        EnumHappyParam{"explicit_numbers", "enum Status: integer { ACTIVE = 1, PENDING = 2 }"},
        EnumHappyParam{"mixed_values", "enum Codes: integer { START = 100, CONTINUE, END = 999 }"},
        EnumHappyParam{"expression_values", "enum Math: decimal { PI = 3.14, TAU = 3.14 * 2.0 }"},
        EnumHappyParam{"empty_enum", "enum Phantom: string {}"},
        EnumHappyParam{"custom_type", "enum Complex: Result<decimal, string> { OK, ERR }"}
    ),
    [](const testing::TestParamInfo<EnumHappyParam>& info) {
    return info.param.test_id;
    }
);

struct EnumSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class EnumSadTest : public ParserEnumDefinitionTest,
                    public testing::WithParamInterface<EnumSadParam> {
};

TEST_P(EnumSadTest, ThrowsCorrectSyntaxError) {
    auto param = GetParam();

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
    InvalidEnumDefinitions,
    EnumSadTest,
    testing::Values(
        EnumSadParam{"missing_name", "enum : string { a }", ErrorCode::ExpectedEnumName},
        EnumSadParam{"missing_colon", "enum Option string { a }", ErrorCode::ExpectedColonAfterEnumName},
        EnumSadParam{"missing_type", "enum Option: { a }", ErrorCode::MissingTypeAnnotation},
        EnumSadParam{"missing_left_brace", "enum Option: string a }", ErrorCode::ExpectedLeftBrace},
        EnumSadParam{"invalid_case_name_number", "enum Option: string { 1 = \"a\" }", ErrorCode::ExpectedEnumCaseName},
        EnumSadParam{"invalid_case_name_string", "enum Option: string { \"call\" }", ErrorCode::ExpectedEnumCaseName},
        EnumSadParam{"missing_right_brace", "enum Option: string { a, b ", ErrorCode::ExpectedRightBrace},
        EnumSadParam{"invalid_value_expression", "enum Option: string { a = let }", ErrorCode::InvalidExpression},
        EnumSadParam{"trailing_comma", "enum Colors: string { red, blue, }", ErrorCode::ExpectedEnumCaseName},
        EnumSadParam{"keyword_as_case_name", "enum Bad: string { if = \"a\" }", ErrorCode::ExpectedEnumCaseName}
    ),
    [](const testing::TestParamInfo<EnumSadParam>& info) {
    return info.param.test_id;
    }
);
