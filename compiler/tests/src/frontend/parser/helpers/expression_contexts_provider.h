#pragma once

#include <string>
#include <vector>
#include <functional>
#include "node_matchers.h"

namespace valuascript::compiler::test
{
    struct ExpressionContext
    {
        std::string name;
        std::string source_template;
        std::function<void(ProgramSpec&, ExprVerifier)> add_to_spec;
    };

    class ExpressionContextsProvider
    {
    public:
        static std::string inject(const std::string& templ, const std::string& expr)
        {
            std::string res = templ;
            size_t pos = 0;
            while ((pos = res.find("{expr}", pos)) != std::string::npos)
            {
                res.replace(pos, 6, expr);
                pos += expr.length();
            }
            return res;
        }

        static std::vector<ExpressionContext> get_all()
        {
            return {
                {
                    "single_assignment",
                    "let ctx_single = {expr}\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(IsAssignment({}, {{"ctx_single"}}, v));
                    }
                },
                {
                    "multi_assignment",
                    "let ctx_m1, ctx_m2 = {expr}\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(IsAssignment({}, {{"ctx_m1"}, {"ctx_m2"}}, v));
                    }
                },
                {
                    "func_def_default",
                    "func ctx_func(arg: int = {expr}) -> void {}\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_func", {}, {ParamSpec{"arg", {}, IsType("int"), v}},
                                                            {IsType("void")}));
                    }
                },
                {
                    "func_def_return",
                    "func ctx_func_ret() -> int {\n    return {expr}\n}\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.functions.push_back(IsFunctionDef("ctx_func_ret", {}, {}, {IsType("int")}, {IsReturn({v})}));
                    }
                },
                {
                    "directive_no_eq",
                    "#ctx_dir_no_eq {expr}\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.directives.push_back(IsDirective("ctx_dir_no_eq", v));
                    }
                },
                {
                    "directive_eq",
                    "#ctx_dir_eq = {expr}\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.directives.push_back(IsDirective("ctx_dir_eq", v));
                    }
                },
                {
                    "switch_cond",
                    "let ctx_sw_cond = switch ({expr}) { default -> 1 }\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.
                          push_back(IsAssignment({}, {{"ctx_sw_cond"}}, IsSwitch(v, {}, IsNumber("1"))));
                    }
                },
                {
                    "switch_case",
                    "let ctx_sw_case = switch (1) { case A -> {expr} default -> 1 }\n",
                    [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(IsAssignment({}, {{"ctx_sw_case"}},
                                                                 IsSwitch(IsNumber("1"), {SwitchCaseSpec{{"A"}, v}},
                                                                          IsNumber("1"))));
                    }
                },
                {
                    "enum_case",
                    "enum CtxEnum: int { A = {expr} }\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.enums.push_back(IsEnumDef("CtxEnum", {}, IsType("int"), {{"A", {}, v}}));
                    }
                },
                {
                    "modifier_arg",
                    "@ctx_mod(arg: {expr})\nlet ctx_mod_var = 1\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({{"ctx_mod", {{"arg", v}}}}, {{"ctx_mod_var"}}, IsNumber("1")));
                    }
                },
                {
                    "if_cond",
                    "let ctx_if_cond = if {expr} then 1 else 2\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_if_cond"}}, IsConditional(v, IsNumber("1"), IsNumber("2"))));
                    }
                },
                {
                    "if_then",
                    "let ctx_if_then = if 1 then {expr} else 2\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_if_then"}}, IsConditional(IsNumber("1"), v, IsNumber("2"))));
                    }
                },
                {
                    "if_else",
                    "let ctx_if_else = if 1 then 2 else {expr}\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_if_else"}}, IsConditional(IsNumber("1"), IsNumber("2"), v)));
                    }
                },
                {
                    "reassignment",
                    "ctx_reassign = {expr}\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(IsReassignment(IsIdentifier("ctx_reassign"), v));
                    }
                },
                {
                    "tuple_element",
                    "let ctx_tuple = ({expr}, 1)\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(IsAssignment({}, {{"ctx_tuple"}}, IsTuple({v, IsNumber("1")})));
                    }
                },
                {
                    "tensor_element",
                    "let ctx_tensor = [{expr}, 1]\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(IsAssignment({}, {{"ctx_tensor"}}, IsTensor({v, IsNumber("1")})));
                    }
                },
                {
                    "dict_value",
                    "let ctx_dict = { k: {expr} }\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(IsAssignment({}, {{"ctx_dict"}}, IsDict({{"k", {}, v}})));
                    }
                },
                {
                    "bracket_access_index",
                    "let ctx_bracket = arr[{expr}]\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_bracket"}}, IsBracket(IsIdentifier("arr"), v)));
                    }
                },
                {
                    "function_call_arg",
                    "let ctx_call = f(arg: {expr})\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_call"}}, IsCall(IsIdentifier("f"), {{"arg", v}})));
                    }
                },
                {
                    "binary_lhs",
                    "let ctx_bin_lhs = ({expr}) + 100\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_bin_lhs"}},
                                         IsBinary(TokenType::Plus, IsGrouping(v), IsNumber("100")))
                        );
                    }
                },
                {
                    "binary_rhs",
                    "let ctx_bin_rhs = 100 + ({expr})\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_bin_rhs"}},
                                         IsBinary(TokenType::Plus, IsNumber("100"), IsGrouping(v)))
                        );
                    }
                },
                {
                    "grouping",
                    "let ctx_group = ({expr})\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(IsAssignment({}, {{"ctx_group"}}, IsGrouping(v)));
                    }
                },
                {
                    "unary_grouping",
                    "let ctx_u_group = -({expr})\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_u_group"}}, IsUnary(TokenType::Minus, IsGrouping(v))));
                    }
                },
                {
                    "as_call_target",
                    "let ctx_as_call = ({expr})()\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_as_call"}}, IsCall(IsGrouping(v), {}))
                        );
                    }
                },
                {
                    "as_dot_target",
                    "let ctx_as_dot = ({expr}).prop\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_as_dot"}}, IsDot(IsGrouping(v), "prop"))
                        );
                    }
                },
                {
                    "as_bracket_target",
                    "let ctx_as_bracket = ({expr})[0]\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_as_bracket"}}, IsBracket(IsGrouping(v), IsNumber("0")))
                        );
                    }
                },
                {
                    "as_slice_target",
                    "let ctx_as_slice = ({expr})[0:10]\n", [](ProgramSpec& s, const ExprVerifier& v)
                    {
                        s.execution_steps.push_back(
                            IsAssignment({}, {{"ctx_as_slice"}},
                                         IsBracket(
                                             IsGrouping(v),
                                             IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10"))
                                         )
                            )
                        );
                    }
                }
            };
        }
    };
}
