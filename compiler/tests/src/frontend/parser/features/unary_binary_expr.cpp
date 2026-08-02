#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<ExprVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "StandardPrecedence",
                .code = "1 + 2 * 3 ^ 4",
                .verifier = IsBinary(
                    TokenType::Plus,
                    IsNumber("1"),
                    IsBinary(TokenType::Star,
                             IsNumber("2"),
                             IsBinary(TokenType::Caret,
                                      IsNumber("3"),
                                      IsNumber("4")
                             )
                    )
                )
            });

            reg({
                .name = "LeftAssociativitySubtraction",
                .code = "10 - 5 - 2",
                .verifier = IsBinary(
                    TokenType::Minus,
                    IsBinary(TokenType::Minus,
                             IsNumber("10"),
                             IsNumber("5")
                    ),
                    IsNumber("2")
                )
            });

            reg({
                .name = "LeftAssociativityDivision",
                .code = "20 / 5 / 2",
                .verifier = IsBinary(
                    TokenType::Slash,
                    IsBinary(TokenType::Slash,
                             IsNumber("20"),
                             IsNumber("5")
                    ),
                    IsNumber("2")
                )
            });

            reg({
                .name = "FactorLeftAssociativityMixed",
                .code = "a * b / c mod d",
                .verifier = IsBinary(
                    TokenType::Mod,
                    IsBinary(TokenType::Slash,
                             IsBinary(TokenType::Star,
                                      IsIdentifier("a"),
                                      IsIdentifier("b")
                             ),
                             IsIdentifier("c")
                    ),
                    IsIdentifier("d")
                )
            });

            reg({
                .name = "UnaryMinusPrecedence",
                .code = "-5 * 2",
                .verifier = IsBinary(
                    TokenType::Star,
                    IsUnary(TokenType::Minus,
                            IsNumber("5")
                    ),
                    IsNumber("2")
                )
            });

            reg({
                .name = "Modulo",
                .code = "10 mod 5",
                .verifier = IsBinary(TokenType::Mod, IsNumber("10"), IsNumber("5"))
            });

            reg({
                .name = "UnaryPlusBasic",
                .code = "+42",
                .verifier = IsUnary(TokenType::Plus, IsNumber("42"))
            });

            reg({
                .name = "UnaryBindsTighterThanPower",
                .code = "-2 ^ 2",
                .verifier = IsBinary(
                    TokenType::Caret,
                    IsUnary(TokenType::Minus,
                            IsNumber("2")
                    ),
                    IsNumber("2")
                )
            });

            reg({
                .name = "MixedRightAssociativityAndUnary",
                .code = "2 ^ -3 ^ 4",
                .verifier = IsBinary(
                    TokenType::Caret,
                    IsNumber("2"),
                    IsBinary(TokenType::Caret,
                             IsUnary(TokenType::Minus,
                                     IsNumber("3")
                             ),
                             IsNumber("4")
                    )
                )
            });

            reg({
                .name = "ConsecutiveDifferentUnary",
                .code = "not - + x",
                .verifier = IsUnary(
                    TokenType::Not,
                    IsUnary(TokenType::Minus,
                            IsUnary(TokenType::Plus,
                                    IsIdentifier("x")
                            )
                    )
                )
            });

            reg({
                .name = "ParenthesesOverride",
                .code = "(1 + 2) * 3",
                .verifier = IsBinary(
                    TokenType::Star,
                    IsGrouping(
                        IsBinary(TokenType::Plus,
                                 IsNumber("1"),
                                 IsNumber("2")
                        )
                    ),
                    IsNumber("3")
                )
            });

            reg({
                .name = "DeepParenthesesNesting",
                .code = "((1 + 2) * (3 - 4)) / 5",
                .verifier = IsBinary(
                    TokenType::Slash,
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
            });

            reg({
                .name = "GroupingRedundancy",
                .code = "(((x))) + 1",
                .verifier = IsBinary(
                    TokenType::Plus,
                    IsGrouping(
                        IsGrouping(
                            IsGrouping(
                                IsIdentifier("x")
                            )
                        )
                    ),
                    IsNumber("1")
                )
            });

            reg({
                .name = "RelationalPrecedence",
                .code = "1 + 2 > 3 * 4",
                .verifier = IsBinary(
                    TokenType::Greater,
                    IsBinary(TokenType::Plus,
                             IsNumber("1"),
                             IsNumber("2")
                    ),
                    IsBinary(TokenType::Star,
                             IsNumber("3"),
                             IsNumber("4")
                    )
                )
            });

            reg({
                .name = "EqualityPrecedence",
                .code = "x == y + 1",
                .verifier = IsBinary(
                    TokenType::Equals,
                    IsIdentifier("x"),
                    IsBinary(TokenType::Plus,
                             IsIdentifier("y"),
                             IsNumber("1")
                    )
                )
            });

            reg({
                .name = "ModuloPrecedence",
                .code = "10 mod 3 * 2",
                .verifier = IsBinary(
                    TokenType::Star,
                    IsBinary(TokenType::Mod,
                             IsNumber("10"),
                             IsNumber("3")
                    ),
                    IsNumber("2")
                )
            });

            reg({
                .name = "RightAssociativityPower",
                .code = "2 ^ 3 ^ 4",
                .verifier = IsBinary(
                    TokenType::Caret,
                    IsNumber("2"),
                    IsBinary(TokenType::Caret,
                             IsNumber("3"),
                             IsNumber("4")
                    )
                )
            });

            reg({
                .name = "RightAssociativityPowerMixed",
                .code = "2 ^ 3 ^ 4 * 5 + 6",
                .verifier = IsBinary(
                    TokenType::Plus,
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
            });

            reg({
                .name = "LogicalAndOrPrecedence",
                .code = "true or false and true",
                .verifier = IsBinary(
                    TokenType::Or,
                    IsBoolean(true),
                    IsBinary(TokenType::And,
                             IsBoolean(false),
                             IsBoolean(true)
                    )
                )
            });

            reg({
                .name = "RelationalBeforeLogical",
                .code = "x > y and z == w",
                .verifier = IsBinary(
                    TokenType::And,
                    IsBinary(TokenType::Greater,
                             IsIdentifier("x"),
                             IsIdentifier("y")
                    ),
                    IsBinary(TokenType::Equals,
                             IsIdentifier("z"),
                             IsIdentifier("w")
                    )
                )
            });

            reg({
                .name = "LogicalMixedWithArithmeticAndEquality",
                .code = "a + 1 == b and c * 2 != d or e",
                .verifier = IsBinary(
                    TokenType::Or,
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
            });

            reg({
                .name = "StringConcatenation",
                .code = R"("hello" + " " + "world")",
                .verifier = IsBinary(
                    TokenType::Plus,
                    IsBinary(TokenType::Plus,
                             IsString("\"hello\""),
                             IsString("\" \"")
                    ),
                    IsString("\"world\"")
                )
            });

            reg({
                .name = "PrefixPostfixInteraction",
                .code = "not a.b[0]()",
                .verifier = IsUnary(
                    TokenType::Not,
                    IsCall(
                        IsBracket(
                            IsDot(IsIdentifier("a"), "b"),
                            IsNumber("0")
                        )
                    )
                )
            });

            reg({
                .name = "PostfixAndPrefixPrecedence",
                .code = "-b() * 2",
                .verifier = IsBinary(
                    TokenType::Star,
                    IsUnary(TokenType::Minus,
                            IsCall(IsIdentifier("b"))
                    ),
                    IsNumber("2")
                )
            });

            reg({
                .name = "BracketAccessPrecedence",
                .code = "b[0] * 2",
                .verifier = IsBinary(
                    TokenType::Star,
                    IsBracket(
                        IsIdentifier("b"),
                        IsNumber("0")
                    ),
                    IsNumber("2")
                )
            });

            reg({
                .name = "PercentageInMathExpressions",
                .code = "100 * 5% + 2%",
                .verifier = IsBinary(
                    TokenType::Plus,
                    IsBinary(TokenType::Star,
                             IsNumber("100"),
                             IsPercentage("5%")
                    ),
                    IsPercentage("2%")
                )
            });

            reg({
                .name = "ArithmeticNegation",
                .code = "-5",
                .verifier = IsUnary(TokenType::Minus, IsNumber("5"))
            });

            reg({
                .name = "LogicalInversion",
                .code = "not is_active",
                .verifier = IsUnary(TokenType::Not, IsIdentifier("is_active"))
            });

            reg({
                .name = "UnaryPrecedenceOverBinary",
                .code = "-a * b",
                .verifier = IsBinary(
                    TokenType::Star,
                    IsUnary(TokenType::Minus,
                            IsIdentifier("a")
                    ),
                    IsIdentifier("b")
                )
            });

            reg({
                .name = "NestedUnaryChaining1",
                .code = "not not flag",
                .verifier = IsUnary(
                    TokenType::Not,
                    IsUnary(TokenType::Not,
                            IsIdentifier("flag")
                    )
                )
            });

            reg({
                .name = "NestedUnaryChaining2",
                .code = "+ + flag",
                .verifier = IsUnary(
                    TokenType::Plus,
                    IsUnary(TokenType::Plus,
                            IsIdentifier("flag")
                    )
                )
            });

            reg({
                .name = "NestedUnaryChaining3",
                .code = "- - flag",
                .verifier = IsUnary(
                    TokenType::Minus,
                    IsUnary(TokenType::Minus,
                            IsIdentifier("flag")
                    )
                )
            });

            reg({
                .name = "DeeplyNestedUnaryAndBinaryMath",
                .code = "-a * (b + c) - (not d) / e",
                .verifier = IsBinary(
                    TokenType::Minus,
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
                    IsBinary(
                        TokenType::Slash,
                        IsGrouping(
                            IsUnary(TokenType::Not,
                                    IsIdentifier("d")
                            )
                        ),
                        IsIdentifier("e")
                    )
                )
            });

            reg({
                .name = "DeeplyNestedUnaryAndBinaryMath2",
                .code = "-a * (b + c) - not d / e",
                .verifier = IsBinary(
                    TokenType::Minus,
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
            });

            reg({
                .name = "ComplexMixedPrecedence",
                .code = "-a.b ^ c[0] * d + e == f and not g or h",
                .verifier = IsBinary(
                    TokenType::Or,
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
            });

            reg({
                .name = "OrExpr",
                .code = "x or y",
                .verifier = IsBinary(TokenType::Or, IsIdentifier("x"), IsIdentifier("y"))
            });

            reg({
                .name = "AndExpr",
                .code = "x and y",
                .verifier = IsBinary(TokenType::And, IsIdentifier("x"), IsIdentifier("y"))
            });

            reg({
                .name = "NotExpr",
                .code = "not x",
                .verifier = IsUnary(TokenType::Not, IsIdentifier("x"))
            });

            reg({
                .name = "EqExpr",
                .code = "x == y",
                .verifier = IsBinary(TokenType::Equals, IsIdentifier("x"), IsIdentifier("y"))
            });

            reg({
                .name = "NeqExpr",
                .code = "x != y",
                .verifier = IsBinary(TokenType::NotEquals, IsIdentifier("x"), IsIdentifier("y")
                )
            });

            reg({
                .name = "GtExpr",
                .code = "x > y",
                .verifier = IsBinary(TokenType::Greater, IsIdentifier("x"), IsIdentifier("y")
                )
            });

            reg({
                .name = "LtExpr",
                .code = "x < y",
                .verifier = IsBinary(TokenType::Less, IsIdentifier("x"), IsIdentifier("y")
                )
            });

            reg({
                .name = "GteExpr",
                .code = "x >= y",
                .verifier = IsBinary(TokenType::GreaterEqual, IsIdentifier("x"), IsIdentifier("y")
                )
            });

            reg({
                .name = "LteExpr",
                .code = "x <= y",
                .verifier = IsBinary(TokenType::LessEqual, IsIdentifier("x"), IsIdentifier("y"))
            });

            reg({
                .name = "PowExpr",
                .code = "x ^ y",
                .verifier = IsBinary(TokenType::Caret, IsIdentifier("x"), IsIdentifier("y"))
            });

            reg({
                .name = "BoolGrouped",
                .code = "(a and b) or (c and not d)",
                .verifier = IsBinary(
                    TokenType::Or,
                    IsGrouping(
                        IsBinary(
                            TokenType::And,
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
            });

            reg({
                .name = "ValidatesBooleanPrecedence",
                .code = "x or y and not z",
                .verifier = IsBinary(
                    TokenType::Or,
                    IsIdentifier("x"),
                    IsBinary(TokenType::And,
                             IsIdentifier("y"),
                             IsUnary(TokenType::Not,
                                     IsIdentifier("z")
                             )
                    )
                )
            });

            reg({
                .name = "ValidatesBooleanGrouping",
                .code = "(x or y) and z",
                .verifier = IsBinary(
                    TokenType::And,
                    IsGrouping(
                        IsBinary(TokenType::Or,
                                 IsIdentifier("x"),
                                 IsIdentifier("y")
                        )
                    ),
                    IsIdentifier("z")
                )
            });

            reg({
                .name = "MultilineUnary",
                .code = "-\n2\n",
                .verifier = IsUnary(TokenType::Minus, IsNumber("2"))
            });

            return true;
        }();
    }
}
