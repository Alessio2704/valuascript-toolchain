#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class DirectiveSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(DirectiveSuccessPathTest, NoEqual1)
    {
        ExpectValidParse(
            "#no_equal 1",
            ProgramSpec{
                .directives = {
                    IsDirective("no_equal", IsNumber("1"))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, NoValueDirective)
    {
        ExpectValidParse(
            "#no_value",
            ProgramSpec{
                .directives = {
                    IsDirective("no_value", IsNull())
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, NoValueDirectiveUnderscoreAndNumber)
    {
        ExpectValidParse(
            "#no_value_1",
            ProgramSpec{
                .directives = {
                    IsDirective("no_value_1", IsNull())
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, ValueDirective1)
    {
        ExpectValidParse(
            "#value = 1",
            ProgramSpec{
                .directives = {
                    IsDirective("value", IsNumber("1"))
                }
            }
        );
    }
}
