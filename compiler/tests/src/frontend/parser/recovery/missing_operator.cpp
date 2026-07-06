#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class MissingOperatorTestBase : public ParserTestBase
    {
    };

    using E = ParserErrorCode;

    TEST_F(MissingOperatorTestBase, vec_access_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "let a = vec[1 + 2 3]",
            {{E::MissingOperator, 1, 19, 1, 20}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"a"}
                        },
                        IsBracket(
                            IsIdentifier("vec"),
                            IsBinary(
                                TokenType::Plus,
                                IsNumber("1"),
                                IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3"))))
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, vec_access_missing_operator_3)
    {
        ExpectParseErrorsWithRecovery(
            "let a = vec[1 + (2 3)]",
            {{E::MissingOperator, 1, 20, 1, 21}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"a"}
                        },
                        IsBracket(
                            IsIdentifier("vec"),
                            IsBinary(TokenType::Plus,
                                     IsNumber("1"),
                                     IsGrouping(IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3"))))
                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, vec_access_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "let a = vec[1 (2 + 3)]",
            {{E::MissingOperator, 1, 15, 1, 16}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"a"}
                        },
                        IsBracket(IsIdentifier("vec"), IsCall(IsNumber("1")))
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, vec_access_missing_operator_5)
    {
        ExpectParseErrorsWithRecovery(
            "let a = vec[1 + a() b()]",
            {{E::MissingOperator, 1, 21, 1, 22}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"a"}
                        },
                        IsBracket(IsIdentifier("vec"),
                                  IsBinary(TokenType::Plus,
                                           IsNumber("1"),
                                           IsBinary(TokenType::Error,
                                                    IsCall(IsIdentifier("a")),
                                                    IsCall(IsIdentifier("b")
                                                    )
                                           )
                                  )
                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, tensor_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "let a = [1 + 2 3]",
            {{E::MissingOperator, 1, 16, 1, 17}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"a"}
                        },
                        IsTensor({
                            IsBinary(TokenType::Plus, IsNumber("1"), IsNumber("2")),
                            IsNumber("3")
                        })
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, tensor_missing_operator_3)
    {
        ExpectParseErrorsWithRecovery(
            "let a = [1 + (2 3)]",
            {{E::MissingOperator, 1, 17, 1, 18}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"a"}
                        },
                        IsTensor({
                            IsBinary(TokenType::Plus,
                                     IsNumber("1"),
                                     IsGrouping(IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3")))
                            )
                        })
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, tensor_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "let a = [1 (2 + 3)]",
            {{E::MissingOperator, 1, 12, 1, 13}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"a"}
                        },
                        IsTensor({IsCall(IsNumber("1"))})
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, tensor_missing_operator_5)
    {
        ExpectParseErrorsWithRecovery(
            "let a = [1 + a() b()]",
            {{E::MissingOperator, 1, 18, 1, 19}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            {"a"}
                        },
                        IsTensor({
                            IsBinary(TokenType::Plus, IsNumber("1"), IsCall(IsIdentifier("a"))),
                            IsCall(IsIdentifier("b"))
                        })
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, dir_missing_operator_1)
    {
        ExpectParseErrorsWithRecovery(
            "#iterations = 1000 1",
            {{E::MissingOperator, 1, 20, 1, 21}},
            ProgramSpec{
                .directives = {
                    IsDirective("iterations", IsBinary(TokenType::Error, IsNumber("1000"), IsNumber("1")))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, dir_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "#iterations = 1000 (1 + 2)",
            {{E::MissingOperator, 1, 20, 1, 21}},
            ProgramSpec{
                .directives = {
                    IsDirective("iterations", IsCall(IsNumber("1000")))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, dir_missing_operator_5)
    {
        ExpectParseErrorsWithRecovery(
            "#iterations = 1000 a() + b()",
            {{E::MissingOperator, 1, 20, 1, 21}},
            ProgramSpec{
                .directives = {
                    IsDirective("iterations",
                                IsBinary(TokenType::Plus,
                                         IsBinary(TokenType::Error, IsNumber("1000"), IsCall(IsIdentifier("a"))),
                                         IsCall(IsIdentifier("b"))
                                )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, dict_key_missing_operator_no_comma)
    {
        ExpectParseErrorsWithRecovery(
            "let a = { market_size: 13_624 / 11%   4 }",
            {{E::MissingOperator, 1, 39, 1, 40}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"a"}},
                        IsDict({
                            {
                                "market_size", {},
                                IsBinary(TokenType::Slash, IsNumber("13_624"),
                                         IsBinary(TokenType::Error, IsPercentage("11%"), IsNumber("4")))
                            }
                        })
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, dict_key_missing_operator_comma)
    {
        ExpectParseErrorsWithRecovery(
            "let a = { market_size: 13_624 / 11%   4, }",
            {{E::MissingOperator, 1, 39, 1, 40}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"a"}},
                        IsDict({
                            {
                                "market_size", {},
                                IsBinary(TokenType::Slash, IsNumber("13_624"),
                                         IsBinary(TokenType::Error, IsPercentage("11%"), IsNumber("4")))
                            }
                        })
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, enum_missing_operator_1)
    {
        ExpectParseErrorsWithRecovery(
            "enum Test : int { A = a b, B = 2 }",
            {{E::MissingOperator, 1, 25, 1, 26}},
            ProgramSpec{
                .enums = {
                    IsEnumDef("Test", {}, IsType("int"), {
                                  {"A", {}, IsBinary(TokenType::Error, IsIdentifier("a"), IsIdentifier("b"))},
                                  {"B", {}, IsNumber("2")}
                              })
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, enum_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "enum Test : int { A = (a b), B = 2 }",
            {{E::MissingOperator, 1, 26, 1, 27}},
            ProgramSpec{
                .enums = {
                    IsEnumDef("Test", {}, IsType("int"), {
                                  {
                                      "A", {},
                                      IsGrouping(IsBinary(TokenType::Error, IsIdentifier("a"), IsIdentifier("b")))
                                  },
                                  {"B", {}, IsNumber("2")}
                              })
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, enum_missing_operator_3)
    {
        ExpectParseErrorsWithRecovery(
            "enum Test : int { A = 1 (a + b), B = 2 }",
            {{E::MissingOperator, 1, 25, 1, 26}},
            ProgramSpec{
                .enums = {
                    IsEnumDef("Test", {}, IsType("int"), {
                                  {"A", {}, IsCall(IsNumber("1"))},
                                  {"B", {}, IsNumber("2")}
                              })
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, expr_missing_operator_1)
    {
        ExpectParseErrorsWithRecovery(
            "let result = a + b c",
            {{E::MissingOperator, 1, 20, 1, 21}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"result"}},
                                 IsBinary(TokenType::Plus,
                                          IsIdentifier("a"),
                                          IsBinary(TokenType::Error, IsIdentifier("b"), IsIdentifier("c"))
                                 )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, expr_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "let result = a + b (1 + 2)",
            {{E::MissingOperator, 1, 20, 1, 21}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"result"}},
                                 IsBinary(TokenType::Plus, IsIdentifier("a"), IsCall(IsIdentifier("b")))
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, expr_missing_operator_3)
    {
        ExpectParseErrorsWithRecovery(
            "let result = a + b model.a",
            {{E::MissingOperator, 1, 20, 1, 25}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"result"}},
                                 IsBinary(TokenType::Plus,
                                          IsIdentifier("a"),
                                          IsBinary(TokenType::Error,
                                                   IsIdentifier("b"),
                                                   IsDot(IsIdentifier("model"), "a")
                                          )
                                 )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, expr_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "let result = a + b vec[0]",
            {{E::MissingOperator, 1, 20, 1, 23}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"result"}},
                                 IsBinary(TokenType::Plus,
                                          IsIdentifier("a"),
                                          IsBinary(TokenType::Error,
                                                   IsIdentifier("b"),
                                                   IsBracket(IsIdentifier("vec"), IsNumber("0")))))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, expr_missing_operator_5)
    {
        ExpectParseErrorsWithRecovery(
            "let result = a + b {}",
            {{E::MissingOperator, 1, 20, 1, 21}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"result"}},
                                 IsBinary(TokenType::Plus,
                                          IsIdentifier("a"),
                                          IsBinary(TokenType::Error, IsIdentifier("b"), IsDict({}))
                                 )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, expr_missing_operator_6)
    {
        ExpectParseErrorsWithRecovery(
            "let result = a  b[]",
            {
                {E::MissingOperator, 1, 17, 1, 18},
                {E::EmptyBracketAccess, 1, 18, 1, 19}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"result"}},
                                 IsBinary(TokenType::Error,
                                          IsIdentifier("a"),
                                          IsBracket(IsIdentifier("b"), IsNull())
                                 )

                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, expr_missing_operator_6_a)
    {
        ExpectParseErrorsWithRecovery(
            "let result = a - b[]",
            {{E::EmptyBracketAccess, 1, 19, 1, 20}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"result"}},
                                 IsBinary(TokenType::Minus,
                                          IsIdentifier("a"),
                                          IsBracket(IsIdentifier("b"), IsNull())
                                 )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, expr_missing_operator_7)
    {
        ExpectParseErrorsWithRecovery(
            "let result = a + b (1, 2)",
            {{E::MissingOperator, 1, 20, 1, 21}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"result"}},
                                 IsBinary(
                                     TokenType::Plus,
                                     IsIdentifier("a"),
                                     IsCall(IsIdentifier("b"))
                                 )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, mod_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "@export(a: 1 + 2 3) let x = 1",
            {{E::MissingOperator, 1, 18, 1, 19}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec(
                                {
                                    ModifierSpec{
                                        "export",
                                        {
                                            ArgSpec{
                                                "a",
                                                IsBinary(
                                                    TokenType::Plus,
                                                    IsNumber("1"),
                                                    IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3")))
                                            }
                                        }
                                    }
                                }, "x")
                        },
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, mod_missing_operator_3)
    {
        ExpectParseErrorsWithRecovery(
            "@export(a: 1 + (2 3)) let x = 1",
            {{E::MissingOperator, 1, 19, 1, 20}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec(
                                {
                                    ModifierSpec{
                                        "export",
                                        {
                                            ArgSpec{
                                                "a",
                                                IsBinary(TokenType::Plus,
                                                         IsNumber("1"),
                                                         IsGrouping(
                                                             IsBinary(
                                                                 TokenType::Error, IsNumber("2"), IsNumber("3"))
                                                         )
                                                )
                                            }
                                        }
                                    }
                                }, "x")
                        },
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, mod_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "@export(a: 1 (2 + 3)) let x = 1",
            {{E::MissingOperator, 1, 14, 1, 15}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {AssignmentTargetSpec({ModifierSpec{"export", {ArgSpec{"a", IsCall(IsNumber("1"))}}}}, "x")},
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, mod_missing_operator_5)
    {
        ExpectParseErrorsWithRecovery(
            "@export(a: 1 a() + b()) let x = 1",
            {{E::MissingOperator, 1, 14, 1, 15}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {
                            AssignmentTargetSpec(
                                {
                                    ModifierSpec{
                                        "export",
                                        {
                                            ArgSpec{
                                                "a",
                                                IsBinary(
                                                    TokenType::Plus,
                                                    IsBinary(TokenType::Error, IsNumber("1"),
                                                             IsCall(IsIdentifier("a"))),
                                                    IsCall(IsIdentifier("b")))
                                            }
                                        }
                                    }
                                }, "x")
                        },
                        IsNumber("1")
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, reassign_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "a = 1 + 2 3",
            {{E::MissingOperator, 1, 11, 1, 12}},
            ProgramSpec{
                .execution_steps = {
                    IsReassignment(
                        IsIdentifier("a"),
                        IsBinary(
                            TokenType::Plus,
                            IsNumber("1"),
                            IsBinary(TokenType::Error, IsNumber("2"), IsNumber("3"))
                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, reassign_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "a = 1  (2 + 3)",
            {{E::MissingOperator, 1, 8, 1, 9}},
            ProgramSpec{
                .execution_steps = {
                    IsReassignment(IsIdentifier("a"), IsCall(IsNumber("1")))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, reassign_missing_operator_5)
    {
        ExpectParseErrorsWithRecovery(
            "a = 1 + a() b()",
            {{E::MissingOperator, 1, 13, 1, 14}},
            ProgramSpec{
                .execution_steps = {
                    IsReassignment(IsIdentifier("a"),
                                   IsBinary(
                                       TokenType::Plus,
                                       IsNumber("1"),
                                       IsBinary(TokenType::Error,
                                                IsCall(IsIdentifier("a")),
                                                IsCall(IsIdentifier("b"))
                                       )
                                   )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, reassign_missing_operator_6)
    {
        ExpectParseErrorsWithRecovery(
            "a = 1 + a[0] b[1:2]",
            {{E::MissingOperator, 1, 14, 1, 15}},
            ProgramSpec{
                .execution_steps = {
                    IsReassignment(IsIdentifier("a"),
                                   IsBinary(TokenType::Plus, IsNumber("1"),
                                            IsBinary(TokenType::Error, IsBracket(IsIdentifier("a"), IsNumber("0")),
                                                     IsBracket(IsIdentifier("b"),
                                                               IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))
                                                     )
                                            )
                                   )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, reassign_missing_operator_7)
    {
        ExpectParseErrorsWithRecovery(
            "a = 1 + a[0] + b[1:2] a.b",
            {{E::MissingOperator, 1, 23, 1, 24}},
            ProgramSpec{
                .execution_steps = {
                    IsReassignment(
                        IsIdentifier("a"),
                        IsBinary(TokenType::Plus,
                                 IsBinary(TokenType::Plus,
                                          IsNumber("1"),
                                          IsBracket(IsIdentifier("a"), IsNumber("0"))
                                 ),
                                 IsBinary(TokenType::Error,
                                          IsBracket(
                                              IsIdentifier("b"),
                                              IsBinary(TokenType::Colon, IsNumber("1"), IsNumber("2"))
                                          ),
                                          IsDot(IsIdentifier("a"), "b"))
                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, switch_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "let a = switch (res) { case UP -> 1 (2 + 3) }",
            {{E::MissingOperator, 1, 37, 1, 38}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"a"}},
                        IsSwitch(IsIdentifier("res"), {
                                     SwitchCaseSpec{{"UP"}, IsCall(IsNumber("1"))}
                                 }))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, switch_missing_operator_3)
    {
        ExpectParseErrorsWithRecovery(
            "let a = switch (res) { case UP -> 1 + a() (2 + 3) }",
            {{E::MissingOperator, 1, 43, 1, 44}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"a"}},
                        IsSwitch(IsIdentifier("res"), {
                                     SwitchCaseSpec{
                                         {"UP"},
                                         IsBinary(TokenType::Plus, IsNumber("1"),
                                                  IsCall(IsCall(IsIdentifier("a"))))
                                     }
                                 }))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, switch_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "let a = switch (res) { case UP -> 1 + a() b() }",
            {
                {E::MissingOperator, 1, 43, 1, 44}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"a"}},
                        IsSwitch(IsIdentifier("res"), {
                                     SwitchCaseSpec{
                                         {"UP"},
                                         IsBinary(TokenType::Plus,
                                                  IsNumber("1"),
                                                  IsBinary(TokenType::Error,
                                                           IsCall(IsIdentifier("a")),
                                                           IsCall(IsIdentifier("b"))
                                                  )
                                         )
                                     }
                                 }))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, tuple_missing_operator_1)
    {
        ExpectParseErrorsWithRecovery(
            "let x = (a b)",
            {{E::MissingOperator, 1, 12, 1, 13}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"x"}}, IsGrouping(IsBinary(TokenType::Error, IsIdentifier("a"), IsIdentifier("b"))))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, tuple_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "let x = (a, b c)",
            {{E::MissingOperator, 1, 15, 1, 16}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"x"}}, IsTuple({IsIdentifier("a"), IsIdentifier("b"), IsIdentifier("c")}))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, tuple_missing_operator_3)
    {
        ExpectParseErrorsWithRecovery(
            "let x = (a, b (c + d))",
            {{E::MissingOperator, 1, 15, 1, 16}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"x"}}, IsTuple({IsIdentifier("a"), IsCall(IsIdentifier("b"))}))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, tuple_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "let x = (a, b + (c  d))",
            {{E::MissingOperator, 1, 21, 1, 22}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"x"}},
                        IsTuple({
                            IsIdentifier("a"),
                            IsBinary(TokenType::Plus, IsIdentifier("b"),
                                     IsGrouping(IsBinary(TokenType::Error, IsIdentifier("c"),
                                                         IsIdentifier("d"))))
                        }))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_1)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a  (b - c)",
            {{E::MissingOperator, 1, 14, 1, 15}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"res"}}, IsCall(IsIdentifier("a")))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_2)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a + (b  c)",
            {{E::MissingOperator, 1, 19, 1, 20}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"res"}},
                        IsBinary(TokenType::Plus,
                                 IsIdentifier("a"),
                                 IsGrouping(IsBinary(TokenType::Error, IsIdentifier("b"), IsIdentifier("c")))

                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_3)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a[1]  (b - c)",
            {{E::MissingOperator, 1, 17, 1, 18}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"res"}}, IsCall(IsBracket(IsIdentifier("a"), IsNumber("1"))))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_4)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a[1] / (b  c)",
            {{E::MissingOperator, 1, 22, 1, 23}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"res"}},
                        IsBinary(
                            TokenType::Slash,
                            IsBracket(IsIdentifier("a"), IsNumber("1")),
                            IsGrouping(IsBinary(TokenType::Error, IsIdentifier("b"), IsIdentifier("c")))
                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_5)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a[1] / (1  c)",
            {{E::MissingOperator, 1, 22, 1, 23}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"res"}},
                        IsBinary(
                            TokenType::Slash,
                            IsBracket(IsIdentifier("a"), IsNumber("1")),
                            IsGrouping(IsBinary(TokenType::Error, IsNumber("1"), IsIdentifier("c")))
                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_6)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a[1] (1 + c)",
            {{E::MissingOperator, 1, 16, 1, 17}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"res"}}, IsCall(IsBracket(IsIdentifier("a"), IsNumber("1"))))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_7)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a[1] + (b.a  c)",
            {{E::MissingOperator, 1, 24, 1, 25}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"res"}},
                        IsBinary(
                            TokenType::Plus,
                            IsBracket(IsIdentifier("a"), IsNumber("1")),
                            IsGrouping(IsBinary(TokenType::Error, IsDot(IsIdentifier("b"), "a"), IsIdentifier("c")))
                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_8)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a[1] + (b.a  c[3].b)",
            {{E::MissingOperator, 1, 24, 1, 25}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment(
                        {{"res"}},
                        IsBinary(
                            TokenType::Plus,
                            IsBracket(IsIdentifier("a"), IsNumber("1")),
                            IsGrouping(
                                IsBinary(
                                    TokenType::Error,
                                    IsDot(IsIdentifier("b"), "a"),
                                    IsDot(IsBracket(IsIdentifier("c"), IsNumber("3")), "b")
                                )
                            )
                        )
                    )
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_9)
    {
        ExpectParseErrorsWithRecovery(
            "func test() -> scalar {\n"
            "return a + a.key (1 + 2)\n"
            "}",
            {{E::MissingOperator, 2, 18, 2, 19}},
            ProgramSpec{
                .functions = {
                    IsFunctionDef("test", {}, {}, {IsType("scalar")}, {
                                      IsReturn({
                                          IsBinary(TokenType::Plus, IsIdentifier("a"),
                                                   IsCall(IsDot(IsIdentifier("a"), "key")))
                                      })
                                  })
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_10)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a ([1, 2])",
            {{E::MissingOperator, 1, 13, 1, 14}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"res"}}, IsCall(IsIdentifier("a")))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_11)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a ({1, 2})",
            {{E::MissingOperator, 1, 13, 1, 14}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"res"}}, IsCall(IsIdentifier("a")))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_12)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a ([[1, 2], [3, 4])",
            {{E::MissingOperator, 1, 13, 1, 14}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"res"}}, IsCall(IsIdentifier("a")))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_13)
    {
        ExpectParseErrorsWithRecovery(
            "let res = a (-5)",
            {{E::MissingOperator, 1, 13, 1, 14}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"res"}}, IsCall(IsIdentifier("a")))
                }
            }
        );
    }

    TEST_F(MissingOperatorTestBase, sync_port_missing_operator_14)
    {
        ExpectParseErrorsWithRecovery(
            "let a = switch (s) { case LOW -> 1  (3 + 3) case HIGH -> 3 }",
            {{E::MissingOperator, 1, 37, 1, 38}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"a"}},
                                 IsSwitch(IsIdentifier("s"), {
                                              SwitchCaseSpec{{"LOW"}, IsCall(IsNumber("1"))},
                                              SwitchCaseSpec{{"HIGH"}, IsNumber("3")}
                                          })
                    )
                }
            }
        );
    }
}
