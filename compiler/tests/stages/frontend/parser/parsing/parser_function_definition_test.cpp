#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct FunctionHappyParam {
    std::string test_id;
    std::string source_code;
    size_t expected_param_count;
    size_t expected_return_type_count;
    size_t expected_body_statements;
    bool expects_docstring;
};

class FunctionHappyPathTest : public test::AstBaseTest,
                              public testing::WithParamInterface<FunctionHappyParam> {
};

TEST_P(FunctionHappyPathTest, ParsesSuccessfully) {
    const FunctionHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser threw an exception on valid function test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->function_definitions.size(), 1) << "Expected exactly 1 function in AST.";
        EXPECT_EQ(ast->execution_steps.size(), 0);
        EXPECT_EQ(ast->directives.size(), 0);

        auto &func = ast->function_definitions[0];
        EXPECT_EQ(func->parameters.size(), param.expected_param_count) << "Parameter count mismatch.";
        EXPECT_EQ(func->return_types.size(), param.expected_return_type_count) << "Return type count mismatch.";
        EXPECT_EQ(func->body.size(), param.expected_body_statements) << "Body statement count mismatch.";
        EXPECT_EQ(func->docstring.has_value(), param.expects_docstring) << "Docstring presence mismatch.";
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    FunctionHappyPathTest,
    testing::Values(
        FunctionHappyParam{"minimal_func", "func test() -> scalar {}", 0, 1, 0, false},
        FunctionHappyParam{"single_param", "func test(a: scalar) -> scalar {}", 1, 1, 0, false},
        FunctionHappyParam{"multi_param", "func test(a: scalar, b: boolean) -> scalar {}", 2, 1, 0, false},
        FunctionHappyParam{"generic_param", "func process(data: vector<matrix>) -> scalar {}", 1, 1, 0, false},
        FunctionHappyParam{"nested_generic_param", "func process(data: vector<vector<scalar>>) -> scalar {}", 1, 1, 0,
        false},
        FunctionHappyParam{"tuple_return", "func bounds() -> (scalar, scalar) {}", 0, 1, 0, false},
        FunctionHappyParam{"tuple_return_generic", "func bounds() -> (scalar, vector<scalar>) {}", 0, 1, 0, false},
        FunctionHappyParam{"tuple_param", "func bounds(a: (scalar, scalar, Custom)) -> (scalar, Custom) {}", 1, 1, 0, false},
        FunctionHappyParam{"tuple_param_generic", "func bounds(a: (scalar, vector<scalar>, Custom)) -> (scalar, Custom) {}", 1, 1, 0, false},
        FunctionHappyParam{"param_generic", "func test(a: Input<A, B>) -> Result<T, E> {}", 1, 1, 0, false},
        FunctionHappyParam{"tuple_and_struct_param", "func bounds(a: (scalar, scalar, Custom), b: Other) -> (scalar, Custom) {}", 2, 1, 0, false},
        FunctionHappyParam{"tuple_and_struct_return", "func bounds(a: (scalar, scalar, Custom), b: Other) -> (scalar, Custom), Other {}", 2, 2, 0, false},
        FunctionHappyParam{"dict_literal_return", "func dict() -> dict { return { cagr: 1, yrs: 10 } }", 0, 1, 1, false},
        FunctionHappyParam{"docstring", "func test() -> scalar { \"\"\"Calculates something.\"\"\" }", 0, 1, 0, true},
        FunctionHappyParam{"body_statements", "func test() -> scalar { let a = 1 \n return a }", 0, 1, 2, false},
        FunctionHappyParam{"kitchen_sink",
        "func full(v: vector<scalar>, b: boolean) -> (vector<scalar>, boolean) { \"\"\"Docs\"\"\" let out = v \n return out }"
        , 2, 1, 2, true}
    ),
    [](const testing::TestParamInfo<FunctionHappyParam>& info) {
    return info.param.test_id;
    }
);

struct FunctionSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class FunctionSadPathTest : public test::AstBaseTest,
                            public testing::WithParamInterface<FunctionSadParam> {
};

TEST_P(FunctionSadPathTest, ThrowsCorrectSyntaxError) {
    const FunctionSadParam &param = GetParam();

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
    FunctionSadPathTest,
    testing::Values(
        FunctionSadParam{"missing_func_name", "func () -> scalar {}", ErrorCode::MissingFunctionName},
        FunctionSadParam{"missing_left_paren", "func test) -> scalar {}", ErrorCode::ExpectedLeftParenAfterFunctionName},
        FunctionSadParam{"missing_right_paren", "func test(a: scalar -> scalar {}", ErrorCode::ExpectedRightParenAfterParameters},
        FunctionSadParam{"missing_arrow", "func test() scalar {}", ErrorCode::MissingArrowInFunction},
        FunctionSadParam{"malformed_arrow_1", "func test() scalar - {}", ErrorCode::MissingArrowInFunction},
        FunctionSadParam{"malformed_arrow_2", "func test() scalar > {}", ErrorCode::MissingArrowInFunction},
        FunctionSadParam{"missing_left_brace", "func test() -> scalar }", ErrorCode::ExpectedLeftBraceBeforeFunctionBody},
        FunctionSadParam{"missing_right_brace", "func test() -> scalar { return 1", ErrorCode::ExpectedRightBraceAfterFunctionBody},
        FunctionSadParam{"missing_param_name", "func test(: scalar) -> scalar {}", ErrorCode::MissingParameterName},
        FunctionSadParam{"missing_colon", "func test(a scalar) -> scalar {}", ErrorCode::MissingColonAfterParameter},
        FunctionSadParam{"missing_param_type", "func test(a: ) -> scalar {}", ErrorCode::MissingTypeAnnotation},
        FunctionSadParam{"unclosed_generic", "func test(a: vector<scalar) -> scalar {}", ErrorCode::UnmatchedBracketAfterGenericArgs},
        FunctionSadParam{"missing_return_type", "func test() -> {}", ErrorCode::MissingTypeAnnotationAfterArrow},
        FunctionSadParam{"unclosed_tuple_return", "func test() -> (scalar, bool {}", ErrorCode::UnmatchedParenthesisInTuple},
        FunctionSadParam{"invalid_statement_in_body", "func test() -> scalar { 1 + 1 }", ErrorCode::InvalidStandaloneStatement},
        FunctionSadParam{"missing_comma_in_params", "func test(a: scalar b: decimal) -> scalar { return 1 + 1 }", ErrorCode::ExpectedCommaSeparatorInParameterList},
        FunctionSadParam{"missing_comma_return", "func test(a: scalar, b: decimal) -> scalar decimal { return 1 + 1 }", ErrorCode::ExpectedCommaSeparatorInReturnTypeList},
        FunctionSadParam{"top_level_declaration_in_func_1", "func test(a: s) -> s { return 1 \n let a = b()\n enum Test: s {}\n", ErrorCode::TopLevelDeclarationInsideFunction},
        FunctionSadParam{"top_level_declaration_in_func_2", "func test(a: s) -> s { return 1 \n let a = b()\n struct Test: s {}\n", ErrorCode::TopLevelDeclarationInsideFunction},
        FunctionSadParam{"top_level_declaration_in_func_3", "func test(a: s) -> s { return 1 \n let a = b()\n #dir\n", ErrorCode::TopLevelDeclarationInsideFunction},
        FunctionSadParam{"top_level_declaration_in_func_4", "func test(a: s) -> s { return 1 \n let a = b()\n func other() -> scalar {}\n", ErrorCode::TopLevelDeclarationInsideFunction}
    ),
    [](const testing::TestParamInfo<FunctionSadParam>& info) {
    return info.param.test_id;
    }
);
