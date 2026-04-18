#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class DirectiveSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(DirectiveSuccessPathTest, NoEqual1)
    {
        ExpectValidParse(
            "#no_equal one",
            ProgramSpec{
                .directives = {
                    IsDirective("no_equal", IsIdentifier("one"))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, NoEqual2)
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

    TEST_F(DirectiveSuccessPathTest, NoEqual3)
    {
        ExpectValidParse(
            "#no_equal {a: 1, b: 2}",
            ProgramSpec{
                .directives = {
                    IsDirective("no_equal",
                                IsDict({
                                    {"a", {}, IsNumber("1")},
                                    {"b", {}, IsNumber("2")}
                                }))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, NoEqual4)
    {
        ExpectValidParse(
            "#no_equal (1, 2, 3)",
            ProgramSpec{
                .directives = {
                    IsDirective("no_equal",
                                IsTuple({
                                    IsNumber("1"),
                                    IsNumber("2"),
                                    IsNumber("3")
                                }))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, NoEqual5)
    {
        ExpectValidParse(
            "#no_equal if a then 1 else 2",
            ProgramSpec{
                .directives = {
                    IsDirective("no_equal",
                                IsConditional(
                                    IsIdentifier("a"),
                                    IsNumber("1"),
                                    IsNumber("2")
                                ))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, NoEqual6)
    {
        ExpectValidParse(
            "#no_equal \"string\"",
            ProgramSpec{
                .directives = {
                    IsDirective("no_equal", IsString("\"string\""))
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

    TEST_F(DirectiveSuccessPathTest, ValueDirective2)
    {
        ExpectValidParse(
            "#value = 10.5",
            ProgramSpec{
                .directives = {
                    IsDirective("value", IsNumber("10.5"))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, ValueDirective3)
    {
        ExpectValidParse(
            "#value = 1_000.5",
            ProgramSpec{
                .directives = {
                    IsDirective("value", IsNumber("1_000.5"))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, ValueDirective4)
    {
        ExpectValidParse(
            "#value = \"string\"",
            ProgramSpec{
                .directives = {
                    IsDirective("value", IsString("\"string\""))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, ValueDirective5)
    {
        ExpectValidParse(
            "#value = true",
            ProgramSpec{
                .directives = {
                    IsDirective("value", IsBoolean(true))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, ValueDirective6)
    {
        ExpectValidParse(
            "#value = var_name",
            ProgramSpec{
                .directives = {
                    IsDirective("value", IsIdentifier("var_name"))
                }
            }
        );
    }

    TEST_F(DirectiveSuccessPathTest, ValueDirectiveComplex)
    {
        ExpectValidParse(
            "#value = 60 * 5",
            ProgramSpec{
                .directives = {
                    IsDirective("value",
                                IsBinary(TokenType::Star,
                                         IsNumber("60"),
                                         IsNumber("5")
                                ))
                }
            }
        );
    }
}
