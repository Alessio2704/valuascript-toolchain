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
            auto reg = [](auto n, auto c, const std::vector<ParserExpectedError>& errs,
                          const OneOf<TypeVerifier>& v,
                          const std::vector<std::string_view>& skip_contexts = {},
                          const std::vector<ContextOverride<TypeVerifier>>& context_overrides = {})
            {
                ErrorRegistry::add(n, c, errs, v, skip_contexts, context_overrides);
            };

            reg("GarbageType", "123",
                {{E::MissingTypeAnnotation, 1, 1, 1, 4}},
                IsNullType()
            );

            reg("TupleTypeMissingElement", "(int, , string)",
                {{E::MissingTypeAnnotation, 1, 7, 1, 8}},
                IsTupleType({
                    IsType("int"),
                    IsNullType(),
                    IsType("string"),
                })
            );

            reg("TupleTypeBrokenElement", "(int, *, string)",
                {{E::MissingTypeAnnotation, 1, 7, 1, 8}},
                IsTupleType({
                    IsType("int"),
                    IsNullType(),
                    IsType("string"),
                })
            );

            reg("TupleTypeSingleElementDisallowed", "(int, )",
                {{E::TrailingCommaInTuple, 1, 5, 1, 6}},
                IsTupleType({
                    IsType("int"),
                })
            );

            reg("TupleTrailingCommaDisallowed", "(int, string, )",
                {{E::TrailingCommaInTuple, 1, 13, 1, 14}},
                IsTupleType({
                    IsType("int"),
                    IsType("string"),
                })
            );

            reg("EmptyGeneric", "map<>",
                {{E::EmptyGenericTypeAnnotation, 1, 5, 1, 6}},
                IsType("map", {})
            );

            reg("GenericMissingTypeAfterComma", "map<string, >",
                {{E::TrailingCommaInGenericArgument, 1, 11, 1, 12}},
                IsType("map", {
                           IsType("string")
                       }
                )
            );

            reg("GenericGarbageArgument", "map<string, *, int>",
                {{E::MissingTypeAnnotation, 1, 13, 1, 14}},
                IsType("map", {
                           IsType("string"),
                           IsNullType(),
                           IsType("int"),
                       }
                )
            );

            reg("GenericEmptyArgument", "map<string, , int>",
                {{E::MissingTypeAnnotation, 1, 13, 1, 14}},
                IsType("map", {
                           IsType("string"),
                           IsNullType(),
                           IsType("int"),
                       }
                )
            );

            reg("GenericMissingCommasBetweenArguments", "map<string int double>",
                {
                    {E::ExpectedCommaSeparatorInGenericArgs, 1, 12, 1, 15},
                    {E::ExpectedCommaSeparatorInGenericArgs, 1, 16, 1, 22}
                },
                IsType("map", {
                           IsType("string"),
                           IsType("int"),
                           IsType("double"),
                       }
                )
            );

            reg("TupleMissingCommasBetweenArguments", "(string int double)",
                {
                    {E::ExpectedCommaSeparatorInTupleType, 1, 9, 1, 12},
                    {E::ExpectedCommaSeparatorInTupleType, 1, 13, 1, 19}
                },
                IsTupleType({
                        IsType("string"),
                        IsType("int"),
                        IsType("double"),
                    }
                )
            );

            reg("GenericUnclosedNoArguments", "vector<",
                {
                    {E::EmptyGenericTypeAnnotation, 1, 8, 1, 9},
                    {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8},
                },
                IsType("vector", {}),
                {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeAssignmentTarget,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment({{"ctx_assign", IsType("vector", {})}}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 8, 1, 9},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsType("vector", {})}, {"ctx_m2"}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget2,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment({{"ctx_m1"}, {"ctx_m2", IsType("vector", {})}}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeExtensionTarget,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsExtensionDef({}, IsType("vector", {}), {}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {IsType("vector", {})}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 8, 1, 9},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("vector", {}), IsType("int")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturnEnd,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("int"), IsType("vector", {})}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 8, 1, 9},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec{.name = "p1", .type_v = IsType("vector", {})}, ParamSpec{.name = "p2", .type_v = IsType("int")}}, {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeStructField,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsStructDef("ctx_struct", {}, {FieldSpec{.name = "f", .type_v = IsType("vector", {})}}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeEnumUnderlyingType,
                        .errors = std::vector<ParserExpectedError>{
                            {E::EmptyGenericTypeAnnotation, 1, 9, 1, 10},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 7, 1, 8}
                        },
                        .verifier = OneOf<TypeVerifier>(IsEnumDef("ctx_enum", {}, IsType("vector", {}), {EnumCaseSpec{.name = "A"}}))
                    }
                }
            );

            reg("GenericMissingClosingBracket", "vector<int",
                {
                    {E::UnmatchedBracketAfterGenericArgs, 1, 10, 1, 11},
                },
                IsType("vector", {
                           IsType("int")
                       }
                ),
                {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 18, 1, 19}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsType("vector", {IsType("int"), IsType("ctx_m2")})}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 15, 1, 16}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {
                                                                          IsType("vector", {
                                                                              IsType("int"), IsType("int")
                                                                          })
                                                                      }))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 15, 1, 16}
                        },
                        .verifier = IsTupleType({IsType("vector", {IsType("int"), IsType("int")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 18, 1, 19}
                        },
                        .verifier = IsTupleType({IsType("int"), IsType("vector", {IsType("int"), IsType("string")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 16, 1, 17}
                        },
                        .verifier = IsType("vector", {IsType("vector", {IsType("int"), IsType("int")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 16, 1, 17}
                        },
                        .verifier = IsType("vector", {IsType("int"), IsType("vector", {IsType("int"), IsType("string")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 12, 1, 13}
                        },
                        .verifier = IsType("vector", {IsType("int"), IsType("string"), IsType("vector", {IsType("int")})})
                    },
                }
            );

            reg("TupleMissingClosingParen", "(int, string",
                {
                    {E::UnmatchedParenthesisInTuple, 1, 12, 1, 13},
                },
                IsTupleType({IsType("int"), IsType("string")}),
                {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParameter,
                        .errors = std::vector<ParserExpectedError>{
                            {E::ExpectedRightParenAfterParameters, 1, 13, 1, 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {},
                                                                      {ParamSpec{"p", {}, IsTupleType({IsType("int"), IsType("string")})}},
                                                                      {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter2,
                        .errors = std::vector<ParserExpectedError>{
                            {E::ExpectedRightParenAfterParameters, 1, 13, 1, 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {},
                                                                      {ParamSpec{"p1", {}, IsType("int")},
                                                                       ParamSpec{"p2", {}, IsTupleType({IsType("int"), IsType("string")})}},
                                                                      {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 13, 1, 14}
                        },
                        .verifier = IsTupleType({IsType("int"), IsType("string"), IsTupleType({IsType("int"), IsType("string")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 20, 1, 21}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsTupleType({IsType("int"), IsType("string"), IsType("ctx_m2")})}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 17, 1, 18}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {
                                                                          IsTupleType({
                                                                              IsType("int"), IsType("string"), IsType("int")
                                                                          })
                                                                      }))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 17, 1, 18}
                        },
                        .verifier = IsTupleType({IsTupleType({IsType("int"), IsType("string"), IsType("int")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 20, 1, 21}
                        },
                        .verifier = IsTupleType({IsType("int"), IsTupleType({IsType("int"), IsType("string"), IsType("string")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 18, 1, 19}
                        },
                        .verifier = IsType("vector", {IsTupleType({IsType("int"), IsType("string"), IsType("int")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 18, 1, 19}
                        },
                        .verifier = IsType("vector", {IsType("int"), IsTupleType({IsType("int"), IsType("string"), IsType("string")})})
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 14, 1, 15}
                        },
                        .verifier = IsType("vector", {IsType("int"), IsType("string"), IsTupleType({IsType("int"), IsType("string")})})
                    },
                }
            );

            reg("TupleMissingClosingParenNestedInGeneric", "vector<(int, string>",
                {
                    {E::UnmatchedParenthesisInTuple, 1, 19, 1, 20},
                    {E::UnmatchedBracketAfterGenericArgs, 1, 20, 1, 21},
                },
                IsType("vector", {IsTupleType({IsType("int"), IsType("string")})}),
                {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 19, 1, 20},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 28, 1, 29}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsType("vector", {IsTupleType({IsType("int"), IsType("string")}), IsType("ctx_m2")})}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 19, 1, 20},
                            {E::UnmatchedBracketAfterGenericArgs, 1, 25, 1, 26}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("vector", {IsTupleType({IsType("int"), IsType("string")}), IsType("int")})}))
                    }
                }
            );

            reg("TupleMissingClosingParenNestedInTuple", "((int, string, float)",
                {
                    {E::UnmatchedParenthesisInTuple, 1, 21, 1, 22},
                },
                IsTupleType({IsTupleType({IsType("int"), IsType("string"), IsType("float")})}),
                {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 29, 1, 30}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsTupleType({IsTupleType({IsType("int"), IsType("string"), IsType("float")}), IsType("ctx_m2")})}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedParenthesisInTuple, 1, 26, 1, 27}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsTupleType({IsTupleType({IsType("int"), IsType("string"), IsType("float")}), IsType("int")})}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParameter,
                        .errors = std::vector<ParserExpectedError>{
                            {E::ExpectedRightParenAfterParameters, 1, 22, 1, 23}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec{.name = "p", .type_v = IsTupleType({IsTupleType({IsType("int"), IsType("string"), IsType("float")})})}}, {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter2,
                        .errors = std::vector<ParserExpectedError>{
                            {E::ExpectedRightParenAfterParameters, 1, 22, 1, 23}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec{.name = "p1", .type_v = IsType("int")}, ParamSpec{.name = "p2", .type_v = IsTupleType({IsTupleType({IsType("int"), IsType("string"), IsType("float")})})}}, {IsType("void")}))
                    }
                }
            );

            reg("GenericMissingClosingBracketNestedInTuple", "(vector<int, string)",
                {
                    {E::UnmatchedBracketAfterGenericArgs, 1, 19, 1, 20},
                },
                IsTupleType({IsType("vector", {IsType("int"), IsType("string")})}),
                {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                }
            );

            reg("GenericMissingClosingBracketNestedInGeneric", "vector<map<int, string>",
                {
                    {E::UnmatchedBracketAfterGenericArgs, 1, 23, 1, 24},
                },
                IsType("vector", {IsType("map", {IsType("int"), IsType("string")})}),
                {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd
                },
                {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 31, 1, 32}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {{"ctx_m1", IsType("vector", {IsType("map", {IsType("int"), IsType("string")}), IsType("ctx_m2")})}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            {E::UnmatchedBracketAfterGenericArgs, 1, 28, 1, 29}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("vector", {IsType("map", {IsType("int"), IsType("string")}), IsType("int")})}))
                    }
                }
            );

            return true;
        }();
    }

    TEST_P(TypeAnnotationErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectTypeAnnotationErrors(code, errors, verifier, skip_contexts, context_overrides);
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
