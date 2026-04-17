#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test {
    class AssignmentSuccessPathTest : public ParserTestBase,
                                      public testing::WithParamInterface<ValidParserTestCase> {
    };

    TEST_P(AssignmentSuccessPathTest, ParsesSuccessfully) {
        run_valid_parser_test(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        AssignmentHappyPaths,
        AssignmentSuccessPathTest,
        ::testing::Values(
            ValidParserTestCase{
            "number_literal", "let a = 1000",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}}, IsNumber("1000")) } }
            },
            ValidParserTestCase{
            "percentage_literal_1", "let a = 1.5%",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}}, IsPercentage("1.5%")) } }
            },
            ValidParserTestCase{
            "percentage_literal_2", "let a = 0.000_001%",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}}, IsPercentage("0.000_001%")) } }
            },
            ValidParserTestCase{
            "string_literal", "let a = \"string\"",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}}, IsString("\"string\"")) } }
            },
            ValidParserTestCase{
            "boolean_true", "let a = true",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}}, IsBoolean(true)) } }
            },
            ValidParserTestCase{
            "boolean_false", "let _a = false",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"_a"}}, IsBoolean(false)) } }
            },
            ValidParserTestCase{
            "identifier_containing_keyword", "let ifthenelse = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"ifthenelse"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "explicit_type_simple", "let a: integer = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a", IsType("integer")}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "explicit_type_tuple", "let a: (integer, string) = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a", IsTupleType({IsType("integer"), IsType("string")})
                }}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "explicit_type_tuple_generic", "let a: (vector<integer>, string) = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a", IsTupleType({IsType("vector", {IsType("integer")})
                    , IsType("string")})
                }}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "explicit_type_generic", "let a: vector<integer> = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a", {IsType("vector", {IsType("integer")})}
                }}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "multi_assignment_2_vars", "let a, b = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}, {"b"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "multi_assignment_3_vars", "let a, b, c = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}, {"b"}, {"c"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "multi_assignment_4_vars", "let a, b, c, d = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}, {"b"}, {"c"}, {"d"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "multi_assignment_5_vars", "let a, b, c, d, e = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}, {"b"}, {"c"}, {"d"}, {"e"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "multi_assignment_type_all", "let a: int, b: int = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a", IsType("int")}, {"b", IsType("int")}}, IsNumber(
                "1")) } }
            },
            ValidParserTestCase{
            "multi_assignment_type_start", "let a: string, b = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a", IsType("string")}, {"b"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "multi_assignment_type_mid", "let a, b: string, c = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}, {"b", IsType("string")}, {"c"}}, IsNumber("1")) }
            }
            },
            ValidParserTestCase{
            "multi_assignment_type_end", "let a, b: bool = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({}, {{"a"}, {"b", IsType("bool")}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "modifier_1", "@export let a = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({{"export"}}, {{"a"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "modifier_2", "@export @memoize let a = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({{"export"}, {"memoize"}}, {{"a"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "modifier_3", "@mod1 @mod2 @mod3 let a = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({{"mod1"}, {"mod2"}, {"mod3"}}, {{"a"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "modifier_with_args", "@bind(target: \"ui\") let a = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({{"bind", {{"target", IsString("\"ui\"")}}}}, {{"a"}},
                IsNumber("1")) } }
            },
            ValidParserTestCase{
            "modifier_mixed_args_and_no_args", "@export @bind(target: \"ui\") @safe let a = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({{"export"}, {"bind", {{"target", IsString("\"ui\"")}}}, {
                "safe"}}, {{"a"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "modifier_across_newlines", "@export\n@safe\nlet a = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({{"export"}, {"safe"}}, {{"a"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "modifiers_with_multi_assignment", "@export let a, b = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({{"export"}}, {{"a"}, {"b"}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "modifiers_with_types", "@export let a: int = 1",
            ProgramSpec{ .execution_steps = { IsAssignment({{"export"}}, {{"a", IsType("int")}}, IsNumber("1")) } }
            },
            ValidParserTestCase{
            "the_works", "@export @meta(id: 10) let a: int, b, c: string = 1",
            ProgramSpec{ .execution_steps = {IsAssignment({{"export"}, {"meta", {{"id", IsNumber("10")}}}},{{"a", IsType
                ("int")}, {"b"}, {"c", IsType("string")}},IsNumber("1")
            )
            } }
            }
        ),
        [](const testing::TestParamInfo<ValidParserTestCase>& info) {
        return info.param.test_name;
        }
    );
}
