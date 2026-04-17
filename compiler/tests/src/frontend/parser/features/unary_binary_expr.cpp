#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test {
    class UnaryBinarySuccessPathTest : public ParserTestBase,
                                       public testing::WithParamInterface<ValidParserTestCase> {
    };

    TEST_P(UnaryBinarySuccessPathTest, ParsesSuccessfully) {
        run_valid_parser_test(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        UnaryBinarySuccessPathTests,
        UnaryBinarySuccessPathTest,
        ::testing::Values(
            ValidParserTestCase{
            "standard_precedence", "let a = 1 + 2 * 3 ^ 4",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Plus,
                    IsNumber("1"),
                    IsBinary(TokenType::Star,
                        IsNumber("2"),
                        IsBinary(TokenType::Caret,
                            IsNumber("3"),
                            IsNumber("4")
                        )
                    )
                )) } }
            },
            ValidParserTestCase{
            "left_associativity_subtraction", "let a = 10 - 5 - 2",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Minus,
                    IsBinary(TokenType::Minus,
                        IsNumber("10"),
                        IsNumber("5")
                    ),
                    IsNumber("2")
                )) } }
            },
            ValidParserTestCase{
            "left_associativity_division", "let a = 20 / 5 / 2",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Slash,
                    IsBinary(TokenType::Slash,
                        IsNumber("20"),
                        IsNumber("5")
                    ),
                    IsNumber("2")
                )) } }
            },
            ValidParserTestCase{
            "unary_minus_precedence", "let a = -5 * 2",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Star,
                    IsUnary(TokenType::Minus,
                        IsNumber("5")
                    ),
                    IsNumber("2")
                )) } }
            },
            ValidParserTestCase{
            "parentheses_override", "let a = (1 + 2) * 3",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Star,
                    IsGrouping(
                        IsBinary(TokenType::Plus,
                            IsNumber("1"),
                            IsNumber("2")
                        )
                    ),
                    IsNumber("3")
                )) } }
            },
            ValidParserTestCase{
            "deep_parentheses_nesting", "let a = ((1 + 2) * (3 - 4)) / 5",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
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
                )) } }
            },
            ValidParserTestCase{
            "relational_precedence", "let a = 1 + 2 > 3 * 4",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Greater,
                    IsBinary(TokenType::Plus,
                        IsNumber("1"),
                        IsNumber("2")
                    ),
                    IsBinary(TokenType::Star,
                        IsNumber("3"),
                        IsNumber("4")
                    )
                )) } }
            },
            ValidParserTestCase{
            "equality_precedence", "let a = x == y + 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Equals,
                    IsIdentifier("x"),
                    IsBinary(TokenType::Plus,
                        IsIdentifier("y"),
                        IsNumber("1")
                    )
                )) } }
            },
            ValidParserTestCase{
            "modulo_precedence", "let a = 10 mod 3 * 2",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Star,
                    IsBinary(TokenType::Mod,
                        IsNumber("10"),
                        IsNumber("3")
                    ),
                    IsNumber("2")
                )) } }
            },
            ValidParserTestCase{
            "right_associativity_power", "let a = 2 ^ 3 ^ 4",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Caret,
                    IsNumber("2"),
                    IsBinary(TokenType::Caret,
                        IsNumber("3"),
                        IsNumber("4")
                    )
                )) } }
            },
            ValidParserTestCase{
            "logical_and_or_precedence", "let a = true or false and true",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Or,
                    IsBoolean(true),
                    IsBinary(TokenType::And,
                        IsBoolean(false),
                        IsBoolean(true)
                    )
                )) } }
            },
            ValidParserTestCase{
            "relational_before_logical", "let a = x > y and z == w",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::And,
                    IsBinary(TokenType::Greater,
                        IsIdentifier("x"),
                        IsIdentifier("y")
                    ),
                    IsBinary(TokenType::Equals,
                        IsIdentifier("z"),
                        IsIdentifier("w")
                    )
                )) } }
            },
            ValidParserTestCase{
            "postfix_and_prefix_precedence", "let a = -b() * 2",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Star,
                    IsUnary(TokenType::Minus,
                        IsCall(IsIdentifier("b"))
                    ),
                    IsNumber("2")
                )) } }
            },
            ValidParserTestCase{
            "bracket_access_precedence", "let a = b[0] * 2",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Star,
                    IsBracket(IsIdentifier("b"), IsNumber("0")),
                    IsNumber("2")
                )) } }
            },
            ValidParserTestCase{
            "percentage_in_math_expressions", "let total = 100 * 5% + 2%",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"total"}},
                IsBinary(TokenType::Plus,
                    IsBinary(TokenType::Star,
                        IsNumber("100"),
                        IsPercentage("5%")
                    ),
                    IsPercentage("2%")
                )) } }
            },
            ValidParserTestCase{
            "arithmetic_negation", "let a = -5",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsUnary(TokenType::Minus,
                    IsNumber("5")
                )) } }
            },
            ValidParserTestCase{
            "logical_inversion", "let a = not is_active",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsUnary(TokenType::Not,
                    IsIdentifier("is_active")
                )) } }
            },
            ValidParserTestCase{
            "unary_precedence_over_binary", "let a = -a * b",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Star,
                    IsUnary(TokenType::Minus,
                        IsIdentifier("a")
                    ),
                    IsIdentifier("b")
                )) } }
            },
            ValidParserTestCase{
            "nested_unary_chaining_1", "let a = not not flag",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsUnary(TokenType::Not,
                    IsUnary(TokenType::Not,
                        IsIdentifier("flag")
                    )
                )) } }
            },
            ValidParserTestCase{
            "nested_unary_chaining_2", "let a = + + flag",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsUnary(TokenType::Plus,
                    IsUnary(TokenType::Plus,
                        IsIdentifier("flag")
                    )
                )) } }
            },
            ValidParserTestCase{
            "nested_unary_chaining_3", "let a = - - flag",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsUnary(TokenType::Minus,
                    IsUnary(TokenType::Minus,
                        IsIdentifier("flag")
                    )
                )) } }
            },
            ValidParserTestCase{
            "deeply_nested_unary_and_binary_math", "let a = -a * (b + c) - (not d) / e",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
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
                )) } }
            },
            ValidParserTestCase{
            "deeply_nested_unary_and_binary_math_2", "let a = -a * (b + c) - not d / e",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
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
                )) } }
            },
            ValidParserTestCase{
            "or_expr", "let a = x or y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Or, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "and_expr", "let a = x and y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::And, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "not_expr", "let a = not x",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsUnary(TokenType::Not, IsIdentifier("x"))
            )} }
            },
            ValidParserTestCase{
            "eq_expr", "let a = x == y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Equals, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "neq_expr", "let a = x != y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::NotEquals, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "gt_expr", "let a = x > y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Greater, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "lt_expr", "let a = x < y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Less, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "gte_expr", "let a = x >= y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::GreaterEqual, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "lte_expr", "let a = x <= y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::LessEqual, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "pow_expr", "let a = x ^ y",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Caret, IsIdentifier("x"), IsIdentifier("y"))
            )} }
            },
            ValidParserTestCase{
            "bool_grouped", "let a = (a and b) or (c and not d)",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}},
                IsBinary(TokenType::Or,
                    IsGrouping(
                        IsBinary(TokenType::And, IsIdentifier("a"), IsIdentifier("b"))
                    ),
                    IsGrouping(
                        IsBinary(TokenType::And,
                            IsIdentifier("c"),
                            IsUnary(TokenType::Not, IsIdentifier("d"))
                        )
                    )
                )
            )} }
            }
        ),
        [](const testing::TestParamInfo<ValidParserTestCase>& info) {
        return info.param.test_name;
        }
    );
}
