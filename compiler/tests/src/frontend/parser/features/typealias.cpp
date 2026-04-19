#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class TypealiasSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(TypealiasSuccessPathTest, Simple)
    {
        ExpectValidTypeAlias(
            "typealias Identifier = string",
            IsTypeAlias("Identifier", {},
                        IsType("string")
            )
        );
    }

    TEST_F(TypealiasSuccessPathTest, MultilineFormatting)
    {
        ExpectValidTypeAlias(
            "typealias\n"
            "Data\n "
            "= \n"
            "string\n",
            IsTypeAlias("Data", {}, IsType("string"))
        );
    }
}
