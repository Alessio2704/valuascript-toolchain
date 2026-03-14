#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct StructHappyParam {
    std::string test_id;
    std::string source_code;
    std::string expected_name;
    size_t expected_field_count;
};

class StructHappyPathTest : public test::AstBaseTest,
                            public testing::WithParamInterface<StructHappyParam> {};

TEST_P(StructHappyPathTest, ParsesSuccessfully) {
    auto param = GetParam();
    
    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
    }) << "Parser threw an exception on valid struct test: " << param.test_id;

    ASSERT_NE(ast, nullptr);
    ASSERT_EQ(ast->struct_definitions.size(), 1) << "Expected exactly 1 struct definition";
    
    auto struct_def = ast->struct_definitions[0].get();
    EXPECT_EQ(struct_def->name, param.expected_name);
    EXPECT_EQ(struct_def->fields.size(), param.expected_field_count);
}

INSTANTIATE_TEST_SUITE_P(
    ValidStructs,
    StructHappyPathTest,
    testing::Values(
        StructHappyParam{"empty_struct", "struct Empty {}", "Empty", 0},
        StructHappyParam{"single_field", "struct Wrapper { val: scalar }", "Wrapper", 1},
        StructHappyParam{"multiple_fields", "struct Point { x: integer, y: integer }", "Point", 2},
        StructHappyParam{"complex_types", "struct Model { rates: Vector<scalar>, bounds: (integer, integer) }", "Model", 2},
        StructHappyParam{"trailing_comma", "struct Trailing { a: scalar, }", "Trailing", 1}
    ),
    [](const testing::TestParamInfo<StructHappyParam>& info) {
        return info.param.test_id;
    }
);

struct StructErrorParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class StructErrorPathTest : public test::AstBaseTest,
                            public testing::WithParamInterface<StructErrorParam> {};

TEST_P(StructErrorPathTest, FailsWithCorrectSyntaxError) {
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
    InvalidStructSyntax,
    StructErrorPathTest,
    testing::Values(
        StructErrorParam{"missing_name", "struct { x: integer }", ErrorCode::ExpectedStructName},
        StructErrorParam{"missing_left_brace", "struct Point x: integer }", ErrorCode::ExpectedBraceInStructDefinition},
        StructErrorParam{"missing_colon", "struct Point { x integer }", ErrorCode::ExpectedColonAfterStructFieldName},
        StructErrorParam{"missing_type", "struct Point { x: , y: integer }", ErrorCode::MissingTypeAnnotation},
        StructErrorParam{"missing_right_brace", "struct Point { x: integer", ErrorCode::ExpectedBraceInStructDefinition}
    ),
    [](const testing::TestParamInfo<StructErrorParam>& info) {
        return info.param.test_id;
    }
);