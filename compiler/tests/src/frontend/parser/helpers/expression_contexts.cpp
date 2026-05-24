#include "context_registry.h"
#include "spec_adder.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_expression_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                "single_assignment", {InjectableType::Expression}, InjectableType::StrongStatement, "let ctx_single = ",
                "\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({{"ctx_single"}}, SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                "multi_assignment", {InjectableType::Expression}, InjectableType::StrongStatement,
                "let ctx_m1, ctx_m2 = ", "\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({{"ctx_m1"}, {"ctx_m2"}},
                                                          SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                "reassignment", {InjectableType::Expression}, InjectableType::StrongStatement, "ctx_reassign = ", "\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsReassignment(IsIdentifier("ctx_reassign"), SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                "return_stmt", {InjectableType::Expression}, InjectableType::WeakStatement, "return ", "\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsReturn({SpecAdder::get_v<ExprVerifier>(v)}));
                }
            },
            {
                "func_def_default", {InjectableType::Expression}, InjectableType::TopLevel, "func ctx_func(arg: int = ",
                ") -> void {}\n", [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsFunctionDef("ctx_func", {},
                                                           {
                                                               ParamSpec{
                                                                   "arg", {}, IsType("int"),
                                                                   SpecAdder::get_v<ExprVerifier>(v)
                                                               }
                                                           }, {IsType("void")}));
                }
            },
            {
                "directive_no_eq", {InjectableType::Expression}, InjectableType::TopLevel, "#ctx_dir_no_eq ", "\n", [
                ](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDirective("ctx_dir_no_eq", SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                "directive_eq", {InjectableType::Expression}, InjectableType::TopLevel, "#ctx_dir_eq = ", "\n", [
                ](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDirective("ctx_dir_eq", SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                "enum_case", {InjectableType::Expression}, InjectableType::TopLevel, "enum CtxEnum: int { A = ", " }\n",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("CtxEnum", {}, IsType("int"),
                                                       {{"A", {}, SpecAdder::get_v<ExprVerifier>(v)}}));
                }
            },
            {
                "modifier_arg", {InjectableType::Expression}, InjectableType::Modifier, "@ctx_mod(arg: ", ") ", [
                ](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(std::vector<ModifierSpec>{
                        {"ctx_mod", {{"arg", SpecAdder::get_v<ExprVerifier>(v)}}}
                    });
                }
            },
            {
                "tuple_element", {InjectableType::Expression}, InjectableType::Expression, "(", ", 1)",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTuple({SpecAdder::get_v<ExprVerifier>(v), IsNumber("1")}));
                }
            },
            {
                "tensor_element", {InjectableType::Expression}, InjectableType::Expression, "[", ", 1]",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTensor({SpecAdder::get_v<ExprVerifier>(v), IsNumber("1")}));
                }
            },
            {
                "dict_value", {InjectableType::Expression}, InjectableType::Expression, "{ k: ", " }",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDict({{"k", {}, SpecAdder::get_v<ExprVerifier>(v)}}));
                }
            },
            {
                "bracket_access_index", {InjectableType::Expression}, InjectableType::Expression, "arr[", "]",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBracket(IsIdentifier("arr"), SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                "function_call_arg", {InjectableType::Expression}, InjectableType::Expression, "f(arg: ", ")",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsCall(IsIdentifier("f"), {{"arg", SpecAdder::get_v<ExprVerifier>(v)}}));
                }
            },
            {
                "binary_lhs", {InjectableType::Expression}, InjectableType::Expression, "(", ") + 100",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBinary(TokenType::Plus, IsGrouping(SpecAdder::get_v<ExprVerifier>(v)),
                                                      IsNumber("100")));
                }
            },
            {
                "binary_rhs", {InjectableType::Expression}, InjectableType::Expression, "100 + (", ")",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBinary(TokenType::Plus, IsNumber("100"),
                                                      IsGrouping(SpecAdder::get_v<ExprVerifier>(v))));
                }
            },
            {
                "grouping", {InjectableType::Expression}, InjectableType::Expression, "(", ")",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                "unary_grouping", {InjectableType::Expression}, InjectableType::Expression, "-(", ")",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsUnary(TokenType::Minus, IsGrouping(SpecAdder::get_v<ExprVerifier>(v))));
                }
            },
            {
                "as_call_target", {InjectableType::Expression}, InjectableType::Expression, "(", ")()",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsCall(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)), {}));
                }
            },
            {
                "as_dot_target", {InjectableType::Expression}, InjectableType::Expression, "(", ").prop",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDot(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)), "prop"));
                }
            },
            {
                "as_bracket_target", {InjectableType::Expression}, InjectableType::Expression, "(", ")[0]",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBracket(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)), IsNumber("0")));
                }
            },
            {
                "as_slice_target", {InjectableType::Expression}, InjectableType::Expression, "(", ")[0:10]",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBracket(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)),
                                                       IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10"))));
                }
            },
            {
                "switch_cond", {InjectableType::Expression}, InjectableType::Expression, "switch (",
                ") { default -> 1 }",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsSwitch(SpecAdder::get_v<ExprVerifier>(v), {}, IsNumber("1")));
                }
            },
            {
                "switch_case", {InjectableType::Expression}, InjectableType::Expression, "switch (1) { case A -> ",
                " default -> 1 }",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsSwitch(IsNumber("1"),
                                                      {SwitchCaseSpec{{"A"}, SpecAdder::get_v<ExprVerifier>(v)}},
                                                      IsNumber("1")));
                }
            },
            {
                "if_cond", {InjectableType::Expression}, InjectableType::Expression, "if ", " then 1 else 2",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsConditional(SpecAdder::get_v<ExprVerifier>(v), IsNumber("1"),
                                                           IsNumber("2")));
                }
            },
            {
                "if_then", {InjectableType::Expression}, InjectableType::Expression, "if 1 then ", " else 2",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsConditional(IsNumber("1"), SpecAdder::get_v<ExprVerifier>(v),
                                                           IsNumber("2")));
                }
            },
            {
                "if_else", {InjectableType::Expression}, InjectableType::Expression, "if 1 then 2 else ", "",
                [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsConditional(IsNumber("1"), IsNumber("2"),
                                                           SpecAdder::get_v<ExprVerifier>(v)));
                }
            }
        };

        return contexts;
    }
}
