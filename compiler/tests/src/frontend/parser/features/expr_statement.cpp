#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ExpressionStatementSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(ExpressionStatementSuccessPathTest, SimpleCallStatement)
    {
        ExpectValidExpressionStatement("init()",
                                       IsExprStmt(IsCall(IsIdentifier("init"), {}))
        );
    }

    TEST_F(ExpressionStatementSuccessPathTest, MultilineFormatting)
    {
        ExpectValidExpressionStatement(
            "my_function \n"
            "  ( \n"
            "    arg: 1 \n"
            "  )",
            IsExprStmt(IsCall(IsIdentifier("my_function"), {{"arg", IsNumber("1")}}))
        );
    }
}
