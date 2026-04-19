#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ModifierSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(ModifierSuccessPathTest, SingleModifier)
    {
        ExpectValidModifiers("@simple", {
                                 {"simple"}
                             });
    }

    TEST_F(ModifierSuccessPathTest, SingleModifierWithEmptyParens)
    {
        ExpectValidModifiers("@simple()", {
                                 {"simple"}
                             });
    }

    TEST_F(ModifierSuccessPathTest, MultipleModifiers)
    {
        ExpectValidModifiers("@first @second", {
                                 {"first"},
                                 {"second"}
                             });
    }

    TEST_F(ModifierSuccessPathTest, ModifierWithOneArgument)
    {
        ExpectValidModifiers("@meta(version: 1)", {
                                 {
                                     "meta", {
                                         {"version", IsNumber("1")}
                                     }
                                 }
                             });
    }

    TEST_F(ModifierSuccessPathTest, ModifierWithMultipleArguments)
    {
        ExpectValidModifiers("@config(active: true, retries: 3, strategy: \"fast\")", {
                                 {
                                     "config", {
                                         {"active", IsBoolean(true)},
                                         {"retries", IsNumber("3")},
                                         {"strategy", IsString("\"fast\"")}
                                     }
                                 }
                             });
    }

    TEST_F(ModifierSuccessPathTest, MixedModifiers)
    {
        ExpectValidModifiers("@inline @deprecated(msg: \"old\") @export", {
                                 {"inline"},
                                 {
                                     "deprecated", {
                                         {"msg", IsString("\"old\"")}
                                     }
                                 },
                                 {"export"}
                             });
    }

    TEST_F(ModifierSuccessPathTest, MultilineModifiers)
    {
        ExpectValidModifiers("@export\n@meta(version: 2)\n", {
                                 {"export"},
                                 {
                                     "meta", {
                                         {"version", IsNumber("2")}
                                     }
                                 }
                             });
    }
}
