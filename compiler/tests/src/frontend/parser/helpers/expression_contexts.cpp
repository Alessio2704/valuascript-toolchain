#include "context_registry.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    std::vector<Context> ContextRegistry::get_expression_contexts()
    {
        using namespace SpecAdder;
        auto add_as_assign = [](ProgramSpec& s,
                                const UniversalVerifier& v,
                                const std::vector<AssignmentTargetSpec>& targets)
        {
            add(s, IsAssignment({}, targets, get_v<ExprVerifier>(v)));
        };

        return {
            {
                "single_assignment", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_single = ", "\n",
                [&](ProgramSpec& s, const UniversalVerifier& v) { add_as_assign(s, v, {{"ctx_single"}}); }
            },

            {
                "multi_assignment", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_m1, ctx_m2 = ",
                "\n",
                [&](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add_as_assign(s, v, {{"ctx_m1"}, {"ctx_m2"}});
                }
            },

            {
                "func_def_default", NestingLevel::TopLevel, {InjectableType::Expression}, "func ctx_func(arg: int = ",
                ") -> void {}\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsFunctionDef("ctx_func", {}, {ParamSpec{"arg", {}, IsType("int"), get_v<ExprVerifier>(v)}},
                                         {IsType("void")}));
                }
            },

            {
                "func_def_return", NestingLevel::TopLevel, {InjectableType::Expression},
                "func ctx_func_ret() -> int {\n  return ", "\n}\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsFunctionDef("ctx_func_ret", {}, {}, {IsType("int")},
                                         {IsReturn({get_v<ExprVerifier>(v)})}));
                }
            },

            {
                "directive_no_eq", NestingLevel::TopLevel, {InjectableType::Expression}, "#ctx_dir_no_eq ", "\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsDirective("ctx_dir_no_eq", get_v<ExprVerifier>(v)));
                }
            },

            {
                "directive_eq", NestingLevel::TopLevel, {InjectableType::Expression}, "#ctx_dir_eq = ", "\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsDirective("ctx_dir_eq", get_v<ExprVerifier>(v)));
                }
            },

            {
                "switch_cond", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_sw_cond = switch (",
                ") { default -> 1 }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_sw_cond"}}, IsSwitch(get_v<ExprVerifier>(v), {}, IsNumber("1"))));
                }
            },

            {
                "switch_case", NestingLevel::BlockLevel, {InjectableType::Expression},
                "let ctx_sw_case = switch (1) { case A -> ", " default -> 1 }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_sw_case"}},
                                        IsSwitch(IsNumber("1"), {SwitchCaseSpec{{"A"}, get_v<ExprVerifier>(v)}},
                                                 IsNumber("1"))));
                }
            },

            {
                "enum_case", NestingLevel::TopLevel, {InjectableType::Expression}, "enum CtxEnum: int { A = ", " }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsEnumDef("CtxEnum", {}, IsType("int"), {{"A", {}, get_v<ExprVerifier>(v)}}));
                }
            },

            {
                "modifier_arg", NestingLevel::BlockLevel, {InjectableType::Expression}, "@ctx_mod(arg: ",
                ")\nlet ctx_mod_var = 1\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({{"ctx_mod", {{"arg", get_v<ExprVerifier>(v)}}}}, {{"ctx_mod_var"}},
                                        IsNumber("1")));
                }
            },

            {
                "if_cond", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_if_cond = if ",
                " then 1 else 2\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_if_cond"}},
                                        IsConditional(get_v<ExprVerifier>(v), IsNumber("1"), IsNumber("2"))));
                }
            },

            {
                "if_then", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_if_then = if 1 then ",
                " else 2\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_if_then"}},
                                        IsConditional(IsNumber("1"), get_v<ExprVerifier>(v), IsNumber("2"))));
                }
            },

            {
                "if_else", NestingLevel::BlockLevel, {InjectableType::Expression},
                "let ctx_if_else = if 1 then 2 else ", "\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_if_else"}},
                                        IsConditional(IsNumber("1"), IsNumber("2"), get_v<ExprVerifier>(v))));
                }
            },

            {
                "reassignment", NestingLevel::BlockLevel, {InjectableType::Expression}, "ctx_reassign = ", "\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsReassignment(IsIdentifier("ctx_reassign"), get_v<ExprVerifier>(v)));
                }
            },

            {
                "tuple_element", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_tuple = (", ", 1)\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_tuple"}}, IsTuple({get_v<ExprVerifier>(v), IsNumber("1")})));
                }
            },

            {
                "tensor_element", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_tensor = [",
                ", 1]\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_tensor"}}, IsTensor({get_v<ExprVerifier>(v), IsNumber("1")})));
                }
            },

            {
                "dict_value", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_dict = { k: ", " }\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_dict"}}, IsDict({{"k", {}, get_v<ExprVerifier>(v)}})));
                }
            },

            {
                "bracket_access_index", NestingLevel::BlockLevel, {InjectableType::Expression},
                "let ctx_bracket = arr[", "]\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_bracket"}}, IsBracket(IsIdentifier("arr"), get_v<ExprVerifier>(v))));
                }
            },

            {
                "function_call_arg", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_call = f(arg: ",
                ")\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_call"}},
                                        IsCall(IsIdentifier("f"), {{"arg", get_v<ExprVerifier>(v)}})));
                }
            },

            {
                "binary_lhs", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_bin_lhs = (",
                ") + 100\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_bin_lhs"}},
                                        IsBinary(TokenType::Plus, IsGrouping(get_v<ExprVerifier>(v)),
                                                 IsNumber("100"))));
                }
            },

            {
                "binary_rhs", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_bin_rhs = 100 + (",
                ")\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_bin_rhs"}},
                                        IsBinary(TokenType::Plus, IsNumber("100"),
                                                 IsGrouping(get_v<ExprVerifier>(v)))));
                }
            },

            {
                "grouping", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_group = (", ")\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_group"}}, IsGrouping(get_v<ExprVerifier>(v))));
                }
            },

            {
                "unary_grouping", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_u_group = -(", ")\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_u_group"}},
                                        IsUnary(TokenType::Minus, IsGrouping(get_v<ExprVerifier>(v)))));
                }
            },

            {
                "as_call_target", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_as_call = (",
                ")()\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_as_call"}}, IsCall(IsGrouping(get_v<ExprVerifier>(v)), {})));
                }
            },

            {
                "as_dot_target", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_as_dot = (",
                ").prop\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_as_dot"}}, IsDot(IsGrouping(get_v<ExprVerifier>(v)), "prop")));
                }
            },

            {
                "as_bracket_target", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_as_bracket = (",
                ")[0]\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_as_bracket"}},
                                        IsBracket(IsGrouping(get_v<ExprVerifier>(v)), IsNumber("0"))));
                }
            },

            {
                "as_slice_target", NestingLevel::BlockLevel, {InjectableType::Expression}, "let ctx_as_slice = (",
                ")[0:10]\n",
                [](ProgramSpec& s, const UniversalVerifier& v)
                {
                    add(s, IsAssignment({}, {{"ctx_as_slice"}},
                                        IsBracket(IsGrouping(get_v<ExprVerifier>(v)),
                                                  IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10")))));
                }
            }
        };
    }
}
