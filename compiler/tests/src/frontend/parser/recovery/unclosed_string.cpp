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
            {PErr{.code = LexerErrorCode::UnclosedString, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 24}},
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
            {PErr{.code = LexerErrorCode::UnclosedString, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 22}},
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
            {PErr{.code = LexerErrorCode::UnclosedString, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 20}},
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
                PErr{.code = LexerErrorCode::UnclosedString, .line_start = 2, .column_start = 1, .line_end = 4, .column_end = 3},
                PErr{.code = E::ExpectedRightBraceAfterFunctionBody, .line_start = 4, .column_start = 1, .line_end = 4, .column_end = 2}
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
                PErr{.code = LexerErrorCode::UnclosedString, .line_start = 1, .column_start = 19, .line_end = 1, .column_end = 29},
                PErr{.code = E::ExpectedRightParenAfterTupleElements, .line_start = 1, .column_start = 28, .line_end = 1, .column_end = 29}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec{.name = "t"}
                        },
                        IsTuple(
                            IsString("\"first\""),
                            IsString("\"unclosed)")
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
                PErr{.code = LexerErrorCode::UnclosedString, .line_start = 1, .column_start = 16, .line_end = 1, .column_end = 31},
                PErr{.code = E::UnmatchedBraceInDictionaryLiteral, .line_start = 1, .column_start = 30, .line_end = 1, .column_end = 31}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec{.name = "d"}
                        },
                        IsDict(
                            DictItemSpec{.key = "key", .value_v = IsString("\"unclosed_val }")}
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
                PErr{.code = LexerErrorCode::UnclosedString, .line_start = 1, .column_start = 17, .line_end = 1, .column_end = 23},
                PErr{.code = E::UnmatchedBracketAfterTensorElements, .line_start = 1, .column_start = 22, .line_end = 1, .column_end = 23}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec{.name = "v"}
                        },
                        IsTensor(
                            IsString("\"val1\""),
                            IsString("\"val2]")
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
                PErr{.code = LexerErrorCode::UnclosedString, .line_start = 1, .column_start = 31, .line_end = 1, .column_end = 41},
                PErr{.code = E::ExpectedRightBraceAfterEnumBody, .line_start = 1, .column_start = 40, .line_end = 1, .column_end = 41}
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
                PErr{.code = LexerErrorCode::UnclosedString, .line_start = 1, .column_start = 25, .line_end = 1, .column_end = 35},
                PErr{.code = E::UnmatchedParenthesisAfterModifierArgs, .line_start = 1, .column_start = 34, .line_end = 1, .column_end = 35}
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
