#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("StandardPrecedence",
                "1 + 2 * 3 ^ 4",
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

            reg("LeftAssociativitySubtraction",
                "10 - 5 - 2",
                IsBinary(TokenType::Minus,
                         IsBinary(TokenType::Minus,
                                  IsNumber("10"),
                                  IsNumber("5")
                         ),
                         IsNumber("2")
                ));

            reg("LeftAssociativityDivision",
                "20 / 5 / 2",
                IsBinary(TokenType::Slash,
                         IsBinary(TokenType::Slash,
                                  IsNumber("20"),
                                  IsNumber("5")
                         ),
                         IsNumber("2")
                ));

            reg("FactorLeftAssociativityMixed",
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
                ));

            reg("UnaryMinusPrecedence",
                "-5 * 2",
                IsBinary(TokenType::Star,
                         IsUnary(TokenType::Minus,
                                 IsNumber("5")
                         ),
                         IsNumber("2")
                ));

            reg("UnaryPlusBasic",
                "+42",
                IsUnary(TokenType::Plus,
                        IsNumber("42")
                ));

            reg("UnaryBindsTighterThanPower",
                "-2 ^ 2",
                IsBinary(TokenType::Caret,
                         IsUnary(TokenType::Minus,
                                 IsNumber("2")
                         ),
                         IsNumber("2")
                ));

            reg("MixedRightAssociativityAndUnary",
                "2 ^ -3 ^ 4",
                IsBinary(TokenType::Caret,
                         IsNumber("2"),
                         IsBinary(TokenType::Caret,
                                  IsUnary(TokenType::Minus,
                                          IsNumber("3")
                                  ),
                                  IsNumber("4")
                         )
                ));

            reg("ConsecutiveDifferentUnary",
                "not - + x",
                IsUnary(TokenType::Not,
                        IsUnary(TokenType::Minus,
                                IsUnary(TokenType::Plus,
                                        IsIdentifier("x")
                                )
                        )
                ));

            reg("ParenthesesOverride",
                "(1 + 2) * 3",
                IsBinary(TokenType::Star,
                         IsGrouping(
                             IsBinary(TokenType::Plus,
                                      IsNumber("1"),
                                      IsNumber("2")
                             )
                         ),
                         IsNumber("3")
                ));

            reg("DeepParenthesesNesting",
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
                ));

            reg("GroupingRedundancy",
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
                ));

            reg("RelationalPrecedence",
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
                ));

            reg("EqualityPrecedence",
                "x == y + 1",
                IsBinary(TokenType::Equals,
                         IsIdentifier("x"),
                         IsBinary(TokenType::Plus,
                                  IsIdentifier("y"),
                                  IsNumber("1")
                         )
                ));

            reg("ModuloPrecedence",
                "10 mod 3 * 2",
                IsBinary(TokenType::Star,
                         IsBinary(TokenType::Mod,
                                  IsNumber("10"),
                                  IsNumber("3")
                         ),
                         IsNumber("2")
                ));

            reg("RightAssociativityPower",
                "2 ^ 3 ^ 4",
                IsBinary(TokenType::Caret,
                         IsNumber("2"),
                         IsBinary(TokenType::Caret,
                                  IsNumber("3"),
                                  IsNumber("4")
                         )
                ));

            reg("RightAssociativityPowerMixed",
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
                ));

            reg("LogicalAndOrPrecedence",
                "true or false and true",
                IsBinary(TokenType::Or,
                         IsBoolean(true),
                         IsBinary(TokenType::And,
                                  IsBoolean(false),
                                  IsBoolean(true)
                         )
                ));

            reg("RelationalBeforeLogical",
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
                ));

            reg("LogicalMixedWithArithmeticAndEquality",
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
                ));

            reg("StringConcatenation",
                "\"hello\" + \" \" + \"world\"",
                IsBinary(TokenType::Plus,
                         IsBinary(TokenType::Plus,
                                  IsString("\"hello\""),
                                  IsString("\" \"")
                         ),
                         IsString("\"world\"")
                ));

            reg("PrefixPostfixInteraction",
                "not a.b[0]()",
                IsUnary(TokenType::Not,
                        IsCall(
                            IsBracket(
                                IsDot(IsIdentifier("a"), "b"),
                                IsNumber("0")
                            )
                        )
                ));

            reg("PostfixAndPrefixPrecedence",
                "-b() * 2",
                IsBinary(TokenType::Star,
                         IsUnary(TokenType::Minus,
                                 IsCall(IsIdentifier("b"))
                         ),
                         IsNumber("2")
                ));

            reg("BracketAccessPrecedence",
                "b[0] * 2",
                IsBinary(TokenType::Star,
                         IsBracket(
                             IsIdentifier("b"),
                             IsNumber("0")
                         ),
                         IsNumber("2")
                ));

            reg("PercentageInMathExpressions",
                "100 * 5% + 2%",
                IsBinary(TokenType::Plus,
                         IsBinary(TokenType::Star,
                                  IsNumber("100"),
                                  IsPercentage("5%")
                         ),
                         IsPercentage("2%")
                ));

            reg("ArithmeticNegation",
                "-5",
                IsUnary(TokenType::Minus,
                        IsNumber("5")
                ));

            reg("LogicalInversion",
                "not is_active",
                IsUnary(TokenType::Not,
                        IsIdentifier("is_active")
                ));

            reg("UnaryPrecedenceOverBinary",
                "-a * b",
                IsBinary(TokenType::Star,
                         IsUnary(TokenType::Minus,
                                 IsIdentifier("a")
                         ),
                         IsIdentifier("b")
                ));

            reg("NestedUnaryChaining1",
                "not not flag",
                IsUnary(TokenType::Not,
                        IsUnary(TokenType::Not,
                                IsIdentifier("flag")
                        )
                ));

            reg("NestedUnaryChaining2",
                "+ + flag",
                IsUnary(TokenType::Plus,
                        IsUnary(TokenType::Plus,
                                IsIdentifier("flag")
                        )
                ));

            reg("NestedUnaryChaining3",
                "- - flag",
                IsUnary(TokenType::Minus,
                        IsUnary(TokenType::Minus,
                                IsIdentifier("flag")
                        )
                ));

            reg("DeeplyNestedUnaryAndBinaryMath",
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
                ));

            reg("DeeplyNestedUnaryAndBinaryMath2",
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
                ));

            reg("ComplexMixedPrecedence",
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
                ));

            reg("OrExpr",
                "x or y",
                IsBinary(TokenType::Or,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("AndExpr",
                "x and y",
                IsBinary(TokenType::And,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("NotExpr",
                "not x",
                IsUnary(TokenType::Not,
                        IsIdentifier("x")
                ));

            reg("EqExpr",
                "x == y",
                IsBinary(TokenType::Equals,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("NeqExpr",
                "x != y",
                IsBinary(TokenType::NotEquals,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("GtExpr",
                "x > y",
                IsBinary(TokenType::Greater,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("LtExpr",
                "x < y",
                IsBinary(TokenType::Less,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("GteExpr",
                "x >= y",
                IsBinary(TokenType::GreaterEqual,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("LteExpr",
                "x <= y",
                IsBinary(TokenType::LessEqual,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("PowExpr",
                "x ^ y",
                IsBinary(TokenType::Caret,
                         IsIdentifier("x"),
                         IsIdentifier("y")
                ));

            reg("BoolGrouped",
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
                ));

            reg("ValidatesBooleanPrecedence",
                "x or y and not z",
                IsBinary(TokenType::Or,
                         IsIdentifier("x"),
                         IsBinary(TokenType::And,
                                  IsIdentifier("y"),
                                  IsUnary(TokenType::Not,
                                          IsIdentifier("z")
                                  )
                         )
                ));

            reg("ValidatesBooleanGrouping",
                "(x or y) and z",
                IsBinary(TokenType::And,
                         IsGrouping(
                             IsBinary(TokenType::Or,
                                      IsIdentifier("x"),
                                      IsIdentifier("y")
                             )
                         ),
                         IsIdentifier("z")
                ));

            return true;
        }();
    }
}
