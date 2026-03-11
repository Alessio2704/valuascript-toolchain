#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct FunctionCallHappyParam {
    std::string test_id;
    std::string source_code;
    size_t param_num;
};

class FunctionCallHappyPathTest : public test::AstBaseTest,
                                  public testing::WithParamInterface<FunctionCallHappyParam> {
};

TEST_P(FunctionCallHappyPathTest, ParsesSuccessfully) {
    const FunctionCallHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code("let result = " + param.source_code);
        }) << "Parser threw an exception on valid function test: " << param.test_id;

    if (ast) {
        ASSERT_EQ(ast->function_definitions.size(), 0);
        EXPECT_EQ(ast->execution_steps.size(), 1);
        EXPECT_EQ(ast->directives.size(), 0);

        auto assignment = dynamic_cast<Assignment *>(ast->execution_steps[0].get());
        ASSERT_EQ(assignment->targets.size(), 1);
        ASSERT_EQ(assignment->targets[0].first, "result");
        auto func_call = dynamic_cast<FunctionCall *>(assignment->value.get());
        auto func_target = dynamic_cast<IdentifierAccess *>(func_call->target.get());
        ASSERT_EQ(func_target->name, "test");
        ASSERT_EQ(func_call->arguments.size(), param.param_num);
    }
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    FunctionCallHappyPathTest,
    testing::Values(
        FunctionCallHappyParam{"basic_1_param", "test(a: 1)", 1},
        FunctionCallHappyParam{"basic_2_param", "test(a: 1, b: 2)", 2},
        FunctionCallHappyParam{"string_arg", "test(a: 1, b: 1.5, c: true, d: \"call\")", 4},
        FunctionCallHappyParam{"dict_param", "test(a: { b:1, c:2 })", 1},
        FunctionCallHappyParam{"tuple_literal_param", "test(a: (1, 2))", 1},
        FunctionCallHappyParam{"tuple_literal_identifiers_param", "test(a: (b, c))", 1},
        FunctionCallHappyParam{"tuple_literal_complex", "test(a: (if b then 1 else (2 + d), c))", 1},
        FunctionCallHappyParam{"dict_literal_complex", "test(a: { b: c, d: if z >= 1 then w else y })", 1},
        FunctionCallHappyParam{"dict_literal_plus_tuple_inside", "test(a: { b: (1, 3, d) })", 1},
        FunctionCallHappyParam{"call_no_args", "test()", 0},
        FunctionCallHappyParam{"call_multiple_args", "test(a: 1, b: 2, c: 3)", 3},
        FunctionCallHappyParam{"call_nested", "test(a: inner_func(a: x), b: y)", 2},
        FunctionCallHappyParam{"call_with_math_args", "test(one: a + b, second: c * d)", 2}
    ),
    [](const testing::TestParamInfo<FunctionCallHappyParam>& info) {
    return info.param.test_id;
    }
);

struct FunctionCallSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class FunctionCallSadPathTest : public test::AstBaseTest,
                                public testing::WithParamInterface<FunctionCallSadParam> {
};

TEST_P(FunctionCallSadPathTest, ThrowsCorrectSyntaxError) {
    const FunctionCallSadParam &param = GetParam();

    try {
        parse_code("let result = " + param.source_code);
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
    FunctionCallSadPathTest,
    testing::Values(
        FunctionCallSadParam{"func_missing_argument", "test(1)", ErrorCode::MissingArgumentName},
        FunctionCallSadParam{"func_missing_colon", "test(a 1)", ErrorCode::MissingColonAfterArgument},
        FunctionCallSadParam{"func_missing_argument_value", "test(a: )", ErrorCode::InvalidExpression},
        FunctionCallSadParam{"func_missing_argument_after_comma", "test(a: 1, )", ErrorCode::
        MissingArgumentName},
        FunctionCallSadParam{"unclosed_call", "test(a: 1, b: 2", ErrorCode::ExpectedRightParen}
    ),
    [](const testing::TestParamInfo<FunctionCallSadParam>& info) {
    return info.param.test_id;
    }
);
