#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class ReassignmentRegressionTest : public ParserTestBase
    {
    };

    using E = ParserErrorCode;

    TEST_F(ReassignmentRegressionTest, FunctionCallAndTupleReassignmentAmbiguity)
    {
        ExpectParseErrorsWithRecovery(
            "let ifthenelse = 1\n"
            "\n"
            "extension ctx_target {\n"
            "  my_function\n"
            "  (\n"
            "    arg: 1\n"
            "  )\n"
            "  (a, b) = 1\n"
            "  obj\n"
            "  .prop\n"
            "  =\n"
            "  1\n"
            "}\n"
            "\n"
            "func f() -> void {}\n",
            {PErr{.code = E::InvalidLeftSideExpressionInReassignment, .line_start = 8, .column_start = 3, .line_end = 8, .column_end = 9}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({AssignmentTargetSpec{.name = "ifthenelse"}}, IsNumber("1"))
                },
                .functions = {
                    IsFunctionDef("f", {}, {}, {IsType("void")}, {})
                },
                .extensions = {
                    IsExtensionDef({},
                                   IsType("ctx_target"),
                                   ProgramSpec{
                                       .execution_steps = {
                                           IsExprStmt(IsCall(IsIdentifier("my_function"), {{"arg", IsNumber("1")}})),
                                           IsReassignment(IsDot(IsIdentifier("obj"), "prop"), IsNumber("1"))
                                       }
                                   }
                    )
                }
            }
        );
    }

    TEST_F(ReassignmentRegressionTest, IncompleteReassignmentBeforeBracketReassignmentInFunction)
    {
        ExpectParseErrorsWithRecovery(
            "let a, b = 1\n"
            "\n"
            "func ctx_wrapper() -> void {\n"
            "  @mod2() return 1\n"
            "\n"
            "  a = \n"
            "  arr[0] = 1\n"
            "}\n"
            "\n"
            "struct Point { x: float, y: float, z: float }\n",
            {PErr{.code = E::MissingValueAfterEquals, .line_start = 6, .column_start = 5, .line_end = 6, .column_end = 6}},
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({AssignmentTargetSpec{.name = "a"}, AssignmentTargetSpec{.name = "b"}}, IsNumber("1"))
                },
                .functions = {
                    IsFunctionDef("ctx_wrapper", {}, {}, {IsType("void")}, {
                        IsReturn({{"mod2", {}}}, {IsNumber("1")}),
                        IsReassignment(IsIdentifier("a"), IsNull()),
                        IsReassignment(IsBracket(IsIdentifier("arr"), IsNumber("0")), IsNumber("1"))
                    })
                },
                .structs = {
                    IsStructDef("Point",
                        FieldSpec{.name = "x", .type_v = IsType("float")},
                        FieldSpec{.name = "y", .type_v = IsType("float")},
                        FieldSpec{.name = "z", .type_v = IsType("float")}
                    )
                }
            }
        );
    }

    TEST_F(ReassignmentRegressionTest, IncompleteReassignmentBeforeFunctionCallInExtension)
    {
        ExpectParseErrorsWithRecovery(
            "@mod2() import \"lib\"\n"
            "\n"
            "\n"
            "extension ctx_target {\n"
            "  a = 1\n"
            "  a = \n"
            "  init()\n"
            "}\n"
            "\n"
            "let a: int = 1\n",
            {PErr{.code = E::MissingValueAfterEquals, .line_start = 6, .column_start = 5, .line_end = 6, .column_end = 6}},
            ProgramSpec{
                .imports = {
                    IsImport("\"lib\"", {{"mod2", {}}})
                },
                .execution_steps = {
                    IsAssignment({AssignmentTargetSpec{.name = "a", .type_v = IsType("int")}}, IsNumber("1"))
                },
                .extensions = {
                    IsExtensionDef({}, IsType("ctx_target"), ProgramSpec{
                        .execution_steps = {
                            IsReassignment(IsIdentifier("a"), IsNumber("1")),
                            IsReassignment(IsIdentifier("a"), IsNull()),
                            IsExprStmt(IsCall(IsIdentifier("init"), {}))
                        }
                    })
                }
            }
        );
    }
}
