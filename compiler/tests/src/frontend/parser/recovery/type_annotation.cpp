#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/context_names_helpers.h"

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
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 1, .line_end = 1,
                        .column_end = 4
                    }
                },
                .verifier = IsNullType()
            });

            reg({
                .name = "TupleTypeMissingElement",
                .code = "(int, , string)",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 7, .line_end = 1,
                        .column_end = 8
                    }
                },
                .verifier = IsTupleType(
                    IsType("int"),
                    IsNullType(),
                    IsType("string")
                )
            });

            reg({
                .name = "TupleTypeBrokenElement",
                .code = "(int, *, string)",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 7, .line_end = 1,
                        .column_end = 8
                    }
                },
                .verifier = IsTupleType(
                    IsType("int"),
                    IsNullType(),
                    IsType("string")
                )
            });

            reg({
                .name = "TupleTypeSingleElementDisallowed",
                .code = "(int, )",
                .errors = {
                    PErr{
                        .code = E::TrailingCommaInTuple, .line_start = 1, .column_start = 5, .line_end = 1,
                        .column_end = 6
                    }
                },
                .verifier = IsTupleType(
                    IsType("int")
                )
            });

            reg({
                .name = "TupleTrailingCommaDisallowed",
                .code = "(int, string, )",
                .errors = {
                    PErr{
                        .code = E::TrailingCommaInTuple, .line_start = 1, .column_start = 13, .line_end = 1,
                        .column_end = 14
                    }
                },
                .verifier = IsTupleType(
                    IsType("int"),
                    IsType("string")
                )
            });

            reg({
                .name = "EmptyGeneric",
                .code = "map<>",
                .errors = {
                    PErr{
                        .code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 5, .line_end = 1,
                        .column_end = 6
                    }
                },
                .verifier = IsType("map")
            });

            reg({
                .name = "MissingBaseTypeBeforeGeneric",
                .code = "<int>",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 1, .line_end = 1,
                        .column_end = 2
                    }
                },
                .verifier = IsNullType()
            });

            reg({
                .name = "MultipleReturnTypesTrailingCommaError",
                .code = "int, string,",
                .errors = {
                    PErr{.code = E::TrailingComma, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                },
                .verifier = IsTupleType(IsType("int"), IsType("string")),
                .skip_contexts = ContextNames::all(),
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 13, .line_end = 1,
                                .column_end = 14
                            }
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {
                                                                          IsType("int"), IsType("string"), IsNullType(),
                                                                          IsType("int"), IsType("string")
                                                                      })),
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 13, .line_end = 1,
                                .column_end = 14
                            }
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {
                                                                          IsType("int"), IsType("int"),
                                                                          IsType("string"), IsNullType(),
                                                                          IsType("string")
                                                                      })),
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::TrailingComma, .line_start = 1, .column_start = 12, .line_end = 1,
                                .column_end = 13
                            }
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {IsType("int"), IsType("string")})),
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::TrailingComma, .line_start = 1, .column_start = 12, .line_end = 1,
                                .column_end = 13
                            }
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {
                                                                          IsType("int"), IsType("string"),
                                                                          IsType("int"), IsType("string")
                                                                      })),
                    }
                }
            });

            reg({
                .name = "GenericMissingTypeAfterComma",
                .code = "map<string, >",
                .errors = {
                    PErr{
                        .code = E::TrailingCommaInGenericArgument, .line_start = 1, .column_start = 11, .line_end = 1,
                        .column_end = 12
                    }
                },
                .verifier = IsType("map", IsType("string"))
            });

            reg({
                .name = "GenericGarbageArgument",
                .code = "map<string, *, int>",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 13, .line_end = 1,
                        .column_end = 14
                    }
                },
                .verifier = IsType("map",
                                   IsType("string"),
                                   IsNullType(),
                                   IsType("int")
                )
            });

            reg({
                .name = "GenericEmptyArgument",
                .code = "map<string, , int>",
                .errors = {
                    PErr{
                        .code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 13, .line_end = 1,
                        .column_end = 14
                    }
                },
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
                    PErr{
                        .code = E::ExpectedCommaSeparatorInGenericArgs, .line_start = 1, .column_start = 12,
                        .line_end = 1, .column_end = 15
                    },
                    PErr{
                        .code = E::ExpectedCommaSeparatorInGenericArgs, .line_start = 1, .column_start = 16,
                        .line_end = 1, .column_end = 22
                    }
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
                    PErr{
                        .code = E::ExpectedCommaSeparatorInTupleType, .line_start = 1, .column_start = 9, .line_end = 1,
                        .column_end = 12
                    },
                    PErr{
                        .code = E::ExpectedCommaSeparatorInTupleType, .line_start = 1, .column_start = 13,
                        .line_end = 1, .column_end = 19
                    }
                },
                .verifier = IsTupleType(
                    IsType("string"),
                    IsType("int"),
                    IsType("double")
                )
            });

            reg({
                .name = "GenericMissingClosingBracket",
                .code = "vector<int",
                .errors = {
                    PErr{
                        .code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 10, .line_end = 1,
                        .column_end = 11
                    },
                },
                .verifier = IsType("vector", IsType("int")),
                .skip_contexts = ContextNames::all_nested_swallowing_type_contexts(),
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 23,
                                .line_end = 1, .column_end = 24
                            }
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {
                                                                          IsType("vector", IsType("int"), IsType("int"),
                                                                              IsType("string"))
                                                                      })),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 18,
                                .line_end = 1, .column_end = 19
                            }
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {
                                                                          IsType("int"),
                                                                          IsType("vector", IsType("int"),
                                                                              IsType("string"))
                                                                      })),
                        .skip_after_depth_0 = true
                    }
                },
                .accepted_sentinels = SentinelKinds::all()
            });

            reg({
                .name = "TupleMissingClosingParen",
                .code = "(int, string",
                .errors = {
                    PErr{
                        .code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 12, .line_end = 1,
                        .column_end = 13
                    },
                },
                .verifier = IsTupleType(IsType("int"), IsType("string")),
                .skip_contexts = ContextNames::all_nested_swallowing_type_contexts(),
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 25,
                                .line_end = 1, .column_end = 26
                            }
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {
                                                                          IsTupleType(
                                                                              IsType("int"), IsType("string"),
                                                                              IsType("int"), IsType("string"))
                                                                      })),
                        .skip_after_depth_0 = true
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{
                                .code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 20,
                                .line_end = 1, .column_end = 21
                            }
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {
                                                                          IsType("int"),
                                                                          IsTupleType(
                                                                              IsType("int"), IsType("string"),
                                                                              IsType("string"))
                                                                      })),
                        .skip_after_depth_0 = true
                    }
                },
                .accepted_sentinels = SentinelKinds::all()
            });

            return true;
        }();
    }

    TEST_P(TypeAnnotationErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
            accepted_sentinels] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectTypeAnnotationErrors(code, errors, verifier, skip_contexts, context_overrides, excluded_sentinels,
                                   accepted_sentinels);
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAnnotation,
        TypeAnnotationErrorRegistryRunner,
        testing::ValuesIn(ErrorRegistry::type_annotations()),
        TestNameGenerator{}
    );
}
