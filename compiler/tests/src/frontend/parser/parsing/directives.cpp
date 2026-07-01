#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct DirectiveSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class DirectiveSadPathTest : public AstBaseTest,
                                 public testing::WithParamInterface<DirectiveSadParam>
    {
    };

    TEST_P(DirectiveSadPathTest, ThrowsCorrectSyntaxError)
    {
        const DirectiveSadParam& param = GetParam();

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
        DirectiveSadPathTest,
        testing::Values(
            DirectiveSadParam{"missing_operator_1", "#iterations = 1000 1", E::MissingOperator},
            DirectiveSadParam{"missing_operator_2", "#iterations = 1000 + 1 2", E::MissingOperator},
            DirectiveSadParam{"missing_operator_3", "#iterations = 1000 + (1 2)", E::MissingOperator},
            DirectiveSadParam{"missing_operator_4", "#iterations = 1000  (1 + 2)", E::MissingOperator},
            DirectiveSadParam{"missing_operator_5", "#iterations = 1000 a() + b()", E::MissingOperator}
        ),
        [](const testing::TestParamInfo<DirectiveSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
