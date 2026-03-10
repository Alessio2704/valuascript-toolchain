#include <gtest/gtest.h>
#include "../ast_base_test.h"
#include "errors/valuascript_exception.h"

using namespace valuascript;
using namespace valuascript::compiler;

struct ReassignmentHappyParam {
    std::string test_id;
    std::string source_code;
};

class ReassignmentHappyPathTest : public test::AstBaseTest,
                                  public testing::WithParamInterface<ReassignmentHappyParam> {
};

TEST_P(ReassignmentHappyPathTest, ParsesSuccessfully) {
    const ReassignmentHappyParam &param = GetParam();

    std::shared_ptr<Program> ast;
    EXPECT_NO_THROW({
        ast = parse_code(param.source_code);
        }) << "Parser threw an exception on valid assignment test: " << param.test_id;

    ASSERT_NE(ast, nullptr);
    EXPECT_GT(ast->execution_steps.size(), 0) << "Expected at least one execution step.";
}

INSTANTIATE_TEST_SUITE_P(
    ParserStageTest,
    ReassignmentHappyPathTest,
    testing::Values(
        ReassignmentHappyParam{"simple_reassignment", "a = 10"},
        ReassignmentHappyParam{"bracket_reassignment", "tensor[0] = 5.5"},
        ReassignmentHappyParam{"dot_reassignment", "portfolio.risk = 0.05"},
        ReassignmentHappyParam{"deep_target_reassignment", "portfolio.assets[0].weight = 25%"},
        ReassignmentHappyParam{"complex_value_reassignment", "rate = (base + premium) * 1.5"},
        ReassignmentHappyParam{"simple_function_call", "set_seed(s: 42)"},
        ReassignmentHappyParam{"method_call", "sys.init()"},
        ReassignmentHappyParam{"chained_method_call", "builder.set_rate(p: 5%).build()"}
    ),
    [](const testing::TestParamInfo<ReassignmentHappyParam>& info) {
    return info.param.test_id;
    }
);

struct ReassignmentSadParam {
    std::string test_id;
    std::string source_code;
    ErrorCode expected_error;
};

class ReassignmentSadPathTest : public test::AstBaseTest,
                                public testing::WithParamInterface<ReassignmentSadParam> {
};

TEST_P(ReassignmentSadPathTest, ThrowsCorrectSyntaxError) {
    const ReassignmentSadParam &param = GetParam();

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
    ReassignmentSadPathTest,
    testing::Values(
        ReassignmentSadParam{"assign_to_number", "10 = 5", ErrorCode::UnexpectedToken},
        ReassignmentSadParam{"assign_to_string", "\"val\" = 5", ErrorCode::UnexpectedToken},
        ReassignmentSadParam{"assign_to_function", "get_rate() = 5", ErrorCode::InvalidLeftSideExpressionInReassignment}
        ,
        ReassignmentSadParam{"assign_to_binary_expr", "a + b = 10", ErrorCode::InvalidLeftSideExpressionInReassignment},
        ReassignmentSadParam{"missing_rhs", "a = ", ErrorCode::InvalidExpression},
        ReassignmentSadParam{"standalone_literal", "42", ErrorCode::UnexpectedToken},
        ReassignmentSadParam{"standalone_variable", "my_var", ErrorCode::InvalidStandaloneStatement},
        ReassignmentSadParam{"standalone_binary", "1 + 1", ErrorCode::UnexpectedToken},
        ReassignmentSadParam{"standalone_bracket_access", "tensor[0]", ErrorCode::InvalidStandaloneStatement},
        ReassignmentSadParam{"multiple_reassignment", "a, b = 1, 2", ErrorCode::MultiReassignmentNotSupported}
    ),
    [](const testing::TestParamInfo<ReassignmentSadParam>& info) {
    return info.param.test_id;
    }
);
