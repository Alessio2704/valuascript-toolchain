#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ReturnStatementSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(ReturnStatementSuccessPathTest, ReturnSingleValue)
    {
        ExpectValidReturn("return 1", IsReturn({IsNumber("1")}));
    }

    TEST_F(ReturnStatementSuccessPathTest, ReturnMultipleValues)
    {
        ExpectValidReturn("return 1, true, \"success\"", IsReturn({
                              IsNumber("1"),
                              IsBoolean(true),
                              IsString("\"success\"")
                          }));
    }

    TEST_F(ReturnStatementSuccessPathTest, ReturnWithIdentifier)
    {
        ExpectValidReturn("return result", IsReturn({IsIdentifier("result")}));
    }

    TEST_F(ReturnStatementSuccessPathTest, MultilineFormatting)
    {
        ExpectValidReturn(
            "return \n"
            "  1, \n"
            "  2",
            IsReturn({IsNumber("1"), IsNumber("2")})
        );
    }
}
