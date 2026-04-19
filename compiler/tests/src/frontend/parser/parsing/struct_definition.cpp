#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct StructErrorParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
    };

    class StructErrorPathTest : public AstBaseTest,
                                public testing::WithParamInterface<StructErrorParam>
    {
    };

    TEST_P(StructErrorPathTest, FailsWithCorrectSyntaxError)
    {
        auto param = GetParam();

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
        InvalidStructSyntax,
        StructErrorPathTest,
        testing::Values(
            StructErrorParam{"missing_name", "struct { x: integer }", ValuascriptErrorCode::ExpectedStructName},
            StructErrorParam{"missing_left_brace", "struct Point x: integer }", ValuascriptErrorCode::
            ExpectedBraceInStructDefinition},
            StructErrorParam{"missing_colon", "struct Point { x integer }", ValuascriptErrorCode::
            ExpectedColonAfterStructFieldName},
            StructErrorParam{"missing_type", "struct Point { x: , y: integer }", ValuascriptErrorCode::
            MissingTypeAnnotation},
            StructErrorParam{"missing_right_brace", "struct Point { x: integer", ValuascriptErrorCode::
            ExpectedRightBraceAfterStructBody}
        ),
        [](const testing::TestParamInfo<StructErrorParam>& info) {
        return info.param.test_name;
        }
    );
}
