#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct ReassignmentSadParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class ReassignmentSadPathTest : public AstBaseTest,
                                    public testing::WithParamInterface<ReassignmentSadParam>
    {
    };

    TEST_P(ReassignmentSadPathTest, ThrowsCorrectSyntaxError)
    {
        const ReassignmentSadParam& param = GetParam();

        try
        {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        }
        catch (const ValuaScriptException& e)
        {
            EXPECT_EQ(e.get_category(), ValuascriptErrorCategory::Syntax)
                << "Category mismatch on test: " << param.test_name;
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        ReassignmentSadPathTest,
        testing::Values(
            ReassignmentSadParam{"assign_to_number", "10 = 5", ValuascriptErrorCode::
            InvalidLeftSideExpressionInReassignment},
            ReassignmentSadParam{"assign_to_string", "\"val\" = 5", ValuascriptErrorCode::
            InvalidLeftSideExpressionInReassignment},
            ReassignmentSadParam{"assign_to_function", "get_rate() = 5", ValuascriptErrorCode::
            InvalidLeftSideExpressionInReassignment}
            ,
            ReassignmentSadParam{"assign_to_binary_expr", "a + b = 10", ValuascriptErrorCode::
            InvalidLeftSideExpressionInReassignment},
            ReassignmentSadParam{"missing_rhs", "a = ", ValuascriptErrorCode::MissingValueAfterEquals},
            ReassignmentSadParam{"standalone_literal", "42", ValuascriptErrorCode::InvalidStandaloneStatement},
            ReassignmentSadParam{"standalone_variable", "my_var", ValuascriptErrorCode::InvalidStandaloneStatement},
            ReassignmentSadParam{"standalone_binary", "1 + 1", ValuascriptErrorCode::InvalidStandaloneStatement},
            ReassignmentSadParam{"standalone_bracket_access", "tensor[0]", ValuascriptErrorCode::
            InvalidStandaloneStatement},
            ReassignmentSadParam{"multiple_reassignment", "a, b = 1, 2", ValuascriptErrorCode::
            MultiReassignmentNotSupported},
            ReassignmentSadParam{"missing_operator_1", "a = 1 2", ValuascriptErrorCode::MissingOperator},
            ReassignmentSadParam{"missing_operator_2", "a = 1 + 2 3", ValuascriptErrorCode::MissingOperator},
            ReassignmentSadParam{"missing_operator_3", "a = 1 + (2 3)", ValuascriptErrorCode::
            MissingOperatorInsideGrouping},
            ReassignmentSadParam{"missing_operator_4", "a = 1  (2 + 3)", ValuascriptErrorCode::
            MissingOperatorOrArgumentName},
            ReassignmentSadParam{"missing_operator_5", "a = 1 + a() b()", ValuascriptErrorCode::MissingOperator},
            ReassignmentSadParam{"missing_operator_6", "a = 1 + a[0] b[1:2]", ValuascriptErrorCode::MissingOperator},
            ReassignmentSadParam{"missing_operator_7", "a = 1 + a[0] + b[1:2] a.b", ValuascriptErrorCode::
            MissingOperator}
        ),
        [](const testing::TestParamInfo<ReassignmentSadParam>& info) {
        return info.param.test_name;
        }
    );
}
