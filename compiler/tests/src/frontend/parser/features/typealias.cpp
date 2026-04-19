#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class TypealiasSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(TypealiasSuccessPathTest, Simple)
    {
        ExpectValidParse(
            "typealias Identifier = string",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("Identifier", {},
                                IsType("string")
                    )
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, MultipleTypeAliases)
    {
        ExpectValidParse(
            "typealias ID = string\n"
            "typealias Callback = string\n"
            "typealias Registry = string",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("ID", {},
                                IsType("string")
                    ),
                    IsTypeAlias("Callback", {},
                                IsType("string")
                    ),
                    IsTypeAlias("Registry", {},
                                IsType("string")
                    ),
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, MultilineFormatting)
    {
        ExpectValidParse(
            "typealias\n"
            "Data\n "
            "= \n"
            "string\n",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("Data", {}, IsType("string")),
                }
            }
        );
    }
}
