#include <gtest/gtest.h>
#include "frontend/parser/helpers/ast_base_test.h"
#include "core/valuascript_exception.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    struct DictLiteralSadParam
    {
        std::string test_name;
        std::string source_code;
        ValuascriptErrorCode expected_error;
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
            EXPECT_EQ(e.get_code(), param.expected_error)
                << "Error code mismatch on test: " << param.test_name;
        }
    }

    INSTANTIATE_TEST_SUITE_P(
        ParserStageTest,
        DictLiteralSadPathTest,
        testing::Values(
            DictLiteralSadParam{"dict_missing_brace", "{a: 1", ValuascriptErrorCode::
            UnmatchedBraceInDictionaryLiteral},
            DictLiteralSadParam{"dict_missing_comma", "{a: 1 b: 2}", ValuascriptErrorCode::
            ExpectedCommaSeparatorInDictionaryLiteral},
            DictLiteralSadParam{"dict_missing_key", "{1}", ValuascriptErrorCode::
            ExpectedDictionaryKey},
            DictLiteralSadParam{"dict_missing_colon", "{a 1}", ValuascriptErrorCode::
            ExpectedColonAfterDictionaryKey},
            DictLiteralSadParam{"dict_empty", "{a}", ValuascriptErrorCode::
            ExpectedColonAfterDictionaryKey},
            DictLiteralSadParam{"dict_key_string_literal", "{ \"key\" 10 }", ValuascriptErrorCode::ExpectedDictionaryKey
            },
            DictLiteralSadParam{"dict_key_missing_operator", "{ market_size: 13_624 / 11%   4, }", ValuascriptErrorCode
            ::MissingOperator},
            DictLiteralSadParam{"dict_self_missing_property_name", "{ a: 1, b: self. }", ValuascriptErrorCode::
            ExpectedPropertyName},
            DictLiteralSadParam{"dict_self_empty_bracket", "{ a: 1, b: self[] }", ValuascriptErrorCode::
            EmptyBracketAccess},
            DictLiteralSadParam{"dict_self_missing_operator", "{ a: 1, b: self.a 5 }", ValuascriptErrorCode::
            MissingOperator}
        ),
        [](const testing::TestParamInfo<DictLiteralSadParam>& info) {
        return info.param.test_name;
        }
    );
}
