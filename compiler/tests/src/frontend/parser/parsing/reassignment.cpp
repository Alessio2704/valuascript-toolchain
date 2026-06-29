#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct ReassignmentSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
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
            EXPECT_TRUE(e.is_error(param.expected_error))
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        ReassignmentSadPathTest,
        testing::Values(
            ReassignmentSadParam{"assign_to_number", "10 = 5", E::
            InvalidLeftSideExpressionInReassignment},
            ReassignmentSadParam{"assign_to_string", "\"val\" = 5", E::
            InvalidLeftSideExpressionInReassignment},
            ReassignmentSadParam{"assign_to_function", "get_rate() = 5", E::
            InvalidLeftSideExpressionInReassignment}
            ,
            ReassignmentSadParam{"assign_to_binary_expr", "a + b = 10", E::
            InvalidLeftSideExpressionInReassignment},
            ReassignmentSadParam{"missing_rhs", "a = ", E::MissingValueAfterEquals},
            ReassignmentSadParam{"standalone_literal", "42", E::InvalidStandaloneStatement},
            ReassignmentSadParam{"standalone_variable", "my_var", E::InvalidStandaloneStatement},
            ReassignmentSadParam{"standalone_binary", "1 + 1", E::InvalidStandaloneStatement},
            ReassignmentSadParam{"standalone_bracket_access", "tensor[0]", E::
            InvalidStandaloneStatement},
            ReassignmentSadParam{"multiple_reassignment", "a, b = 1, 2", E::
            MultiReassignmentNotSupported},
            ReassignmentSadParam{"missing_operator_1", "a = 1 2", E::MissingOperator},
            ReassignmentSadParam{"missing_operator_2", "a = 1 + 2 3", E::MissingOperator},
            ReassignmentSadParam{"missing_operator_3", "a = 1 + (2 3)", E::
            MissingOperator},
            ReassignmentSadParam{"missing_operator_4", "a = 1  (2 + 3)", E::
            MissingOperatorOrArgumentName},
            ReassignmentSadParam{"missing_operator_5", "a = 1 + a() b()", E::MissingOperator},
            ReassignmentSadParam{"missing_operator_6", "a = 1 + a[0] b[1:2]", E::MissingOperator},
            ReassignmentSadParam{"missing_operator_7", "a = 1 + a[0] + b[1:2] a.b", E::
            MissingOperator}
        ),
        [](const testing::TestParamInfo<ReassignmentSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
