#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstStmtSpanTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstStmtSpanTest, SimpleAssignment)
    {
        std::string code = "let x = 42";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({
                    AssignmentTargetSpec{.name = "x"}
                        .with_name_span(1, 5, 1, 6)
                        .with_span(1, 5, 1, 6)
                },
                IsNumber("42").with_span(1, 9, 1, 11)
                ).with_span(1, 1, 1, 11)
            }
        });
    }

    TEST_F(AstStmtSpanTest, AssignmentWithOuterModifier)
    {
        std::string code =
                "@persist\n"
                "let total = 100";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({
                    AssignmentTargetSpec{
                        .modifiers = {
                            ModifierSpec{.name = "persist"}.with_name_span(1, 2, 1, 9).with_span(1, 1, 1, 9)
                        },
                        .name = "total"
                    }.with_name_span(2, 5, 2, 10)
                     .with_span(2, 5, 2, 10)
                },
                IsNumber("100").with_span(2, 13, 2, 16)
                ).with_span(1, 1, 2, 16)
            }
        });
    }

    TEST_F(AstStmtSpanTest, AssignmentWithMultiTargetAndInnerModifiers)
    {
        std::string code = "let @readonly x: int, @atomic y: string = 10";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({
                    AssignmentTargetSpec{
                        .modifiers = {
                            ModifierSpec{.name = "readonly"}.with_name_span(1, 6, 1, 14).with_span(1, 5, 1, 14)
                        },
                        .name = "x",
                        .type_v = IsType("int").with_name_span(1, 18, 1, 21).with_span(1, 18, 1, 21)
                    }.with_name_span(1, 15, 1, 16)
                     .with_span(1, 5, 1, 21),
                    AssignmentTargetSpec{
                        .modifiers = {
                            ModifierSpec{.name = "atomic"}.with_name_span(1, 24, 1, 30).with_span(1, 23, 1, 30)
                        },
                        .name = "y",
                        .type_v = IsType("string").with_name_span(1, 34, 1, 40).with_span(1, 34, 1, 40)
                    }.with_name_span(1, 31, 1, 32)
                     .with_span(1, 23, 1, 40)
                },
                IsNumber("10").with_span(1, 43, 1, 45)
                ).with_span(1, 1, 1, 45)
            }
        });
    }

    TEST_F(AstStmtSpanTest, AssignmentWithOuterAndInnerModifiers)
    {
        std::string code =
                "@logged\n"
                "let @sec token = \"abc\"";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({
                    AssignmentTargetSpec{
                        .modifiers = {
                            ModifierSpec{.name = "logged"}.with_name_span(1, 2, 1, 8).with_span(1, 1, 1, 8),
                            ModifierSpec{.name = "sec"}.with_name_span(2, 6, 2, 9).with_span(2, 5, 2, 9)
                        },
                        .name = "token"
                    }.with_name_span(2, 10, 2, 15)
                     .with_span(2, 5, 2, 15)
                },
                IsString("\"abc\"").with_span(2, 18, 2, 23)
                ).with_span(1, 1, 2, 23)
            }
        });
    }

    TEST_F(AstStmtSpanTest, SimpleReassignment)
    {
        std::string code = "counter = 10";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsReassignment(
                    IsIdentifier("counter").with_span(1, 1, 1, 8),
                    IsNumber("10").with_span(1, 11, 1, 13)
                ).with_span(1, 1, 1, 13)
            }
        });
    }

    TEST_F(AstStmtSpanTest, DotAndBracketReassignments)
    {
        std::string code =
                "user.name = \"Alice\"\n"
                "scores[0] = 99";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsReassignment(
                    IsDot(
                        IsIdentifier("user").with_span(1, 1, 1, 5),
                        "name"
                    ).with_name_span(1, 6, 1, 10)
                     .with_span(1, 1, 1, 10),
                    IsString("\"Alice\"").with_span(1, 13, 1, 20)
                ).with_span(1, 1, 1, 20),
                IsReassignment(
                    IsBracket(
                        IsIdentifier("scores").with_span(2, 1, 2, 7),
                        IsNumber("0").with_span(2, 8, 2, 9)
                    ).with_span(2, 1, 2, 10),
                    IsNumber("99").with_span(2, 13, 2, 15)
                ).with_span(2, 1, 2, 15)
            }
        });
    }

    TEST_F(AstStmtSpanTest, ReturnStatementSimple)
    {
        std::string code =
                "func getVal() -> int {\n"
                "    return 42\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("getVal",
                    /*modifiers=*/{},
                    /*params=*/{},
                    /*returns=*/{IsType("int").with_name_span(1, 18, 1, 21).with_span(1, 18, 1, 21)},
                    /*body=*/{
                        IsReturn(IsNumber("42").with_span(2, 12, 2, 14))
                            .with_span(2, 5, 2, 14)
                    }
                ).with_name_span(1, 6, 1, 12)
                 .with_span(1, 1, 3, 2)
            }
        });
    }

    TEST_F(AstStmtSpanTest, ReturnStatementWithModifier)
    {
        std::string code =
                "func getRecursive(n: int) -> int {\n"
                "    @tailrec\n"
                "    return compute(val: n - 1)\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("getRecursive",
                    /*modifiers=*/{},
                    /*params=*/{
                        ParamSpec{
                            .name = "n",
                            .type_v = IsType("int").with_name_span(1, 22, 1, 25).with_span(1, 22, 1, 25)
                        }.with_name_span(1, 19, 1, 20)
                         .with_span(1, 19, 1, 25)
                    },
                    /*returns=*/{IsType("int").with_name_span(1, 30, 1, 33).with_span(1, 30, 1, 33)},
                    /*body=*/{
                        IsReturn(
                            IsCall(
                                IsIdentifier("compute").with_span(3, 12, 3, 19),
                                ArgSpec{
                                    .label = "val",
                                    .value_v = IsBinary(TokenType::Minus,
                                        IsIdentifier("n").with_span(3, 25, 3, 26),
                                        IsNumber("1").with_span(3, 29, 3, 30)
                                    ).with_span(3, 25, 3, 30)
                                }.with_name_span(3, 20, 3, 23)
                                 .with_span(3, 20, 3, 30)
                            ).with_span(3, 12, 3, 31)
                        ).with_span(2, 5, 3, 31)
                    }
                ).with_name_span(1, 6, 1, 18)
                 .with_span(1, 1, 4, 2)
            }
        });
    }

    TEST_F(AstStmtSpanTest, ReturnStatementMultipleValues)
    {
        std::string code =
                "func coords() -> int, int {\n"
                "    return x, y\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .functions = {
                IsFunctionDef("coords",
                    /*modifiers=*/{},
                    /*params=*/{},
                    /*returns=*/{
                        IsType("int").with_name_span(1, 18, 1, 21).with_span(1, 18, 1, 21),
                        IsType("int").with_name_span(1, 23, 1, 26).with_span(1, 23, 1, 26)
                    },
                    /*body=*/{
                        IsReturn(
                            IsIdentifier("x").with_span(2, 12, 2, 13),
                            IsIdentifier("y").with_span(2, 15, 2, 16)
                        ).with_span(2, 5, 2, 16)
                    }
                ).with_name_span(1, 6, 1, 12)
                 .with_span(1, 1, 3, 2)
            }
        });
    }

    TEST_F(AstStmtSpanTest, ExpressionStatementCall)
    {
        std::string code = "print(message: \"hello\")";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsExprStmt(
                    IsCall(
                        IsIdentifier("print").with_span(1, 1, 1, 6),
                        ArgSpec{
                            .label = "message",
                            .value_v = IsString("\"hello\"").with_span(1, 16, 1, 23)
                        }.with_name_span(1, 7, 1, 14)
                         .with_span(1, 7, 1, 23)
                    ).with_span(1, 1, 1, 24)
                ).with_span(1, 1, 1, 24)
            }
        });
    }
}
