#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class UnaryBinarySuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(UnaryBinarySuccessPathTest, StandardPrecedence)
    {
        ExpectValidParse(
            "let a = 1 + 2 * 3 ^ 4",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Plus,
                                          IsNumber("1"),
                                          IsBinary(TokenType::Star,
                                                   IsNumber("2"),
                                                   IsBinary(TokenType::Caret,
                                                            IsNumber("3"),
                                                            IsNumber("4")
                                                   )
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LeftAssociativitySubtraction)
    {
        ExpectValidParse(
            "let a = 10 - 5 - 2",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Minus,
                                          IsBinary(TokenType::Minus,
                                                   IsNumber("10"),
                                                   IsNumber("5")
                                          ),
                                          IsNumber("2")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LeftAssociativityDivision)
    {
        ExpectValidParse(
            "let a = 20 / 5 / 2",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Slash,
                                          IsBinary(TokenType::Slash,
                                                   IsNumber("20"),
                                                   IsNumber("5")
                                          ),
                                          IsNumber("2")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, UnaryMinusPrecedence)
    {
        ExpectValidParse(
            "let a = -5 * 2",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Star,
                                          IsUnary(TokenType::Minus,
                                                  IsNumber("5")
                                          ),
                                          IsNumber("2")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ParenthesesOverride)
    {
        ExpectValidParse(
            "let a = (1 + 2) * 3",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Star,
                                          IsGrouping(
                                              IsBinary(TokenType::Plus,
                                                       IsNumber("1"),
                                                       IsNumber("2")
                                              )
                                          ),
                                          IsNumber("3")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, DeepParenthesesNesting)
    {
        ExpectValidParse(
            "let a = ((1 + 2) * (3 - 4)) / 5",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Slash,
                                          IsGrouping(
                                              IsBinary(TokenType::Star,
                                                       IsGrouping(
                                                           IsBinary(TokenType::Plus,
                                                                    IsNumber("1"),
                                                                    IsNumber("2")
                                                           )
                                                       ),
                                                       IsGrouping(
                                                           IsBinary(TokenType::Minus,
                                                                    IsNumber("3"),
                                                                    IsNumber("4")
                                                           )
                                                       )
                                              )
                                          ),
                                          IsNumber("5")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, RelationalPrecedence)
    {
        ExpectValidParse(
            "let a = 1 + 2 > 3 * 4",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Greater,
                                          IsBinary(TokenType::Plus,
                                                   IsNumber("1"),
                                                   IsNumber("2")
                                          ),
                                          IsBinary(TokenType::Star,
                                                   IsNumber("3"),
                                                   IsNumber("4")
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, EqualityPrecedence)
    {
        ExpectValidParse(
            "let a = x == y + 1",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Equals,
                                          IsIdentifier("x"),
                                          IsBinary(TokenType::Plus,
                                                   IsIdentifier("y"),
                                                   IsNumber("1")
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ModuloPrecedence)
    {
        ExpectValidParse(
            "let a = 10 mod 3 * 2",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Star,
                                          IsBinary(TokenType::Mod,
                                                   IsNumber("10"),
                                                   IsNumber("3")
                                          ),
                                          IsNumber("2")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, RightAssociativityPower)
    {
        ExpectValidParse(
            "let a = 2 ^ 3 ^ 4",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Caret,
                                          IsNumber("2"),
                                          IsBinary(TokenType::Caret,
                                                   IsNumber("3"),
                                                   IsNumber("4")
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LogicalAndOrPrecedence)
    {
        ExpectValidParse(
            "let a = true or false and true",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Or,
                                          IsBoolean(true),
                                          IsBinary(TokenType::And,
                                                   IsBoolean(false),
                                                   IsBoolean(true)
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, RelationalBeforeLogical)
    {
        ExpectValidParse(
            "let a = x > y and z == w",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::And,
                                          IsBinary(TokenType::Greater,
                                                   IsIdentifier("x"),
                                                   IsIdentifier("y")
                                          ),
                                          IsBinary(TokenType::Equals,
                                                   IsIdentifier("z"),
                                                   IsIdentifier("w")
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, PostfixAndPrefixPrecedence)
    {
        ExpectValidParse(
            "let a = -b() * 2",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Star,
                                          IsUnary(TokenType::Minus,
                                                  IsCall(IsIdentifier("b"))
                                          ),
                                          IsNumber("2")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, BracketAccessPrecedence)
    {
        ExpectValidParse(
            "let a = b[0] * 2",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Star,
                                          IsBracket(
                                              IsIdentifier("b"),
                                              IsNumber("0")
                                          ),
                                          IsNumber("2")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, PercentageInMathExpressions)
    {
        ExpectValidParse(
            "let total = 100 * 5% + 2%",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"total"}},
                                 IsBinary(TokenType::Plus,
                                          IsBinary(TokenType::Star,
                                                   IsNumber("100"),
                                                   IsPercentage("5%")
                                          ),
                                          IsPercentage("2%")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ArithmeticNegation)
    {
        ExpectValidParse(
            "let a = -5",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsUnary(TokenType::Minus,
                                         IsNumber("5")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LogicalInversion)
    {
        ExpectValidParse(
            "let a = not is_active",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsUnary(TokenType::Not,
                                         IsIdentifier("is_active")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, UnaryPrecedenceOverBinary)
    {
        ExpectValidParse(
            "let a = -a * b",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Star,
                                          IsUnary(TokenType::Minus,
                                                  IsIdentifier("a")
                                          ),
                                          IsIdentifier("b")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NestedUnaryChaining1)
    {
        ExpectValidParse(
            "let a = not not flag",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsUnary(TokenType::Not,
                                         IsUnary(TokenType::Not,
                                                 IsIdentifier("flag")
                                         )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NestedUnaryChaining2)
    {
        ExpectValidParse(
            "let a = + + flag",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsUnary(TokenType::Plus,
                                         IsUnary(TokenType::Plus,
                                                 IsIdentifier("flag")
                                         )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NestedUnaryChaining3)
    {
        ExpectValidParse(
            "let a = - - flag",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsUnary(TokenType::Minus,
                                         IsUnary(TokenType::Minus,
                                                 IsIdentifier("flag")
                                         )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, DeeplyNestedUnaryAndBinaryMath)
    {
        ExpectValidParse(
            "let a = -a * (b + c) - (not d) / e",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Minus,
                                          IsBinary(TokenType::Star,
                                                   IsUnary(TokenType::Minus,
                                                           IsIdentifier("a")
                                                   ),
                                                   IsGrouping(
                                                       IsBinary(TokenType::Plus,
                                                                IsIdentifier("b"),
                                                                IsIdentifier("c")
                                                       )
                                                   )
                                          ),
                                          IsBinary(TokenType::Slash,
                                                   IsGrouping(
                                                       IsUnary(TokenType::Not,
                                                               IsIdentifier("d")
                                                       )
                                                   ),
                                                   IsIdentifier("e")
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, DeeplyNestedUnaryAndBinaryMath2)
    {
        ExpectValidParse(
            "let a = -a * (b + c) - not d / e",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Minus,
                                          IsBinary(TokenType::Star,
                                                   IsUnary(TokenType::Minus,
                                                           IsIdentifier("a")
                                                   ),
                                                   IsGrouping(
                                                       IsBinary(TokenType::Plus,
                                                                IsIdentifier("b"),
                                                                IsIdentifier("c")
                                                       )
                                                   )
                                          ),
                                          IsBinary(TokenType::Slash,
                                                   IsUnary(TokenType::Not,
                                                           IsIdentifier("d")
                                                   ),
                                                   IsIdentifier("e")
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, OrExpr)
    {
        ExpectValidParse(
            "let a = x or y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Or,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, AndExpr)
    {
        ExpectValidParse(
            "let a = x and y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::And,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NotExpr)
    {
        ExpectValidParse(
            "let a = not x",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsUnary(TokenType::Not,
                                         IsIdentifier("x")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, EqExpr)
    {
        ExpectValidParse(
            "let a = x == y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Equals,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NeqExpr)
    {
        ExpectValidParse(
            "let a = x != y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::NotEquals,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, GtExpr)
    {
        ExpectValidParse(
            "let a = x > y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Greater,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LtExpr)
    {
        ExpectValidParse(
            "let a = x < y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Less,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, GteExpr)
    {
        ExpectValidParse(
            "let a = x >= y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::GreaterEqual,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LteExpr)
    {
        ExpectValidParse(
            "let a = x <= y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::LessEqual,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, PowExpr)
    {
        ExpectValidParse(
            "let a = x ^ y",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Caret,
                                          IsIdentifier("x"),
                                          IsIdentifier("y")
                                 )
                    )
                }
            }
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, BoolGrouped)
    {
        ExpectValidParse(
            "let a = (a and b) or (c and not d)",
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({}, {{"a"}},
                                 IsBinary(TokenType::Or,
                                          IsGrouping(
                                              IsBinary(TokenType::And,
                                                       IsIdentifier("a"),
                                                       IsIdentifier("b")
                                              )
                                          ),
                                          IsGrouping(
                                              IsBinary(TokenType::And,
                                                       IsIdentifier("c"),
                                                       IsUnary(TokenType::Not,
                                                               IsIdentifier("d")
                                                       )
                                              )
                                          )
                                 )
                    )
                }
            }
        );
    }
}
