#include <gtest/gtest.h>
#include "../errors_synchronization/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct TensorLiteralSadParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class TensorLiteralSadPathTest : public AstBaseTest,
                                     public testing::WithParamInterface<TensorLiteralSadParam>
    {
    };

    TEST_P(TensorLiteralSadPathTest, ThrowsCorrectSyntaxError)
    {
        const TensorLiteralSadParam& param = GetParam();

        try
        {
            parse_expression_as_assignment(param.source_code);
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
        TensorLiteralSadPathTest,
        testing::Values(
            TensorLiteralSadParam{"vector_literal_unclosed", "[1,2,3", ValuascriptErrorCode::
            UnmatchedBracketAfterTensorElements},
            TensorLiteralSadParam{"matrix_literal_unclosed", "[[1,2], [3, 4]", ValuascriptErrorCode::
            UnmatchedBracketAfterTensorElements},
            TensorLiteralSadParam{"mising_operator_1", "[1,2,4] [1,2,3]", ValuascriptErrorCode::
            UnexpectedCommaInBracketAccess},
            TensorLiteralSadParam{"mising_operator_2", "[1, [2,4]] [[1,2,3], [1,2]]", ValuascriptErrorCode::
            UnexpectedCommaInBracketAccess}
        ),
        [](const testing::TestParamInfo<TensorLiteralSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
