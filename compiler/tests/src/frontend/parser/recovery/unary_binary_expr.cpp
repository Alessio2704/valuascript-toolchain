#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class UnaryBinaryErrorTest : public ParserTestBase
    {
    };

    TEST_F(UnaryBinaryErrorTest, ChainingNotAllowedForComparisonOperations1)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 15, 1, 16);

        ExpectParseErrorsWithRecovery(
            "let a = 1 < 2 < 3",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Less,
                                          IsBinary(TokenType::Less,
                                                   IsNumber("1"),
                                                   IsNumber("2")
                                          ),
                                          IsNumber("3")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinaryErrorTest, ChainingNotAllowedForComparisonOperations2)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 15, 1, 16);

        ExpectParseErrorsWithRecovery(
            "let a = 1 > 2 > 3",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Greater,
                                          IsBinary(TokenType::Greater,
                                                   IsNumber("1"),
                                                   IsNumber("2")
                                          ),
                                          IsNumber("3")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinaryErrorTest, ChainingNotAllowedForComparisonOperations3)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 16, 1, 18);

        ExpectParseErrorsWithRecovery(
            "let a = 1 != 2 != 3",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::NotEquals,
                                          IsBinary(TokenType::NotEquals,
                                                   IsNumber("1"),
                                                   IsNumber("2")
                                          ),
                                          IsNumber("3")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinaryErrorTest, ChainingNotAllowedForComparisonOperations4)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 16, 1, 18);

        ExpectParseErrorsWithRecovery(
            "let a = x == y == z",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Equals,
                                          IsBinary(TokenType::Equals,
                                                   IsIdentifier("x"),
                                                   IsIdentifier("y")
                                          ),
                                          IsIdentifier("z")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinaryErrorTest, ChainingNotAllowedForComparisonOperationsMixedOperators1)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 15, 1, 16);

        ExpectParseErrorsWithRecovery(
            "let a = 1 < 2 > 3",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Greater,
                                          IsBinary(TokenType::Less,
                                                   IsNumber("1"),
                                                   IsNumber("2")
                                          ),
                                          IsNumber("3")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinaryErrorTest, ChainingNotAllowedForComparisonOperationsMixedOperators2)
    {
        std::vector<ExpectedError> errors;
        errors.emplace_back(ValuascriptErrorCode::ChainingNotAllowedForComparisonOperations, 1, 16, 1, 18);

        ExpectParseErrorsWithRecovery(
            "let a = 1 == 2 != 3",
            errors,
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::NotEquals,
                                          IsBinary(TokenType::Equals,
                                                   IsNumber("1"),
                                                   IsNumber("2")
                                          ),
                                          IsNumber("3")
                                 )
                    )
                }
            }
        );
    }
}
