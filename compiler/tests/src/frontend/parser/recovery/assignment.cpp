#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class AssignmentErrorTest : public ParserTestBase
    {
    };

    TEST_F(AssignmentErrorTest, MissingVariableName)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidIdentifier, 1, 5, 1, 6);

        ExpectParseErrorsWithRecovery(
            "let = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"<error>"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, InvalidCharacter1)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidCharacter, 1, 6, 1, 7);

        ExpectParseErrorsWithRecovery(
            "let a! = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, InvalidCharacter2)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidCharacter, 1, 7, 1, 8);

        ExpectParseErrorsWithRecovery(
            "let a ! = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, InvalidVariableNameStart)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidIdentifier, 1, 5, 1, 8);

        ExpectParseErrorsWithRecovery(
            "let 123 = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"<error>"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, IncompleteAssignmentMissingEquals)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::IncompleteAssignment, 1, 7, 1, 8);

        ExpectParseErrorsWithRecovery(
            "let a 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, IncompleteMultipleAssignmentMissingEquals)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::IncompleteAssignment, 1, 10, 1, 11);

        ExpectParseErrorsWithRecovery(
            "let a, b 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingValueAfterEquals)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingValueAfterEquals, 1, 8, 1, 9);

        ExpectParseErrorsWithRecovery(
            "let a =",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNull())
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingValueAfterEqualsWithTypeAnnotation)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingValueAfterEquals, 1, 13, 1, 14);

        ExpectParseErrorsWithRecovery(
            "let a: int =",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a", IsType("int")}}, IsNull())
                }
            }
        );
    }


    TEST_F(AssignmentErrorTest, IncompleteAssignmentAtEOF)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::IncompleteAssignment, 1, 6, 1, 7);
        errors.emplace_back(ValuascriptErrorCode::MissingValueAfterEquals, 1, 6, 1, 7);

        ExpectParseErrorsWithRecovery(
            "let a",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNull())
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MultiAssignmentTrailingComma)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidIdentifier, 1, 8, 1, 9);

        ExpectParseErrorsWithRecovery(
            "let a, = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"<error>"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MultiAssignmentMissingComma)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ExpectedCommaInMultiAssignment, 1, 7, 1, 8);

        ExpectParseErrorsWithRecovery(
            "let a b = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MultiAssignmentDoubleComma)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::InvalidIdentifier, 1, 7, 1, 8);

        ExpectParseErrorsWithRecovery(
            "let a,, b = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"<error>"}, {"b"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, IllegalModifierOnSingleElementOfMultiAssignment)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ModifiersAttachedToMultiAssignmentSingleElements, 1, 8, 1, 15);

        ExpectParseErrorsWithRecovery(
            "let a, @export b = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingTypeAfterColon)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingTypeAnnotation, 1, 8, 1, 9);

        ExpectParseErrorsWithRecovery(
            "let a: = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a", IsNullType()}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, BrokenNestedTypeAnnotation)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::UnmatchedBracketAfterGenericArgs, 1, 19, 1, 20);

        ExpectParseErrorsWithRecovery(
            "let a: vector<int = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a", IsType("vector", {IsType("int")})}
                                 },
                                 IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, ReservedKeywordAsTarget)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ReservedKeywordAsIdentifier, 1, 5, 1, 9);

        ExpectParseErrorsWithRecovery(
            "let func = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"func"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, ReservedKeywordInMultiAssignment)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ReservedKeywordAsIdentifier, 1, 8, 1, 10);

        ExpectParseErrorsWithRecovery(
            "let a, if = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"if"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingModifierNameAfterAt)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ExpectedModifierName, 1, 3, 1, 6);

        ExpectParseErrorsWithRecovery(
            "@ let a = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"<error>"}}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MultipleBrokenModifiers)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ExpectedModifierName, 1, 8, 1, 11);

        ExpectParseErrorsWithRecovery(
            "@mod1 @123 let a = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"mod1"}, {"<error>"}}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingTypeAfterColonInMultiAssignment)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingTypeAnnotation, 1, 20, 1, 21);

        ExpectParseErrorsWithRecovery(
            "let a: integer, b: = 1",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({},
                                 {
                                     {"a", IsType("integer")},
                                     {"b", IsNullType()}
                                 },
                                 IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingValueAfterEqualsMultiAssignment)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::MissingValueAfterEquals, 1, 11, 1, 12);

        ExpectParseErrorsWithRecovery(
            "let x, y = ",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"x"}, {"y"}}, IsNull())
                }
            }
        );
    }
}
