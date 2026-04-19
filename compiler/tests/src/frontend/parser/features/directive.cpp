#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class DirectiveSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(DirectiveSuccessPathTest, NoEqual1)
    {
        ExpectValidDirective(
            "#no_equal 1",
            IsDirective("no_equal", IsNumber("1"))
        );
    }

    TEST_F(DirectiveSuccessPathTest, NoValueDirective)
    {
        ExpectValidDirective(
            "#no_value",
            IsDirective("no_value", IsNull())
        );
    }

    TEST_F(DirectiveSuccessPathTest, NoValueDirectiveUnderscoreAndNumber)
    {
        ExpectValidDirective(
            "#no_value_1",
            IsDirective("no_value_1", IsNull())
        );
    }

    TEST_F(DirectiveSuccessPathTest, ValueDirective1)
    {
        ExpectValidDirective(
            "#value = 1",
            IsDirective("value", IsNumber("1"))
        );
    }
}
