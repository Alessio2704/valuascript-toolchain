#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names.h"

namespace valuascript::compiler::test
{
    class TypeAnnotationErrorRegistryRunner : public ParserTestBase,
                                              public testing::WithParamInterface<ErrorRegistryEntry<TypeVerifier>>
    {
    };

    using E = ParserErrorCode;

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const RecoveryCase<TypeVerifier>& spec) { ErrorRegistry::add(spec); };

            reg({
                .name = "GarbageType",
                .code = "123",
                .errors = {{E::MissingTypeAnnotation, 1, 1, 1, 4}},
                .verifier = IsNullType()
            });

            reg({
                .name = "TupleTypeMissingElement",
                .code = "(int, , string)",
                .errors = {{E::MissingTypeAnnotation, 1, 7, 1, 8}},
                .verifier = IsTupleType(
                    IsType("int"),
                    IsNullType(),
                    IsType("string")
                )
            });

            reg({
                .name = "TupleTypeBrokenElement",
                .code = "(int, *, string)",
                .errors = {{E::MissingTypeAnnotation, 1, 7, 1, 8}},
                .verifier = IsTupleType(
                    IsType("int"),
                    IsNullType(),
                    IsType("string")
                )
            });

            reg({
                .name = "TupleTypeSingleElementDisallowed",
                .code = "(int, )",
                .errors = {{E::TrailingCommaInTuple, 1, 5, 1, 6}},
                .verifier = IsTupleType(
                    IsType("int")
                )
            });

            reg({
                .name = "TupleTrailingCommaDisallowed",
                .code = "(int, string, )",
                .errors = {{E::TrailingCommaInTuple, 1, 13, 1, 14}},
                .verifier = IsTupleType(
                    IsType("int"),
                    IsType("string")
                )
            });

            reg({
                .name = "EmptyGeneric",
                .code = "map<>",
                .errors = {{E::EmptyGenericTypeAnnotation, 1, 5, 1, 6}},
                .verifier = IsType("map")
            });

            reg({
                .name = "GenericMissingTypeAfterComma",
                .code = "map<string, >",
                .errors = {{E::TrailingCommaInGenericArgument, 1, 11, 1, 12}},
                .verifier = IsType("map", IsType("string"))
            });

            reg({
                .name = "GenericGarbageArgument",
                .code = "map<string, *, int>",
                .errors = {{E::MissingTypeAnnotation, 1, 13, 1, 14}},
                .verifier = IsType("map",
                    IsType("string"),
                    IsNullType(),
                    IsType("int")
                )
            });

            reg({
                .name = "GenericEmptyArgument",
                .code = "map<string, , int>",
                .errors = {{E::MissingTypeAnnotation, 1, 13, 1, 14}},
                .verifier = IsType("map",
                    IsType("string"),
                    IsNullType(),
                    IsType("int")
                )
            });

            reg({
                .name = "GenericMissingCommasBetweenArguments",
                .code = "map<string int double>",
                .errors = {
                    {E::ExpectedCommaSeparatorInGenericArgs, 1, 12, 1, 15},
                    {E::ExpectedCommaSeparatorInGenericArgs, 1, 16, 1, 22}
                },
                .verifier = IsType("map",
                    IsType("string"),
                    IsType("int"),
                    IsType("double")
                )
            });

            reg({
                .name = "TupleMissingCommasBetweenArguments",
                .code = "(string int double)",
                .errors = {
                    {E::ExpectedCommaSeparatorInTupleType, 1, 9, 1, 12},
                    {E::ExpectedCommaSeparatorInTupleType, 1, 13, 1, 19}
                },
                .verifier = IsTupleType(
                    IsType("string"),
                    IsType("int"),
                    IsType("double")
                )
            });

            reg({
                .name = "GenericUnclosedNoArguments",
                .code = "vector<",
                .errors = {
                    {E::EmptyGenericTypeAnnotation, 1, 8, 1, 9},
                    {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8},
                },
                .verifier = IsType("vector"),
                .skip_contexts = {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeAssignmentTarget,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment({{"ctx_assign", IsType("vector")}}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 8, 1, 9},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsType("vector")}, {"ctx_m2"}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget2,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment({{"ctx_m1"}, {"ctx_m2", IsType("vector")}}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeExtensionTarget,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsExtensionDef({}, IsType("vector"), {}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {IsType("vector")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 8, 1, 9},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("vector"), IsType("int")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturnEnd,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("int"), IsType("vector")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 8, 1, 9},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec("p1", IsType("vector")), ParamSpec("p2", IsType("int"))}, {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeStructField,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsStructDef("ctx_struct", FieldSpec("f", IsType("vector"))))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeEnumUnderlyingType,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsEnumDef("ctx_enum", {}, IsType("vector"), EnumCaseSpec("A")))
                    }
                }
            });

            reg({
                .name = "GenericMissingClosingBracket",
                .code = "vector<int",
                .errors = {
                    {E::UnmatchedBracketAfterGenericArgs, 1, 10, 1, 11},
                },
                .verifier = IsType("vector", IsType("int")),
                .skip_contexts = {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 18, 1, 19}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsType("vector", IsType("int"), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 15, 1, 16}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {
                                                                          IsType("vector",
                                                                              IsType("int"), IsType("int")
                                                                          )
                                                                      }))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 15, 1, 16}
                        },
                        .verifier = IsTupleType(IsType("vector", IsType("int"), IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 18, 1, 19}
                        },
                        .verifier = IsTupleType(IsType("int"), IsType("vector", IsType("int"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 16, 1, 17}
                        },
                        .verifier = IsType("vector", IsType("vector", IsType("int"), IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 16, 1, 17}
                        },
                        .verifier = IsType("vector", IsType("int"), IsType("vector", IsType("int"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 12, 1, 13}
                        },
                        .verifier = IsType("vector", IsType("int"), IsType("string"), IsType("vector", IsType("int")))
                    }
                }
            });

            reg({
                .name = "TupleMissingClosingParen",
                .code = "(int, string",
                .errors = {
                    {E::UnmatchedParenthesisInTuple, 1, 12, 1, 13},
                },
                .verifier = IsTupleType(IsType("int"), IsType("string")),
                .skip_contexts = {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParameter,
                        .errors = std::vector<ParserExpectedError>{
                            {E::ExpectedRightParenAfterParameters, 1, 13, 1, 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {},
                                                                      {ParamSpec{"p", {}, IsTupleType(IsType("int"), IsType("string"))}},
                                                                      {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter2,
                        .errors = std::vector<ParserExpectedError>{
                            {E::ExpectedRightParenAfterParameters, 1, 13, 1, 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {},
                                                                      {ParamSpec{"p1", {}, IsType("int")},
                                                                       ParamSpec{"p2", {}, IsTupleType(IsType("int"), IsType("string"))}},
                                                                      {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 13, 1, 14}
                        },
                        .verifier = IsTupleType(IsType("int"), IsType("string"), IsTupleType(IsType("int"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 20, 1, 21}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsTupleType(IsType("int"), IsType("string"), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 17, 1, 18}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {
                                                                          IsTupleType(
                                                                              IsType("int"), IsType("string"), IsType("int")
                                                                          )
                                                                      }))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 17, 1, 18}
                        },
                        .verifier = IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 20, 1, 21}
                        },
                        .verifier = IsTupleType(IsType("int"), IsTupleType(IsType("int"), IsType("string"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 18, 1, 19}
                        },
                        .verifier = IsType("vector", IsTupleType(IsType("int"), IsType("string"), IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 18, 1, 19}
                        },
                        .verifier = IsType("vector", IsType("int"), IsTupleType(IsType("int"), IsType("string"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 14, 1, 15}
                        },
                        .verifier = IsType("vector", IsType("int"), IsType("string"), IsTupleType(IsType("int"), IsType("string")))
                    }
                }
            });

            reg({
                .name = "TupleMissingClosingParenNestedInGeneric",
                .code = "vector<(int, string>",
                .errors = {
                    {E::UnmatchedParenthesisInTuple, 1, 19, 1, 20},
                    {E::UnmatchedBracketAfterGenericArgs, 1, 20, 1, 21},
                },
                .verifier = IsType("vector", IsTupleType(IsType("int"), IsType("string"))),
                .skip_contexts = {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 19, 1, 20},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 28, 1, 29}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsType("vector", IsTupleType(IsType("int"), IsType("string")), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 19, 1, 20},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 25, 1, 26}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("vector", IsTupleType(IsType("int"), IsType("string")), IsType("int"))}))
                    }
                }
            });

            reg({
                .name = "TupleMissingClosingParenNestedInTuple",
                .code = "((int, string, float)",
                .errors = {
                    {E::UnmatchedParenthesisInTuple, 1, 21, 1, 22},
                },
                .verifier = IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("float"))),
                .skip_contexts = {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 29, 1, 30}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("float")), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 26, 1, 27}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("float")), IsType("int"))}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParameter,
                        .errors = std::vector<ParserExpectedError>{
                            {E::ExpectedRightParenAfterParameters, 1, 22, 1, 23}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec("p", IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("float"))))}, {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter2,
                        .errors = std::vector<ParserExpectedError>{
                            {E::ExpectedRightParenAfterParameters, 1, 22, 1, 23}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec("p1", IsType("int")), ParamSpec("p2", IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("float"))))}, {IsType("void")}))
                    }
                }
            });

            reg({
                .name = "GenericMissingClosingBracketNestedInTuple",
                .code = "(vector<int, string)",
                .errors = {
                    {E::UnmatchedBracketAfterGenericArgs, 1, 19, 1, 20},
                },
                .verifier = IsTupleType(IsType("vector", IsType("int"), IsType("string"))),
                .skip_contexts = {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                }
            });

            reg({
                .name = "GenericMissingClosingBracketNestedInGeneric",
                .code = "vector<map<int, string>",
                .errors = {
                    {E::UnmatchedBracketAfterGenericArgs, 1, 23, 1, 24},
                },
                .verifier = IsType("vector", IsType("map", IsType("int"), IsType("string"))),
                .skip_contexts = {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 31, 1, 32}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsType("vector", IsType("map", IsType("int"), IsType("string")), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 28, 1, 29}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("vector", IsType("map", IsType("int"), IsType("string")), IsType("int"))}))
                    }
                }
            });

            return true;
        }();
    }

    TEST_P(TypeAnnotationErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectTypeAnnotationErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels, accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAnnotation,
        TypeAnnotationErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::type_annotations()),
        [](const testing::TestParamInfo<ErrorRegistryEntry<TypeVerifier>>& test_info)
        {
        return test_info.param.test_name;
        }
    );
}
