#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test {
    class AssignmentErrorTest : public ParserTestBase {
    };

    TEST_F(AssignmentErrorTest, MissingVariableName) {
        ExpectParseErrorsWithRecovery(
            "let = 1",
            {{ValuascriptErrorCode::InvalidIdentifier, 1, 5, 1, 6}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"<error>"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, InvalidCharacter1) {
        ExpectParseErrorsWithRecovery(
            "let a! = 1",
            {{ValuascriptErrorCode::InvalidCharacter, 1, 6, 1, 7}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, InvalidCharacter2) {
        ExpectParseErrorsWithRecovery(
            "let a ! = 1",
            {{ValuascriptErrorCode::InvalidCharacter, 1, 7, 1, 8}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, InvalidVariableNameStart) {
        ExpectParseErrorsWithRecovery(
            "let 123 = 1",
            {{ValuascriptErrorCode::InvalidIdentifier, 1, 5, 1, 8}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"<error>"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, IncompleteAssignmentMissingEquals) {
        ExpectParseErrorsWithRecovery(
            "let a 1",
            {{ValuascriptErrorCode::IncompleteAssignment, 1, 7, 1, 8}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, IncompleteMultipleAssignmentMissingEquals) {
        ExpectParseErrorsWithRecovery(
            "let a, b 1",
            {{ValuascriptErrorCode::IncompleteAssignment, 1, 10, 1, 11}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingValueAfterEquals) {
        ExpectParseErrorsWithRecovery(
            "let a =",
            {{ValuascriptErrorCode::MissingValueAfterEquals, 1, 8, 1, 9}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, nullptr)
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingValueAfterEqualsWithTypeAnnotation) {
        ExpectParseErrorsWithRecovery(
            "let a: int =",
            {{ValuascriptErrorCode::MissingValueAfterEquals, 1, 13, 1, 14}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a", IsType("int")}}, nullptr)
                }
            }
        );
    }


    TEST_F(AssignmentErrorTest, IncompleteAssignmentAtEOF) {
        ExpectParseErrorsWithRecovery(
            "let a",
            {
                {ValuascriptErrorCode::IncompleteAssignment, 1, 6, 1, 7},
                {ValuascriptErrorCode::MissingValueAfterEquals, 1, 6, 1, 7},
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}}, nullptr)
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MultiAssignmentTrailingComma) {
        ExpectParseErrorsWithRecovery(
            "let a, = 1",
            {{ValuascriptErrorCode::InvalidIdentifier, 1, 8, 1, 9}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"<error>"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MultiAssignmentMissingComma) {
        ExpectParseErrorsWithRecovery(
            "let a b = 1",
            {{ValuascriptErrorCode::ExpectedCommaInMultiAssignment, 1, 7, 1, 8}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MultiAssignmentDoubleComma) {
        ExpectParseErrorsWithRecovery(
            "let a,, b = 1",
            {{ValuascriptErrorCode::InvalidIdentifier, 1, 7, 1, 8}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"},{"<error>"}, {"b"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, IllegalModifierOnSingleElementOfMultiAssignment) {
        ExpectParseErrorsWithRecovery(
            "let a, @export b = 1",
            {{ValuascriptErrorCode::ModifiersAttachedToMultiAssignmentSingleElements, 1, 8, 1, 15}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingTypeAfterColon) {
        ExpectParseErrorsWithRecovery(
            "let a: = 1",
            {{ValuascriptErrorCode::MissingTypeAnnotation, 1, 8, 1, 9}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a", nullptr}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, BrokenNestedTypeAnnotation) {
        ExpectParseErrorsWithRecovery(
            "let a: vector<int = 1",
            {{ValuascriptErrorCode::UnmatchedBracketAfterGenericArgs, 1, 19, 1, 20}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a", IsType("vector", {IsType("int")})}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, ReservedKeywordAsTarget) {
        ExpectParseErrorsWithRecovery(
            "let func = 1",
            {{ValuascriptErrorCode::ReservedKeywordAsIdentifier, 1, 5, 1, 9}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"func"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, ReservedKeywordInMultiAssignment) {
        ExpectParseErrorsWithRecovery(
            "let a, if = 1",
            {{ValuascriptErrorCode::ReservedKeywordAsIdentifier, 1, 8, 1, 10}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}, {"if"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingModifierNameAfterAt) {
        ExpectParseErrorsWithRecovery(
            "@ let a = 1",
            {{ValuascriptErrorCode::ExpectedModifierName, 1, 3, 1, 6}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"<error>"}}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MultipleBrokenModifiers) {
        ExpectParseErrorsWithRecovery(
            "@mod1 @123 let a = 1",
            {{ValuascriptErrorCode::ExpectedModifierName, 1, 8, 1, 11}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"mod1"}, {"<error>"}}, {{"a"}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingTypeAfterColonInMultiAssignment) {
        ExpectParseErrorsWithRecovery(
            "let a: integer, b: = 1",
            {{ValuascriptErrorCode::MissingTypeAnnotation, 1, 20, 1, 21}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a", IsType("integer")}, {"b", nullptr}}, IsNumber("1"))
                }
            }
        );
    }

    TEST_F(AssignmentErrorTest, MissingValueAfterEqualsMultiAssignment) {
        ExpectParseErrorsWithRecovery(
            "let x, y = ",
            {{ValuascriptErrorCode::MissingValueAfterEquals, 1, 11, 1, 12}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"x"}, {"y"}}, nullptr)
                }
            }
        );
    }
}
