#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct TupleSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class TupleSadPathTest : public AstBaseTest,
                             public testing::WithParamInterface<TupleSadParam>
    {
    };

    TEST_P(TupleSadPathTest, ThrowsCorrectSyntaxError)
    {
        const TupleSadParam& param = GetParam();

        try
        {
            parse_expression_as_assignment(param.source_code);
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
        TupleSadPathTest,
        testing::Values(
            TupleSadParam{.test_name = "tuple_missing_second_value_1", .source_code = "(1, ", .expected_error = E::
            ExpectedRightParenAfterTupleElements},
            TupleSadParam{.test_name = "tuple_missing_second_value_2", .source_code = "(a, ", .expected_error = E::
            ExpectedRightParenAfterTupleElements},
            TupleSadParam{.test_name = "tuple_parenthesis", .source_code = "(a, b", .expected_error = E::ExpectedRightParenAfterTupleElements}
        ),
        TestNameGenerator{}
    );
}
