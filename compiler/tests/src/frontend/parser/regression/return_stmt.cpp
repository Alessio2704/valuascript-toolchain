#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ReturnStatementRegressionTest : public ParserTestBase
    {
    };

    TEST_F(ReturnStatementRegressionTest, SequentialReturnStatementsInFunctionBody)
    {
        ExpectParseErrorsWithRecovery(
            "func ctx_wrapper() -> void {\n"
            "  return\n"
            "  return 100\n"
            "}\n",
            {},
            ProgramSpec{
                .functions = {
                    IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {
                        IsReturn({}, {}),
                        IsReturn({}, {IsNumber("100")})
                    })
                }
            }
        );
    }
}
