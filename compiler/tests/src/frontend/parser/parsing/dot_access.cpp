#include <gtest/gtest.h>
#include "ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct DotAccessSadParam
    {
        std::string test_name;
        std::string source_code;
        E expected_error;
    };

    class DotAccessSadPathTest : public AstBaseTest,
                                 public testing::WithParamInterface<DotAccessSadParam>
    {
    };

    TEST_P(DotAccessSadPathTest, ThrowsCorrectSyntaxError)
    {
        const DotAccessSadParam& param = GetParam();

        try
        {
            parse_code(param.source_code);
            FAIL() << "Parser should have thrown an exception for test: " << param.test_name;
        }
        catch (const ValuaScriptException& e)
        {
            EXPECT_TRUE(e.is_error(param.expected_error))
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        DotAccessSadPathTest,
        testing::Values(
            DotAccessSadParam{.test_name = "number_as_property", .source_code = "let a = model.123", .expected_error = E::MissingOperator}
        ),
        TestNameGenerator{}
    );
}
