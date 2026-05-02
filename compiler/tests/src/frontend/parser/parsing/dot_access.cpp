#include <gtest/gtest.h>
#include "../errors_synchronization/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct DotAccessSadParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
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
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        DotAccessSadPathTest,
        testing::Values(
            DotAccessSadParam{"missing_property", "let a = model.", ValuascriptErrorCode::ExpectedPropertyName},
            DotAccessSadParam{"number_as_property", "let a = model.123", ValuascriptErrorCode::MissingOperator},
            DotAccessSadParam{"keyword_as_property", "let a = model.let", ValuascriptErrorCode::
            ReservedKeywordAsIdentifier},
            DotAccessSadParam{"missing_property_deep", "let a = model.assets[0].", ValuascriptErrorCode::
            ExpectedPropertyName}
        ),
        [](const testing::TestParamInfo<DotAccessSadParam>& info) {
        return info.param.test_name;
        }
    );
}
