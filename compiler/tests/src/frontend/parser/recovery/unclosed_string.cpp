#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class UnclosedStringErrorTest : public ParserTestBase
    {
    };

    TEST_F(UnclosedStringErrorTest, ImportStatementWithUnclosedString)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 1, 8, 1, 24);

        ExpectParseErrorsWithRecovery(
            "import \"/path/to/module",
            errors,
            ProgramSpec{
                .imports = {
                    IsImport("\"/path/to/module")
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, DirectiveWithUnclosedString)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 1, 9, 1, 22);

        ExpectParseErrorsWithRecovery(
            "#config \"unclosed_val",
            errors,
            ProgramSpec{
                .directives = {
                    IsDirective("config", IsString("\"unclosed_val"))
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, DirectiveAssignmentWithUnclosedString)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 1, 9, 1, 20);

        ExpectParseErrorsWithRecovery(
            "#mode = \"debug_mode",
            errors,
            ProgramSpec{
                .directives = {
                    IsDirective("mode", IsString("\"debug_mode"))
                }
            }
        );
    }

    TEST_F(UnclosedStringErrorTest, FunctionDocstringUnclosed)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 2, 1, 4, 3);
        errors.emplace_back(ValuascriptErrorCode::ExpectedRightBraceAfterFunctionBody, 4, 2);

        ExpectParseErrors(
            "func test() -> void {\n"
            "\"\"\"This is a broken docstring\n"
            "let x = 1\n"
            "}",
            errors,
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
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 1, 19, 1, 29);
        errors.emplace_back(ValuascriptErrorCode::ExpectedRightParenAfterTupleElements, 1, 29, 1, 30);

        ExpectParseErrorsWithRecovery(
            R"(let t = ("first", "unclosed))",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
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
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 1, 16, 1, 31);
        errors.emplace_back(ValuascriptErrorCode::UnmatchedBraceInDictionaryLiteral, 1, 31);

        ExpectParseErrorsWithRecovery(
            "let d = { key: \"unclosed_val }",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
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
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 1, 17, 1, 23);
        errors.emplace_back(ValuascriptErrorCode::UnmatchedBracketAfterTensorElements, 1, 23);

        ExpectParseErrorsWithRecovery(
            R"(let v =["val1", "val2])",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
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
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 1, 31, 1, 41);
        errors.emplace_back(ValuascriptErrorCode::ExpectedRightBraceAfterEnumBody, 1, 41);

        ExpectParseErrorsWithRecovery(
            "enum Status: string { Error = \"failure }",
            errors,
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
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnclosedString, 1, 21, 1, 31);
        errors.emplace_back(ValuascriptErrorCode::UnmatchedParenthesisAfterModifierArgs, 1, 31);

        ExpectParseErrorsWithRecovery(
            "@deprecated(reason: \"not_safe)\n"
            "let x = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {
                                "deprecated", {
                                    {
                                        "reason", IsString("\"not_safe)")
                                    }
                                }
                            }
                        },
                        {
                            {"x"}
                        },
                        IsNumber("1")
                    )
                }
            }
        );
    }
}
