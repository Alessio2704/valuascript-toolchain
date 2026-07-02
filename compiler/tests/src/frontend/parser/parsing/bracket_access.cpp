#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"


using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct BracketAccessSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class BracketAccessSadPathTest : public AstBaseTest,
                                     public testing::WithParamInterface<BracketAccessSadParam>
    {
    };

    TEST_P(BracketAccessSadPathTest, ThrowsCorrectSyntaxError)
    {
        const BracketAccessSadParam& param = GetParam();

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
        BracketAccessSadPathTest,
        testing::Values(
            BracketAccessSadParam{"unclosed_vector_access", "vec[0", E::
            UnmatchedBracketAfterTensorIndex}
        ),
        [](const testing::TestParamInfo<BracketAccessSadParam>& test_info) {
        return test_info.param.test_name;
        }
    );
}
