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
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 4}},
                .verifier = IsNullType()
            });

            reg({
                .name = "TupleTypeMissingElement",
                .code = "(int, , string)",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = IsTupleType(
                    IsType("int"),
                    IsNullType(),
                    IsType("string")
                )
            });

            reg({
                .name = "TupleTypeBrokenElement",
                .code = "(int, *, string)",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}},
                .verifier = IsTupleType(
                    IsType("int"),
                    IsNullType(),
                    IsType("string")
                )
            });

            reg({
                .name = "TupleTypeSingleElementDisallowed",
                .code = "(int, )",
                .errors = {PErr{.code = E::TrailingCommaInTuple, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}},
                .verifier = IsTupleType(
                    IsType("int")
                )
            });

            reg({
                .name = "TupleTrailingCommaDisallowed",
                .code = "(int, string, )",
                .errors = {PErr{.code = E::TrailingCommaInTuple, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}},
                .verifier = IsTupleType(
                    IsType("int"),
                    IsType("string")
                )
            });

            reg({
                .name = "EmptyGeneric",
                .code = "map<>",
                .errors = {PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}},
                .verifier = IsType("map")
            });

            reg({
                .name = "GenericMissingTypeAfterComma",
                .code = "map<string, >",
                .errors = {PErr{.code = E::TrailingCommaInGenericArgument, .line_start = 1, .column_start = 11, .line_end = 1, .column_end = 12}},
                .verifier = IsType("map", IsType("string"))
            });

            reg({
                .name = "GenericGarbageArgument",
                .code = "map<string, *, int>",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}},
                .verifier = IsType("map",
                    IsType("string"),
                    IsNullType(),
                    IsType("int")
                )
            });

            reg({
                .name = "GenericEmptyArgument",
                .code = "map<string, , int>",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}},
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
                    PErr{.code = E::ExpectedCommaSeparatorInGenericArgs, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 15},
                    PErr{.code = E::ExpectedCommaSeparatorInGenericArgs, .line_start = 1, .column_start = 16, .line_end = 1, .column_end = 22}
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
                    PErr{.code = E::ExpectedCommaSeparatorInTupleType, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 12},
                    PErr{.code = E::ExpectedCommaSeparatorInTupleType, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 19}
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
                    PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                    PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8},
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
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget2,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeExtensionTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturn,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturnEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter1,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeStructField,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeEnumUnderlyingType,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    }
                }
            });

            reg({
                .name = "GenericMissingClosingBracket",
                .code = "vector<int",
                .errors = {
                    PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11},
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
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 19}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {AssignmentTargetSpec{.name = "ctx_m1", .type_v = IsType("vector", IsType("int"), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 15, .line_end = 1, .column_end = 16}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsType("vector", IsType("int"), IsType("int"))}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 15, .line_end = 1, .column_end = 16}
                        },
                        .verifier = IsTupleType(IsType("vector", IsType("int"), IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 19}
                        },
                        .verifier = IsTupleType(IsType("int"), IsType("vector", IsType("int"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 16, .line_end = 1, .column_end = 17}
                        },
                        .verifier = IsType("vector", IsType("vector", IsType("int"), IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 16, .line_end = 1, .column_end = 17}
                        },
                        .verifier = IsType("vector", IsType("int"), IsType("vector", IsType("int"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = IsType("vector", IsType("int"), IsType("string"), IsType("vector", IsType("int")))
                    }
                }
            });

            reg({
                .name = "TupleMissingClosingParen",
                .code = "(int, string",
                .errors = {
                    PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13},
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
                            PErr{.code = E::ExpectedRightParenAfterParameters, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter2,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterParameters, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = IsTupleType(IsType("int"), IsType("string"), IsTupleType(IsType("int"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 20, .line_end = 1, .column_end = 21}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {AssignmentTargetSpec{.name = "ctx_m1", .type_v = IsTupleType(IsType("int"), IsType("string"), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 17, .line_end = 1, .column_end = 18}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsTupleType(IsType("int"), IsType("string"), IsType("int"))}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 17, .line_end = 1, .column_end = 18}
                        },
                        .verifier = IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTupleTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 20, .line_end = 1, .column_end = 21}
                        },
                        .verifier = IsTupleType(IsType("int"), IsTupleType(IsType("int"), IsType("string"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 19}
                        },
                        .verifier = IsType("vector", IsTupleType(IsType("int"), IsType("string"), IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 18, .line_end = 1, .column_end = 19}
                        },
                        .verifier = IsType("vector", IsType("int"), IsTupleType(IsType("int"), IsType("string"), IsType("string")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeGenericTypeEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = IsType("vector", IsType("string"), IsType("string"), IsTupleType(IsType("int"), IsType("string")))
                    }
                }
            });

            reg({
                .name = "TupleMissingClosingParenNestedInGeneric",
                .code = "vector<(int, string>",
                .errors = {
                    PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 19, .line_end = 1, .column_end = 20},
                    PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 20, .line_end = 1, .column_end = 21},
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
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 19, .line_end = 1, .column_end = 20},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 28, .line_end = 1, .column_end = 29}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {AssignmentTargetSpec{.name = "ctx_m1", .type_v = IsType("vector", IsTupleType(IsType("int"), IsType("string")), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 19, .line_end = 1, .column_end = 20},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 25, .line_end = 1, .column_end = 26}
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
                    PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 21, .line_end = 1, .column_end = 22},
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
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 29, .line_end = 1, .column_end = 30}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {AssignmentTargetSpec{.name = "ctx_m1", .type_v = IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("float")), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("float")), IsType("int"))}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParameter,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterParameters, .line_start = 1, .column_start = 22, .line_end = 1, .column_end = 23}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiParameter2,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterParameters, .line_start = 1, .column_start = 22, .line_end = 1, .column_end = 23}
                        }
                    }
                }
            });

            reg({
                .name = "GenericMissingClosingBracketNestedInTuple",
                .code = "(vector<int, string)",
                .errors = {
                    PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 19, .line_end = 1, .column_end = 20},
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
                    PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 23, .line_end = 1, .column_end = 24},
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
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 31, .line_end = 1, .column_end = 32}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment(
                            {AssignmentTargetSpec{.name = "ctx_m1", .type_v = IsType("vector", IsType("map", IsType("int"), IsType("string")), IsType("ctx_m2"))}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionMultiReturn,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 28, .line_end = 1, .column_end = 29}
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
