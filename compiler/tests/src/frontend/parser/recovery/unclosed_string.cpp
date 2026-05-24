#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class UnclosedStringErrorTest : public ParserTestBase
    {
    };

    using E = ParserErrorCode;

    TEST_F(UnclosedStringErrorTest, ImportStatementWithUnclosedString)
    {
        ExpectParseErrorsWithRecovery(
            "import \"/path/to/module",
            {{LexerErrorCode::UnclosedString, 1, 8, 1, 24}},
            ProgramSpec{
                .imports = {
                    IsImport("\"/path/to/module")
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, DirectiveWithUnclosedString)
    {
        ExpectParseErrorsWithRecovery(
            "#config \"unclosed_val",
            {{LexerErrorCode::UnclosedString, 1, 9, 1, 22}},
            ProgramSpec{
                .directives = {
                    IsDirective("config", IsString("\"unclosed_val"))
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, DirectiveAssignmentWithUnclosedString)
    {
        ExpectParseErrorsWithRecovery(
            "#mode = \"debug_mode",
            {{LexerErrorCode::UnclosedString, 1, 9, 1, 20}},
            ProgramSpec{
                .directives = {
                    IsDirective("mode", IsString("\"debug_mode"))
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, FunctionDocstringUnclosed)
    {
        ExpectParseErrors(
            "func test() -> void {\n"
            "\"\"\"This is a broken docstring\n"
            "let x = 1\n"
            "}",
            {
                {LexerErrorCode::UnclosedString, 2, 1, 4, 3},
                {E::ExpectedRightBraceAfterFunctionBody, 4, 2, 4, 3}
            },
            ProgramSpec{
                .functions = {
                    IsFunctionDef(
                        "test",
                        {},
                        {},
                        {IsType("void")},
                        {},
                        "\"\"\"This is a broken docstring\nlet x = 1\n}"
                    )
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, TupleLiteralWithUnclosedString)
    {
        ExpectParseErrorsWithRecovery(
            R"(let t = ("first", "unclosed))",
            {
                {LexerErrorCode::UnclosedString, 1, 19, 1, 29},
                {E::ExpectedRightParenAfterTupleElements, 1, 29, 1, 30}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"t"}
                        },
                        IsTuple(
                            {
                                IsString("\"first\""),
                                IsString("\"unclosed)")
                            }
                        )
                    )
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, DictLiteralWithUnclosedStringValue)
    {
        ExpectParseErrorsWithRecovery(
            "let d = { key: \"unclosed_val }",
            {
                {LexerErrorCode::UnclosedString, 1, 16, 1, 31},
                {E::UnmatchedBraceInDictionaryLiteral, 1, 31, 1, 32}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"d"}
                        },
                        IsDict(
                            {
                                {"key", {}, IsString("\"unclosed_val }")}
                            }
                        )
                    )
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, TensorLiteralWithUnclosedString)
    {
        ExpectParseErrorsWithRecovery(
            R"(let v =["val1", "val2])",
            {
                {LexerErrorCode::UnclosedString, 1, 17, 1, 23},
                {E::UnmatchedBracketAfterTensorElements, 1, 23, 1, 24}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"v"}
                        },
                        IsTensor(
                            {
                                IsString("\"val1\""),
                                IsString("\"val2]")
                            }
                        )
                    )
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, EnumCaseValueWithUnclosedString)
    {
        ExpectParseErrorsWithRecovery(
            "enum Status: string { Error = \"failure }",
            {
                {LexerErrorCode::UnclosedString, 1, 31, 1, 41},
                {E::ExpectedRightBraceAfterEnumBody, 1, 41, 1, 42}
            },
            ProgramSpec{
                .enums = {
                    IsEnumDef("Status", {},
                              IsType("string"),
                              {
                                  {"Error", {}, IsString("\"failure }")}
                              }
                    )
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, ModifierArgumentWithUnclosedString)
    {
        ExpectParseErrorsWithRecovery(
            "let @deprecated(reason: \"not_safe)\n"
            "x = 1",
            {
                {LexerErrorCode::UnclosedString, 1, 25, 1, 35},
                {E::UnmatchedParenthesisAfterModifierArgs, 1, 35, 1, 36}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({
                                     {
                                         {
                                             {
                                                 "deprecated", {
                                                     {
                                                         "reason", IsString("\"not_safe)")
                                                     }
                                                 }
                                             }
                                         },
                                         "x"
                                     }
                                 },
                                 IsNumber("1")
                    )
                }
            }
        );
    }
}
