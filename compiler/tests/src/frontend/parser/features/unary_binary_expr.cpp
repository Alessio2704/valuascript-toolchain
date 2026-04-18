#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class UnaryBinarySuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(UnaryBinarySuccessPathTest, StandardPrecedence)
    {
        ExpectValidExpression("1 + 2 * 3 ^ 4",
                              IsBinary(TokenType::Plus,
                                       IsNumber("1"),
                                       IsBinary(TokenType::Star,
                                                IsNumber("2"),
                                                IsBinary(TokenType::Caret,
                                                         IsNumber("3"),
                                                         IsNumber("4")
                                                )
                                       )
                              ));
    }

    TEST_F(UnaryBinarySuccessPathTest, LeftAssociativitySubtraction)
    {
        ExpectValidExpression(
            "10 - 5 - 2",
            IsBinary(TokenType::Minus,
                     IsBinary(TokenType::Minus,
                              IsNumber("10"),
                              IsNumber("5")
                     ),
                     IsNumber("2")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LeftAssociativityDivision)
    {
        ExpectValidExpression(
            "20 / 5 / 2",
            IsBinary(TokenType::Slash,
                     IsBinary(TokenType::Slash,
                              IsNumber("20"),
                              IsNumber("5")
                     ),
                     IsNumber("2")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, FactorLeftAssociativityMixed)
    {
        ExpectValidExpression(
            "a * b / c mod d",
            IsBinary(TokenType::Mod,
                     IsBinary(TokenType::Slash,
                              IsBinary(TokenType::Star,
                                       IsIdentifier("a"),
                                       IsIdentifier("b")
                              ),
                              IsIdentifier("c")
                     ),
                     IsIdentifier("d")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, UnaryMinusPrecedence)
    {
        ExpectValidExpression(
            "-5 * 2",
            IsBinary(TokenType::Star,
                     IsUnary(TokenType::Minus,
                             IsNumber("5")
                     ),
                     IsNumber("2")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, UnaryPlusBasic)
    {
        ExpectValidExpression(
            "+42",
            IsUnary(TokenType::Plus,
                    IsNumber("42")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, UnaryBindsTighterThanPower)
    {
        ExpectValidExpression(
            "-2 ^ 2",
            IsBinary(TokenType::Caret,
                     IsUnary(TokenType::Minus,
                             IsNumber("2")
                     ),
                     IsNumber("2")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, MixedRightAssociativityAndUnary)
    {
        ExpectValidExpression(
            "2 ^ -3 ^ 4",
            IsBinary(TokenType::Caret,
                     IsNumber("2"),
                     IsBinary(TokenType::Caret,
                              IsUnary(TokenType::Minus,
                                      IsNumber("3")
                              ),
                              IsNumber("4")
                     )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ConsecutiveDifferentUnary)
    {
        ExpectValidExpression(
            "not - + x",
            IsUnary(TokenType::Not,
                    IsUnary(TokenType::Minus,
                            IsUnary(TokenType::Plus,
                                    IsIdentifier("x")
                            )
                    )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ParenthesesOverride)
    {
        ExpectValidExpression(
            "(1 + 2) * 3",
            IsBinary(TokenType::Star,
                     IsGrouping(
                         IsBinary(TokenType::Plus,
                                  IsNumber("1"),
                                  IsNumber("2")
                         )
                     ),
                     IsNumber("3")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, DeepParenthesesNesting)
    {
        ExpectValidExpression(
            "((1 + 2) * (3 - 4)) / 5",
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
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, GroupingRedundancy)
    {
        ExpectValidExpression(
            "(((x))) + 1",
            IsBinary(TokenType::Plus,
                     IsGrouping(
                         IsGrouping(
                             IsGrouping(
                                 IsIdentifier("x")
                             )
                         )
                     ),
                     IsNumber("1")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, RelationalPrecedence)
    {
        ExpectValidExpression(
            "1 + 2 > 3 * 4",
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
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, EqualityPrecedence)
    {
        ExpectValidExpression(
            "x == y + 1",
            IsBinary(TokenType::Equals,
                     IsIdentifier("x"),
                     IsBinary(TokenType::Plus,
                              IsIdentifier("y"),
                              IsNumber("1")
                     )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ModuloPrecedence)
    {
        ExpectValidExpression(
            "10 mod 3 * 2",
            IsBinary(TokenType::Star,
                     IsBinary(TokenType::Mod,
                              IsNumber("10"),
                              IsNumber("3")
                     ),
                     IsNumber("2")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, RightAssociativityPower)
    {
        ExpectValidExpression(
            "2 ^ 3 ^ 4",
            IsBinary(TokenType::Caret,
                     IsNumber("2"),
                     IsBinary(TokenType::Caret,
                              IsNumber("3"),
                              IsNumber("4")
                     )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, RightAssociativityPowerMixed)
    {
        ExpectValidExpression(
            "2 ^ 3 ^ 4 * 5 + 6",
            IsBinary(TokenType::Plus,
                     IsBinary(TokenType::Star,
                              IsBinary(TokenType::Caret,
                                       IsNumber("2"),
                                       IsBinary(TokenType::Caret,
                                                IsNumber("3"),
                                                IsNumber("4")
                                       )
                              ),
                              IsNumber("5")
                     ),
                     IsNumber("6")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LogicalAndOrPrecedence)
    {
        ExpectValidExpression(
            "true or false and true",
            IsBinary(TokenType::Or,
                     IsBoolean(true),
                     IsBinary(TokenType::And,
                              IsBoolean(false),
                              IsBoolean(true)
                     )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, RelationalBeforeLogical)
    {
        ExpectValidExpression(
            "x > y and z == w",
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
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LogicalMixedWithArithmeticAndEquality)
    {
        ExpectValidExpression(
            "a + 1 == b and c * 2 != d or e",
            IsBinary(TokenType::Or,
                     IsBinary(TokenType::And,
                              IsBinary(TokenType::Equals,
                                       IsBinary(TokenType::Plus,
                                                IsIdentifier("a"),
                                                IsNumber("1")
                                       ),
                                       IsIdentifier("b")
                              ),
                              IsBinary(TokenType::NotEquals,
                                       IsBinary(TokenType::Star,
                                                IsIdentifier("c"),
                                                IsNumber("2")
                                       ),
                                       IsIdentifier("d")
                              )
                     ),
                     IsIdentifier("e")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, StringConcatenation)
    {
        ExpectValidExpression(
            "\"hello\" + \" \" + \"world\"",
            IsBinary(TokenType::Plus,
                     IsBinary(TokenType::Plus,
                              IsString("\"hello\""),
                              IsString("\" \"")
                     ),
                     IsString("\"world\"")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, PrefixPostfixInteraction)
    {
        ExpectValidExpression(
            "not a.b[0]()",
            IsUnary(TokenType::Not,
                    IsCall(
                        IsBracket(
                            IsDot(IsIdentifier("a"), "b"),
                            IsNumber("0")
                        )
                    )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, PostfixAndPrefixPrecedence)
    {
        ExpectValidExpression(
            "-b() * 2",
            IsBinary(TokenType::Star,
                     IsUnary(TokenType::Minus,
                             IsCall(IsIdentifier("b"))
                     ),
                     IsNumber("2")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, BracketAccessPrecedence)
    {
        ExpectValidExpression(
            "b[0] * 2",
            IsBinary(TokenType::Star,
                     IsBracket(
                         IsIdentifier("b"),
                         IsNumber("0")
                     ),
                     IsNumber("2")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, PercentageInMathExpressions)
    {
        ExpectValidExpression(
            "100 * 5% + 2%",
            IsBinary(TokenType::Plus,
                     IsBinary(TokenType::Star,
                              IsNumber("100"),
                              IsPercentage("5%")
                     ),
                     IsPercentage("2%")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ArithmeticNegation)
    {
        ExpectValidExpression(
            "-5",
            IsUnary(TokenType::Minus,
                    IsNumber("5")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LogicalInversion)
    {
        ExpectValidExpression(
            "not is_active",
            IsUnary(TokenType::Not,
                    IsIdentifier("is_active")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, UnaryPrecedenceOverBinary)
    {
        ExpectValidExpression(
            "-a * b",
            IsBinary(TokenType::Star,
                     IsUnary(TokenType::Minus,
                             IsIdentifier("a")
                     ),
                     IsIdentifier("b")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NestedUnaryChaining1)
    {
        ExpectValidExpression(
            "not not flag",
            IsUnary(TokenType::Not,
                    IsUnary(TokenType::Not,
                            IsIdentifier("flag")
                    )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NestedUnaryChaining2)
    {
        ExpectValidExpression(
            "+ + flag",
            IsUnary(TokenType::Plus,
                    IsUnary(TokenType::Plus,
                            IsIdentifier("flag")
                    )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NestedUnaryChaining3)
    {
        ExpectValidExpression(
            "- - flag",
            IsUnary(TokenType::Minus,
                    IsUnary(TokenType::Minus,
                            IsIdentifier("flag")
                    )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, DeeplyNestedUnaryAndBinaryMath)
    {
        ExpectValidExpression(
            "-a * (b + c) - (not d) / e",
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
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, DeeplyNestedUnaryAndBinaryMath2)
    {
        ExpectValidExpression(
            "-a * (b + c) - not d / e",
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
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ComplexMixedPrecedence)
    {
        ExpectValidExpression(
            "-a.b ^ c[0] * d + e == f and not g or h",
            IsBinary(TokenType::Or,
                     IsBinary(TokenType::And,
                              IsBinary(TokenType::Equals,
                                       IsBinary(TokenType::Plus,
                                                IsBinary(TokenType::Star,
                                                         IsBinary(TokenType::Caret,
                                                                  IsUnary(TokenType::Minus,
                                                                          IsDot(IsIdentifier("a"), "b")
                                                                  ),
                                                                  IsBracket(IsIdentifier("c"), IsNumber("0"))
                                                         ),
                                                         IsIdentifier("d")
                                                ),
                                                IsIdentifier("e")
                                       ),
                                       IsIdentifier("f")
                              ),
                              IsUnary(TokenType::Not,
                                      IsIdentifier("g")
                              )
                     ),
                     IsIdentifier("h")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, OrExpr)
    {
        ExpectValidExpression(
            "x or y",
            IsBinary(TokenType::Or,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, AndExpr)
    {
        ExpectValidExpression(
            "x and y",
            IsBinary(TokenType::And,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NotExpr)
    {
        ExpectValidExpression(
            "not x",
            IsUnary(TokenType::Not,
                    IsIdentifier("x")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, EqExpr)
    {
        ExpectValidExpression(
            "x == y",
            IsBinary(TokenType::Equals,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, NeqExpr)
    {
        ExpectValidExpression(
            "x != y",
            IsBinary(TokenType::NotEquals,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, GtExpr)
    {
        ExpectValidExpression(
            "x > y",
            IsBinary(TokenType::Greater,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LtExpr)
    {
        ExpectValidExpression(
            "x < y",
            IsBinary(TokenType::Less,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, GteExpr)
    {
        ExpectValidExpression(
            "x >= y",
            IsBinary(TokenType::GreaterEqual,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, LteExpr)
    {
        ExpectValidExpression(
            "x <= y",
            IsBinary(TokenType::LessEqual,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, PowExpr)
    {
        ExpectValidExpression(
            "x ^ y",
            IsBinary(TokenType::Caret,
                     IsIdentifier("x"),
                     IsIdentifier("y")
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, BoolGrouped)
    {
        ExpectValidExpression(
            "(a and b) or (c and not d)",
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
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ValidatesBooleanPrecedence)
    {
        ExpectValidExpression(
            "x or y and not z",
            IsBinary(TokenType::Or,
                     IsIdentifier("x"),
                     IsBinary(TokenType::And,
                              IsIdentifier("y"),
                              IsUnary(TokenType::Not,
                                      IsIdentifier("z")
                              )
                     )
            )
        );
    }

    TEST_F(UnaryBinarySuccessPathTest, ValidatesBooleanGrouping)
    {
        ExpectValidExpression(
            "(x or y) and z",
            IsBinary(TokenType::And,
                     IsGrouping(
                         IsBinary(TokenType::Or,
                                  IsIdentifier("x"),
                                  IsIdentifier("y")
                         )
                     ),
                     IsIdentifier("z")
            )
        );
    }
}
