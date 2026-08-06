#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct DictLiteralSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class DictLiteralSadPathTest : public AstBaseTest,
                                   public testing::WithParamInterface<DictLiteralSadParam>
    {
    };

    TEST_P(DictLiteralSadPathTest, ThrowsCorrectSyntaxError)
    {
        const DictLiteralSadParam& param = GetParam();

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
        DictLiteralSadPathTest,
        testing::Values(
            DictLiteralSadParam{.test_name = "dict_missing_brace", .source_code = "{a: 1", .expected_error = E::
            UnmatchedBraceInDictionaryLiteral}
        ),
        TestNameGenerator{}
    );
}
