#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct TupleSadParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
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
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        TupleSadPathTest,
        testing::Values(
            TupleSadParam{"tuple_missing_second_value_1", "(1, ", ValuascriptErrorCode::
            ExpectedRightParenAfterTupleElements},
            TupleSadParam{"tuple_missing_second_value_2", "(a, ", ValuascriptErrorCode::
            ExpectedRightParenAfterTupleElements},
            TupleSadParam{"tuple_parenthesis", "(a, b", ValuascriptErrorCode::ExpectedRightParenAfterTupleElements},
            TupleSadParam{"tuple_trailing_comma_1", "(a, b,)", ValuascriptErrorCode::TrailingCommaInTuple},
            TupleSadParam{"single_element_tuples_not_allowed", "(1, )", ValuascriptErrorCode::
            SingleElementTuplesNotAllowed},
            TupleSadParam{"missing_operator_1", "(a b)", ValuascriptErrorCode::MissingOperatorInsideGrouping},
            TupleSadParam{"missing_operator_2", "(a, b c)", ValuascriptErrorCode::
            MissingCommaOrOperatorBetweenExpressions},
            TupleSadParam{"missing_operator_3", "(a, b (c + d))", ValuascriptErrorCode::MissingOperatorOrArgumentName},
            TupleSadParam{"missing_operator_4", "(a, b + (c  d))", ValuascriptErrorCode::MissingOperatorInsideGrouping}
        ),
        [](const testing::TestParamInfo<TupleSadParam>& info) {
        return info.param.test_name;
        }
    );
}
