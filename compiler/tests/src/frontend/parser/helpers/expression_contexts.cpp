#include "context_registry.h"
#include "spec_adder.h"
#include "context_names.h"

namespace valuascript::compiler::test
{
    const std::vector<Context>& ContextRegistry::get_expression_contexts()
    {
        static const std::vector<Context> contexts = {
            {
                .name = ContextNames::ExprSingleAssignment,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ctx_single = ",
                .suffix = "\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({{"ctx_single"}}, SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                .name = ContextNames::ExprMultiAssignment,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::StrongStatement,
                .prefix = "let ctx_m1, ctx_m2 = ",
                .suffix = "\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsAssignment({{"ctx_m1"}, {"ctx_m2"}},
                                                          SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                .name = ContextNames::ExprReassignment,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::StrongStatement,
                .prefix = "ctx_reassign = ",
                .suffix = "\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(
                        IsReassignment(IsIdentifier("ctx_reassign"), SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                .name = ContextNames::ExprReturnStmt,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::WeakStatement,
                .prefix = "return ",
                .suffix = "\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsReturn({SpecAdder::get_v<ExprVerifier>(v)}));
                }
            },
            {
                .name = ContextNames::ExprFuncDefDefault,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::TopLevel,
                .prefix = "func ctx_func(arg: int = ",
                .suffix = ") -> void {}\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
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
                .name = ContextNames::ExprDirectiveNoEq,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::TopLevel,
                .prefix = "#ctx_dir_no_eq ",
                .suffix = "\n",
                .transform_verifier = [
                ](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDirective("ctx_dir_no_eq", SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                .name = ContextNames::ExprDirectiveEq,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::TopLevel,
                .prefix = "#ctx_dir_eq = ",
                .suffix = "\n",
                .transform_verifier = [
                ](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDirective("ctx_dir_eq", SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                .name = ContextNames::ExprEnumCase,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::TopLevel,
                .prefix = "enum CtxEnum: int { A = ",
                .suffix = " }\n",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsEnumDef("CtxEnum", {}, IsType("int"),
                                                       {{"A", {}, SpecAdder::get_v<ExprVerifier>(v)}}));
                }
            },
            {
                .name = ContextNames::ExprModifierArg,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Modifier,
                .prefix = "@ctx_mod(arg: ",
                .suffix = ") ",
                .transform_verifier = [
                ](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(std::vector<ModifierSpec>{
                        {"ctx_mod", {{"arg", SpecAdder::get_v<ExprVerifier>(v)}}}
                    });
                }
            },
            {
                .name = ContextNames::ExprTupleElement,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "(",
                .suffix = ", 1)",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTuple({SpecAdder::get_v<ExprVerifier>(v), IsNumber("1")}));
                }
            },
            {
                .name = ContextNames::ExprTensorElement,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "[",
                .suffix = ", 1]",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTensor({SpecAdder::get_v<ExprVerifier>(v), IsNumber("1")}));
                },
                .block_context = BlockContext::None,
                .transform_verifier_block = nullptr,
                .operator_binding_required = false,
                .transform_multi_verifier = [](const std::vector<UniversalVerifier>& vs) -> UniversalVerifier
                {
                    std::vector<ExprVerifier> elements;
                    for (const auto& v : vs) elements.push_back(SpecAdder::get_v<ExprVerifier>(v));
                    elements.push_back(IsNumber("1"));
                    return UniversalVerifier(IsTensor(elements));
                }
            },
            {
                .name = ContextNames::ExprTensorSingleElement,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "[",
                .suffix = "]",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsTensor({SpecAdder::get_v<ExprVerifier>(v)}));
                },
                .block_context = BlockContext::None,
                .transform_verifier_block = nullptr,
                .operator_binding_required = false,
                .transform_multi_verifier = [](const std::vector<UniversalVerifier>& vs) -> UniversalVerifier
                {
                    std::vector<ExprVerifier> elements;
                    for (const auto& v : vs) elements.push_back(SpecAdder::get_v<ExprVerifier>(v));
                    return UniversalVerifier(IsTensor(elements));
                }
            },
            {
                .name = ContextNames::ExprDictValue,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "{ k: ",
                .suffix = " }",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDict({{"k", {}, SpecAdder::get_v<ExprVerifier>(v)}}));
                }
            },
            {
                .name = ContextNames::ExprDictValueFirst,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "{ k: ",
                .suffix = ", x: 1 }",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDict({{"k", {}, SpecAdder::get_v<ExprVerifier>(v)}, {"x", {}, IsNumber("1")}}));
                }
            },
            {
                .name = ContextNames::ExprDictValueComma,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "{ k: ",
                .suffix = ", }",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDict({{"k", {}, SpecAdder::get_v<ExprVerifier>(v)}}));
                }
            },
            {
                .name = ContextNames::ExprBracketAccessIndex,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "arr[",
                .suffix = "]",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBracket(IsIdentifier("arr"), SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                .name = ContextNames::ExprFunctionCallArg,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "f(arg: ",
                .suffix = ")",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsCall(IsIdentifier("f"), {{"arg", SpecAdder::get_v<ExprVerifier>(v)}}));
                }
            },
            {
                .name = ContextNames::ExprBinaryLhs,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "(",
                .suffix = ") + 100",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBinary(TokenType::Plus, IsGrouping(SpecAdder::get_v<ExprVerifier>(v)),
                                                      IsNumber("100")));
                }
            },
            {
                .name = ContextNames::ExprBinaryRhs,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "100 + (",
                .suffix = ")",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBinary(TokenType::Plus, IsNumber("100"),
                                                      IsGrouping(SpecAdder::get_v<ExprVerifier>(v))));
                }
            },
            {
                .name = ContextNames::ExprGrouping,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "(",
                .suffix = ")",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)));
                }
            },
            {
                .name = ContextNames::ExprUnaryGrouping,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "-(",
                .suffix = ")",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsUnary(TokenType::Minus, IsGrouping(SpecAdder::get_v<ExprVerifier>(v))));
                }
            },
            {
                .name = ContextNames::ExprAsCallTarget,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "(",
                .suffix = ")()",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsCall(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)), {}));
                }
            },
            {
                .name = ContextNames::ExprAsDotTarget,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "(",
                .suffix = ").prop",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsDot(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)), "prop"));
                }
            },
            {
                .name = ContextNames::ExprAsBracketTarget,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "(",
                .suffix = ")[0]",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBracket(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)), IsNumber("0")));
                }
            },
            {
                .name = ContextNames::ExprAsSliceTarget,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "(",
                .suffix = ")[0:10]",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsBracket(IsGrouping(SpecAdder::get_v<ExprVerifier>(v)),
                                                       IsBinary(TokenType::Colon, IsNumber("0"), IsNumber("10"))));
                }
            },
            {
                .name = ContextNames::ExprSwitchCond,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "switch (",
                .suffix = ") { default -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsSwitch(SpecAdder::get_v<ExprVerifier>(v), {}, IsNumber("1")));
                }
            },
            {
                .name = ContextNames::ExprSwitchCase,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "switch (1) { case A -> ",
                .suffix = " default -> 1 }",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsSwitch(IsNumber("1"),
                                                      {SwitchCaseSpec{{"A"}, SpecAdder::get_v<ExprVerifier>(v)}},
                                                      IsNumber("1")));
                }
            },
            {
                .name = ContextNames::ExprIfCond,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "if ",
                .suffix = " then 1 else 2",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsConditional(SpecAdder::get_v<ExprVerifier>(v), IsNumber("1"),
                                                           IsNumber("2")));
                }
            },
            {
                .name = ContextNames::ExprIfThen,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "if 1 then ",
                .suffix = " else 2",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsConditional(IsNumber("1"), SpecAdder::get_v<ExprVerifier>(v),
                                                           IsNumber("2")));
                }
            },
            {
                .name = ContextNames::ExprIfElse,
                .input_types = {InjectableType::Expression},
                .output_type = InjectableType::Expression,
                .prefix = "if 1 then 2 else ",
                .suffix = "",
                .transform_verifier = [](const UniversalVerifier& v) -> UniversalVerifier
                {
                    return UniversalVerifier(IsConditional(IsNumber("1"), IsNumber("2"),
                                                           SpecAdder::get_v<ExprVerifier>(v)));
                }
            }
        };

        return contexts;
    }
}
