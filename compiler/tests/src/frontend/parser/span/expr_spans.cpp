#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

using namespace valuascript::compiler;

namespace valuascript::compiler::test
{
    class AstExprSpanTest : public ParserTestBase
    {
    protected:
        static std::shared_ptr<Program> parse_code(const std::string& code)
        {
            CompilerContext context;
            context.settings.fail_fast = true;
            return run_parser(code, context);
        }
    };

    TEST_F(AstExprSpanTest, NumericAndStringLiterals)
    {
        std::string code =
                "let a = 123.45\n"
                "let b = \"hello world\"";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "a"}.with_name_span(1, 5, 1, 6).with_span(1, 5, 1, 6)},
                    IsNumber("123.45").with_span(1, 9, 1, 15)
                ).with_span(1, 1, 1, 15),
                IsAssignment({AssignmentTargetSpec{.name = "b"}.with_name_span(2, 5, 2, 6).with_span(2, 5, 2, 6)},
                    IsString("\"hello world\"").with_span(2, 9, 2, 22)
                ).with_span(2, 1, 2, 22)
            }
        });
    }

    TEST_F(AstExprSpanTest, BooleanLiterals)
    {
        std::string code =
                "let t = true\n"
                "let f = false";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "t"}.with_name_span(1, 5, 1, 6).with_span(1, 5, 1, 6)},
                    IsBoolean(true).with_span(1, 9, 1, 13)
                ).with_span(1, 1, 1, 13),
                IsAssignment({AssignmentTargetSpec{.name = "f"}.with_name_span(2, 5, 2, 6).with_span(2, 5, 2, 6)},
                    IsBoolean(false).with_span(2, 9, 2, 14)
                ).with_span(2, 1, 2, 14)
            }
        });
    }

    TEST_F(AstExprSpanTest, TensorLiteralsFlatAndNested)
    {
        std::string code =
                "let t1 = [1, 2, 3]\n"
                "let t2 = [[10, 20], [30, 40]]";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "t1"}.with_name_span(1, 5, 1, 7).with_span(1, 5, 1, 7)},
                    IsTensor(
                        IsNumber("1").with_span(1, 11, 1, 12),
                        IsNumber("2").with_span(1, 14, 1, 15),
                        IsNumber("3").with_span(1, 17, 1, 18)
                    ).with_span(1, 10, 1, 19)
                ).with_span(1, 1, 1, 19),
                IsAssignment({AssignmentTargetSpec{.name = "t2"}.with_name_span(2, 5, 2, 7).with_span(2, 5, 2, 7)},
                    IsTensor(
                        IsTensor(
                            IsNumber("10").with_span(2, 12, 2, 14),
                            IsNumber("20").with_span(2, 16, 2, 18)
                        ).with_span(2, 11, 2, 19),
                        IsTensor(
                            IsNumber("30").with_span(2, 22, 2, 24),
                            IsNumber("40").with_span(2, 26, 2, 28)
                        ).with_span(2, 21, 2, 29)
                    ).with_span(2, 10, 2, 30)
                ).with_span(2, 1, 2, 30)
            }
        });
    }

    TEST_F(AstExprSpanTest, EmptyTensorAndDictLiterals)
    {
        std::string code =
                "let empty_arr = []\n"
                "let empty_dict = {}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "empty_arr"}.with_name_span(1, 5, 1, 14).with_span(1, 5, 1, 14)},
                    IsTensor().with_span(1, 17, 1, 19)
                ).with_span(1, 1, 1, 19),
                IsAssignment({AssignmentTargetSpec{.name = "empty_dict"}.with_name_span(2, 5, 2, 15).with_span(2, 5, 2, 15)},
                    IsDict().with_span(2, 18, 2, 20)
                ).with_span(2, 1, 2, 20)
            }
        });
    }

    TEST_F(AstExprSpanTest, DictLiteralWithDiverseKeysAndModifiers)
    {
        std::string code = "let map = { userName: 1, age: 2, isActive: true }";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "map"}.with_name_span(1, 5, 1, 8).with_span(1, 5, 1, 8)},
                    IsDict(
                        DictItemSpec{
                            .key = "userName",
                            .value_v = IsNumber("1").with_span(1, 23, 1, 24)
                        }.with_name_span(1, 13, 1, 21)
                         .with_span(1, 13, 1, 24),
                        DictItemSpec{
                            .key = "age",
                            .value_v = IsNumber("2").with_span(1, 31, 1, 32)
                        }.with_name_span(1, 26, 1, 29)
                         .with_span(1, 26, 1, 32),
                        DictItemSpec{
                            .key = "isActive",
                            .value_v = IsBoolean(true).with_span(1, 44, 1, 48)
                        }.with_name_span(1, 34, 1, 42)
                         .with_span(1, 34, 1, 48)
                    ).with_span(1, 11, 1, 50)
                ).with_span(1, 1, 1, 50)
            }
        });
    }

    TEST_F(AstExprSpanTest, NestedDictLiterals)
    {
        std::string code =
                "let omnibus = {\n"
                "    scalar: -100,\n"
                "    equation: (base + 0.05) * multiplier,\n"
                "    logic: not is_valid,\n"
                "    group: (1, a * b),\n"
                "    arr: [10, 20],\n"
                "    subset: history[0 : 10],\n"
                "    invoke: calc_risk(rate: 0.08),\n"
                "    nested: { inner: not not flag }\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({
                    AssignmentTargetSpec{.name = "omnibus"}
                        .with_name_span(1, 5, 1, 12)
                        .with_span(1, 5, 1, 12)
                },
                IsDict(
                    DictItemSpec{
                        .key = "scalar",
                        .value_v = IsUnary(TokenType::Minus,
                            IsNumber("100").with_span(2, 14, 2, 17)
                        ).with_span(2, 13, 2, 17)
                    }.with_name_span(2, 5, 2, 11)
                     .with_span(2, 5, 2, 17),
                    DictItemSpec{
                        .key = "equation",
                        .value_v = IsBinary(TokenType::Star,
                            IsGrouping(
                                IsBinary(TokenType::Plus,
                                    IsIdentifier("base").with_span(3, 16, 3, 20),
                                    IsNumber("0.05").with_span(3, 23, 3, 27)
                                ).with_span(3, 16, 3, 27)
                            ).with_span(3, 15, 3, 28),
                            IsIdentifier("multiplier").with_span(3, 31, 3, 41)
                        ).with_span(3, 15, 3, 41)
                    }.with_name_span(3, 5, 3, 13)
                     .with_span(3, 5, 3, 41),
                    DictItemSpec{
                        .key = "logic",
                        .value_v = IsUnary(TokenType::Not,
                            IsIdentifier("is_valid").with_span(4, 16, 4, 24)
                        ).with_span(4, 12, 4, 24)
                    }.with_name_span(4, 5, 4, 10)
                     .with_span(4, 5, 4, 24),
                    DictItemSpec{
                        .key = "group",
                        .value_v = IsTuple(
                            IsNumber("1").with_span(5, 13, 5, 14),
                            IsBinary(TokenType::Star,
                                IsIdentifier("a").with_span(5, 16, 5, 17),
                                IsIdentifier("b").with_span(5, 20, 5, 21)
                            ).with_span(5, 16, 5, 21)
                        ).with_span(5, 12, 5, 22)
                    }.with_name_span(5, 5, 5, 10)
                     .with_span(5, 5, 5, 22),
                    DictItemSpec{
                        .key = "arr",
                        .value_v = IsTensor(
                            IsNumber("10").with_span(6, 11, 6, 13),
                            IsNumber("20").with_span(6, 15, 6, 17)
                        ).with_span(6, 10, 6, 18)
                    }.with_name_span(6, 5, 6, 8)
                     .with_span(6, 5, 6, 18),
                    DictItemSpec{
                        .key = "subset",
                        .value_v = IsBracket(
                            IsIdentifier("history").with_span(7, 13, 7, 20),
                            AnyMatcher{}
                        ).with_span(7, 13, 7, 28)
                    }.with_name_span(7, 5, 7, 11)
                     .with_span(7, 5, 7, 28),
                    DictItemSpec{
                        .key = "invoke",
                        .value_v = IsCall(
                            IsIdentifier("calc_risk").with_span(8, 13, 8, 22),
                            ArgSpec{
                                .label = "rate",
                                .value_v = IsNumber("0.08").with_span(8, 29, 8, 33)
                            }.with_name_span(8, 23, 8, 27)
                             .with_span(8, 23, 8, 33)
                        ).with_span(8, 13, 8, 34)
                    }.with_name_span(8, 5, 8, 11)
                     .with_span(8, 5, 8, 34),
                    DictItemSpec{
                        .key = "nested",
                        .value_v = IsDict(
                            DictItemSpec{
                                .key = "inner",
                                .value_v = IsUnary(TokenType::Not,
                                    IsUnary(TokenType::Not,
                                        IsIdentifier("flag").with_span(9, 30, 9, 34)
                                    ).with_span(9, 26, 9, 34)
                                ).with_span(9, 22, 9, 34)
                            }.with_name_span(9, 15, 9, 20)
                             .with_span(9, 15, 9, 34)
                        ).with_span(9, 13, 9, 36)
                    }.with_name_span(9, 5, 9, 11)
                     .with_span(9, 5, 9, 36)
                ).with_span(1, 15, 10, 2)
                ).with_span(1, 1, 10, 2)
            }
        });
    }

    TEST_F(AstExprSpanTest, TupleLiterals)
    {
        std::string code = "let pair = (1, \"text\")";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "pair"}.with_name_span(1, 5, 1, 9).with_span(1, 5, 1, 9)},
                    IsTuple(
                        IsNumber("1").with_span(1, 13, 1, 14),
                        IsString("\"text\"").with_span(1, 16, 1, 22)
                    ).with_span(1, 12, 1, 23)
                ).with_span(1, 1, 1, 23)
            }
        });
    }

    TEST_F(AstExprSpanTest, BinaryAndUnaryExpressionPrecedence)
    {
        std::string code =
                "let y = 10 + 20 * 3\n"
                "let r = -x + not y\n"
                "let d = not not flag";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "y"}.with_name_span(1, 5, 1, 6).with_span(1, 5, 1, 6)},
                    IsBinary(TokenType::Plus,
                        IsNumber("10").with_span(1, 9, 1, 11),
                        IsBinary(TokenType::Star,
                            IsNumber("20").with_span(1, 14, 1, 16),
                            IsNumber("3").with_span(1, 19, 1, 20)
                        ).with_span(1, 14, 1, 20)
                    ).with_span(1, 9, 1, 20)
                ).with_span(1, 1, 1, 20),
                IsAssignment({AssignmentTargetSpec{.name = "r"}.with_name_span(2, 5, 2, 6).with_span(2, 5, 2, 6)},
                    IsBinary(TokenType::Plus,
                        IsUnary(TokenType::Minus, IsIdentifier("x").with_span(2, 10, 2, 11)).with_span(2, 9, 2, 11),
                        IsUnary(TokenType::Not, IsIdentifier("y").with_span(2, 18, 2, 19)).with_span(2, 14, 2, 19)
                    ).with_span(2, 9, 2, 19)
                ).with_span(2, 1, 2, 19),
                IsAssignment({AssignmentTargetSpec{.name = "d"}.with_name_span(3, 5, 3, 6).with_span(3, 5, 3, 6)},
                    IsUnary(TokenType::Not,
                        IsUnary(TokenType::Not, IsIdentifier("flag").with_span(3, 17, 3, 21)).with_span(3, 13, 3, 21)
                    ).with_span(3, 9, 3, 21)
                ).with_span(3, 1, 3, 21)
            }
        });
    }

    TEST_F(AstExprSpanTest, GroupingExpressions)
    {
        std::string code = "let a = (1 + 2) * 3";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "a"}.with_name_span(1, 5, 1, 6).with_span(1, 5, 1, 6)},
                    IsBinary(TokenType::Star,
                        IsGrouping(
                            IsBinary(TokenType::Plus,
                                IsNumber("1").with_span(1, 10, 1, 11),
                                IsNumber("2").with_span(1, 14, 1, 15)
                            ).with_span(1, 10, 1, 15)
                        ).with_span(1, 9, 1, 16),
                        IsNumber("3").with_span(1, 19, 1, 20)
                    ).with_span(1, 9, 1, 20)
                ).with_span(1, 1, 1, 20)
            }
        });
    }

    TEST_F(AstExprSpanTest, DotAccessChains)
    {
        std::string code =
                "let v = obj.prop\n"
                "let deep = a.b.c";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "v"}.with_name_span(1, 5, 1, 6).with_span(1, 5, 1, 6)},
                    IsDot(
                        IsIdentifier("obj").with_span(1, 9, 1, 12),
                        "prop"
                    ).with_name_span(1, 13, 1, 17)
                     .with_span(1, 9, 1, 17)
                ).with_span(1, 1, 1, 17),
                IsAssignment({AssignmentTargetSpec{.name = "deep"}.with_name_span(2, 5, 2, 9).with_span(2, 5, 2, 9)},
                    IsDot(
                        IsDot(
                            IsIdentifier("a").with_span(2, 12, 2, 13),
                            "b"
                        ).with_name_span(2, 14, 2, 15)
                         .with_span(2, 12, 2, 15),
                        "c"
                    ).with_name_span(2, 16, 2, 17)
                     .with_span(2, 12, 2, 17)
                ).with_span(2, 1, 2, 17)
            }
        });
    }

    TEST_F(AstExprSpanTest, BracketAccessChains)
    {
        std::string code = "let arr = [1, 2][0]";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "arr"}.with_name_span(1, 5, 1, 8).with_span(1, 5, 1, 8)},
                    IsBracket(
                        IsTensor(
                            IsNumber("1").with_span(1, 12, 1, 13),
                            IsNumber("2").with_span(1, 15, 1, 16)
                        ).with_span(1, 11, 1, 17),
                        IsNumber("0").with_span(1, 18, 1, 19)
                    ).with_span(1, 11, 1, 20)
                ).with_span(1, 1, 1, 20)
            }
        });
    }

    TEST_F(AstExprSpanTest, FunctionCallChainsAndLabeledArgs)
    {
        std::string code =
                "let res = compute(source: data, timeoutMs: 5000)\n"
                "let chain = user.get_roles()[0].name\n"
                "let z = outer(p: inner(p: 42), s: \"test\")";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "res"}
                    .with_name_span(1, 5, 1, 8)
                    .with_span(1, 5, 1, 8)},
                    IsCall(
                        IsIdentifier("compute").with_span(1, 11, 1, 18),
                        ArgSpec{
                            .label = "source",
                            .value_v = IsIdentifier("data").with_span(1, 27, 1, 31)
                        }.with_name_span(1, 19, 1, 25)
                         .with_span(1, 19, 1, 31),
                        ArgSpec{
                            .label = "timeoutMs",
                            .value_v = IsNumber("5000").with_span(1, 44, 1, 48)
                        }.with_name_span(1, 33, 1, 42)
                         .with_span(1, 33, 1, 48)
                    ).with_span(1, 11, 1, 49)
                ).with_span(1, 1, 1, 49),
                IsAssignment({AssignmentTargetSpec{.name = "chain"}
                    .with_name_span(2, 5, 2, 10)
                    .with_span(2, 5, 2, 10)},
                    IsDot(
                        IsBracket(
                            IsCall(
                                IsDot(
                                    IsIdentifier("user").with_span(2, 13, 2, 17),
                                    "get_roles"
                                ).with_name_span(2, 18, 2, 27)
                                 .with_span(2, 13, 2, 27)
                            ).with_span(2, 13, 2, 29),
                            IsNumber("0").with_span(2, 30, 2, 31)
                        ).with_span(2, 13, 2, 32),
                        "name"
                    ).with_name_span(2, 33, 2, 37)
                     .with_span(2, 13, 2, 37)
                ).with_span(2, 1, 2, 37),
                IsAssignment({AssignmentTargetSpec{.name = "z"}
                    .with_name_span(3, 5, 3, 6)
                    .with_span(3, 5, 3, 6)},
                    IsCall(
                        IsIdentifier("outer").with_span(3, 9, 3, 14),
                        ArgSpec{
                            .label = "p",
                            .value_v = IsCall(
                                IsIdentifier("inner").with_span(3, 18, 3, 23),
                                ArgSpec{
                                    .label = "p",
                                    .value_v = IsNumber("42").with_span(3, 27, 3, 29)
                                }.with_name_span(3, 24, 3, 25)
                                 .with_span(3, 24, 3, 29)
                            ).with_span(3, 18, 3, 30)
                        }.with_name_span(3, 15, 3, 16)
                         .with_span(3, 15, 3, 30),
                        ArgSpec{
                            .label = "s",
                            .value_v = IsString("\"test\"").with_span(3, 35, 3, 41)
                        }.with_name_span(3, 32, 3, 33)
                         .with_span(3, 32, 3, 41)
                    ).with_span(3, 9, 3, 42)
                ).with_span(3, 1, 3, 42)
            }
        });
    }

    TEST_F(AstExprSpanTest, SwitchExpressionsWithMultipleCases)
    {
        std::string code =
                "let res = switch (x) {\n"
                "    case Alpha, Beta, Gamma -> 10\n"
                "    default -> 0\n"
                "}";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "res"}
                    .with_name_span(1, 5, 1, 8)
                    .with_span(1, 5, 1, 8)},
                    IsSwitch(
                        IsIdentifier("x").with_span(1, 19, 1, 20),
                        {
                            SwitchCaseSpec{
                                .labels = {"Alpha", "Beta", "Gamma"},
                                .result_v = IsNumber("10").with_span(2, 32, 2, 34)
                            }.with_span(2, 5, 2, 34)
                             .with_label_spans({
                                 SourceSpan{.line_start = 2, .column_start = 10, .line_end = 2, .column_end = 15, .start_offset = 32, .length = 5},
                                 SourceSpan{.line_start = 2, .column_start = 17, .line_end = 2, .column_end = 21, .start_offset = 39, .length = 4},
                                 SourceSpan{.line_start = 2, .column_start = 23, .line_end = 2, .column_end = 28, .start_offset = 45, .length = 5}
                             })
                        },
                        /*default_expr=*/IsNumber("0").with_span(3, 16, 3, 17)
                    ).with_span(1, 11, 4, 2)
                ).with_span(1, 1, 4, 2)
            }
        });
    }

    TEST_F(AstExprSpanTest, SliceExpressions)
    {
        std::string code = "let sub = items[1 : 5]";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "sub"}.with_name_span(1, 5, 1, 8).with_span(1, 5, 1, 8)},
                    IsBracket(
                        IsIdentifier("items").with_span(1, 11, 1, 16),
                        IsBinary(TokenType::Colon,
                            IsNumber("1").with_span(1, 17, 1, 18),
                            IsNumber("5").with_span(1, 21, 1, 22)
                        ).with_span(1, 17, 1, 22)
                    ).with_span(1, 11, 1, 23)
                ).with_span(1, 1, 1, 23)
            }
        });
    }

    TEST_F(AstExprSpanTest, TypeAnnotationSpansGenericAndTuple)
    {
        std::string code =
                "let map: Map<string, int> = 10\n"
                "let pair: (int, int) = (1, 2)\n"
                "let mat: matrix<int> = [[1]]";

        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({
                    AssignmentTargetSpec{
                        .name = "map",
                        .type_v = IsType("Map",
                            IsType("string").with_name_span(1, 14, 1, 20).with_span(1, 14, 1, 20),
                            IsType("int").with_name_span(1, 22, 1, 25).with_span(1, 22, 1, 25)
                        ).with_name_span(1, 10, 1, 13).with_span(1, 10, 1, 26)
                    }.with_name_span(1, 5, 1, 8)
                     .with_span(1, 5, 1, 26)
                }, IsNumber("10").with_span(1, 29, 1, 31)
                ).with_span(1, 1, 1, 31),
                IsAssignment({
                    AssignmentTargetSpec{
                        .name = "pair",
                        .type_v = IsTupleType(
                            IsType("int").with_name_span(2, 12, 2, 15).with_span(2, 12, 2, 15),
                            IsType("int").with_name_span(2, 17, 2, 20).with_span(2, 17, 2, 20)
                        ).with_span(2, 11, 2, 21)
                    }.with_name_span(2, 5, 2, 9)
                     .with_span(2, 5, 2, 21)
                },
                IsTuple(
                    IsNumber("1").with_span(2, 25, 2, 26),
                    IsNumber("2").with_span(2, 28, 2, 29)
                ).with_span(2, 24, 2, 30)
                ).with_span(2, 1, 2, 30),
                IsAssignment({
                    AssignmentTargetSpec{
                        .name = "mat",
                        .type_v = IsType("matrix",
                            IsType("int").with_name_span(3, 17, 3, 20).with_span(3, 17, 3, 20)
                        ).with_name_span(3, 10, 3, 16).with_span(3, 10, 3, 21)
                    }.with_name_span(3, 5, 3, 8)
                     .with_span(3, 5, 3, 21)
                },
                IsTensor(
                    IsTensor(
                        IsNumber("1").with_span(3, 26, 3, 27)
                    ).with_span(3, 25, 3, 28)
                ).with_span(3, 24, 3, 29)
                ).with_span(3, 1, 3, 29)
            }
        });
    }

    TEST_F(AstExprSpanTest, PercentageLiterals)
    {
        std::string code = "let discount = 15.5%";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "discount"}.with_name_span(1, 5, 1, 13).with_span(1, 5, 1, 13)},
                    IsPercentage("15.5%").with_span(1, 16, 1, 21)
                ).with_span(1, 1, 1, 21)
            }
        });
    }

    TEST_F(AstExprSpanTest, SelfExpressions)
    {
        std::string code = "let prop = self.value";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "prop"}.with_name_span(1, 5, 1, 9).with_span(1, 5, 1, 9)},
                    IsDot(
                        IsSelf().with_span(1, 12, 1, 16),
                        "value"
                    ).with_name_span(1, 17, 1, 22)
                     .with_span(1, 12, 1, 22)
                ).with_span(1, 1, 1, 22)
            }
        });
    }

    TEST_F(AstExprSpanTest, ConditionalExpressions)
    {
        std::string code = "let res = if isValid then 10 else 20";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "res"}.with_name_span(1, 5, 1, 8).with_span(1, 5, 1, 8)},
                    IsConditional(
                        IsIdentifier("isValid").with_span(1, 14, 1, 21),
                        IsNumber("10").with_span(1, 27, 1, 29),
                        IsNumber("20").with_span(1, 35, 1, 37)
                    ).with_span(1, 11, 1, 37)
                ).with_span(1, 1, 1, 37)
            }
        });
    }

    TEST_F(AstExprSpanTest, LogicalAndComparisonExpressions)
    {
        std::string code = "let cond = (a == b) and (c < d) or not flag";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({AssignmentTargetSpec{.name = "cond"}.with_name_span(1, 5, 1, 9).with_span(1, 5, 1, 9)},
                    IsBinary(TokenType::Or,
                        IsBinary(TokenType::And,
                            IsGrouping(
                                IsBinary(TokenType::Equals,
                                    IsIdentifier("a").with_span(1, 13, 1, 14),
                                    IsIdentifier("b").with_span(1, 18, 1, 19)
                                ).with_span(1, 13, 1, 19)
                            ).with_span(1, 12, 1, 20),
                            IsGrouping(
                                IsBinary(TokenType::Less,
                                    IsIdentifier("c").with_span(1, 26, 1, 27),
                                    IsIdentifier("d").with_span(1, 30, 1, 31)
                                ).with_span(1, 26, 1, 31)
                            ).with_span(1, 25, 1, 32)
                        ).with_span(1, 12, 1, 32),
                        IsUnary(TokenType::Not,
                            IsIdentifier("flag").with_span(1, 40, 1, 44)
                        ).with_span(1, 36, 1, 44)
                    ).with_span(1, 12, 1, 44)
                ).with_span(1, 1, 1, 44)
            }
        });
    }

    TEST_F(AstExprSpanTest, DeeplyNestedGenericTypeAnnotations)
    {
        std::string code = "let complex: Map<string, list<int>> = 0";
        auto ast = parse_code(code);
        ExpectProgram(ast.get(), {
            .execution_steps = {
                IsAssignment({
                    AssignmentTargetSpec{
                        .name = "complex",
                        .type_v = IsType("Map",
                            IsType("string").with_name_span(1, 18, 1, 24).with_span(1, 18, 1, 24),
                            IsType("list",
                                IsType("int").with_name_span(1, 31, 1, 34).with_span(1, 31, 1, 34)
                            ).with_name_span(1, 26, 1, 30).with_span(1, 26, 1, 35)
                        ).with_name_span(1, 14, 1, 17).with_span(1, 14, 1, 36)
                    }.with_name_span(1, 5, 1, 12)
                     .with_span(1, 5, 1, 36)
                },
                IsNumber("0").with_span(1, 39, 1, 40)
                ).with_span(1, 1, 1, 40)
            }
        });
    }
}
