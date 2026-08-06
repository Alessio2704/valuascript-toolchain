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
                .name = "MissingBaseTypeBeforeGeneric",
                .code = "<int>",
                .errors = {PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 2}},
                .verifier = IsNullType(),
                .skip_contexts = {
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTypealiasTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 1, .line_end = 1, .column_end = 2},
                            PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 5, .line_end = 1, .column_end = 6}
                        },
                        .verifier = IsNullType()
                    }
                }
            });

            reg({
                .name = "MultipleReturnTypesTrailingCommaError",
                .code = "int, string,",
                .errors = {PErr{.code = E::TrailingComma, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}},
                .verifier = IsTupleType(IsType("int"), IsType("string")),
                .skip_contexts = {
                    ContextNames::TypeEnumUnderlyingType,
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeAssignmentTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedIdentifier, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment({AssignmentTargetSpec{.name = "ctx_assign", .type_v = IsType("int")}, AssignmentTargetSpec{.name = "string"}, AssignmentTargetSpec{.name = "<error>"}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget1,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedIdentifier, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment({AssignmentTargetSpec{.name = "ctx_m1", .type_v = IsType("int")}, AssignmentTargetSpec{.name = "string"}, AssignmentTargetSpec{.name = "<error>"}, AssignmentTargetSpec{.name = "ctx_m2"}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeMultiAssignmentTarget2,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedIdentifier, .line_start = 1, .column_start = 14, .line_end = 1, .column_end = 15}
                        },
                        .verifier = OneOf<TypeVerifier>(IsAssignment({AssignmentTargetSpec{.name = "ctx_m1"}, AssignmentTargetSpec{.name = "ctx_m2", .type_v = IsType("int")}, AssignmentTargetSpec{.name = "string"}, AssignmentTargetSpec{.name = "<error>"}}, IsNumber("1")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeTypealiasTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::InvalidExpression, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5}
                        },
                        .verifier = OneOf<TypeVerifier>(IsTypeAlias("ctx_alias", {}, IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeExtensionTarget,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedLeftBraceBeforeExtensionBody, .line_start = 1, .column_start = 4, .line_end = 1, .column_end = 5}
                        },
                        .verifier = OneOf<TypeVerifier>(IsExtensionDef({}, IsType("int")))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingColonAfterParameter, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13},
                            PErr{.code = E::MissingParameterName, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec{.name = "p1", .type_v = IsType("int")}, ParamSpec{.name = "string"}, ParamSpec{.name = "<error>"}, ParamSpec{.name = "p2", .type_v = IsType("int")}, ParamSpec{.name = "p3", .type_v = IsType("string")}}, {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingColonAfterParameter, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13},
                            PErr{.code = E::MissingParameterName, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec{.name = "p1", .type_v = IsType("int")}, ParamSpec{.name = "p2", .type_v = IsType("int")}, ParamSpec{.name = "string"}, ParamSpec{.name = "<error>"}, ParamSpec{.name = "p3", .type_v = IsType("string")}}, {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingColonAfterParameter, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13},
                            PErr{.code = E::TrailingComma, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec{.name = "p", .type_v = IsType("int")}, ParamSpec{.name = "string"}}, {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingColonAfterParameter, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13},
                            PErr{.code = E::TrailingComma, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_param", {}, {ParamSpec{.name = "p1", .type_v = IsType("int")}, ParamSpec{.name = "p2", .type_v = IsType("string")}, ParamSpec{.name = "p3", .type_v = IsType("int")}, ParamSpec{.name = "string"}}, {IsType("void")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {IsType("int"), IsType("string"), IsNullType(), IsType("int"), IsType("string")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnMiddle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::MissingTypeAnnotation, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {IsType("int"), IsType("int"), IsType("string"), IsNullType(), IsType("string")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeStructField,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedColonAfterStructFieldName, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = OneOf<TypeVerifier>(IsStructDef("ctx_struct", {}, FieldSpec{.name = "f", .type_v = IsType("int")}, FieldSpec{.name = "string"}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeStructMultipleFields,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedColonAfterStructFieldName, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13},
                            PErr{.code = E::ExpectedStructFieldName, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        },
                        .verifier = OneOf<TypeVerifier>(IsStructDef("ctx_struct", {}, FieldSpec{.name = "f1", .type_v = IsType("int")}, FieldSpec{.name = "string"}, FieldSpec{.name = "<error>"}, FieldSpec{.name = "f2", .type_v = IsType("int")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::TrailingComma, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {IsType("int"), IsType("string")}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::TrailingComma, .line_start = 1, .column_start = 12, .line_end = 1, .column_end = 13}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {IsType("int"), IsType("string"), IsType("int"), IsType("string")}))
                    }
                }
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
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle
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
                        .context_name = ContextNames::TypeFunctionReturnSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 8, .line_end = 1, .column_end = 9},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionReturnEnd,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::EmptyGenericTypeAnnotation, .line_start = 1, .column_start = 9, .line_end = 1, .column_end = 10},
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 7, .line_end = 1, .column_end = 8}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamStart,
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
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle,
                    ContextNames::TypeFunctionReturnStart,
                    ContextNames::TypeFunctionReturnMiddle,
                    ContextNames::TypeFunctionReturnEnd
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
                        .context_name = ContextNames::TypeFunctionReturnSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedBracketAfterGenericArgs, .line_start = 1, .column_start = 10, .line_end = 1, .column_end = 11}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_ret", {}, {},
                                                                      {IsType("vector", IsType("int"))}))
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
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle,
                    ContextNames::TypeFunctionParamStart,
                    ContextNames::TypeFunctionParamMiddle,
                    ContextNames::TypeFunctionParamEnd,
                    ContextNames::TypeFunctionReturnStart,
                    ContextNames::TypeFunctionReturnMiddle,
                    ContextNames::TypeFunctionReturnEnd
                },
                .context_overrides = {
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterParameters, .line_start = 1, .column_start = 13, .line_end = 1, .column_end = 14}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamMiddle,
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
                        .context_name = ContextNames::TypeFunctionReturnStart,
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
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle,
                    ContextNames::TypeFunctionReturnStart,
                    ContextNames::TypeFunctionReturnMiddle,
                    ContextNames::TypeFunctionReturnEnd
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
                        .context_name = ContextNames::TypeFunctionReturnStart,
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
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle,
                    ContextNames::TypeFunctionParamStart,
                    ContextNames::TypeFunctionParamMiddle,
                    ContextNames::TypeFunctionParamEnd,
                    ContextNames::TypeFunctionReturnStart,
                    ContextNames::TypeFunctionReturnMiddle,
                    ContextNames::TypeFunctionReturnEnd
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
                        .context_name = ContextNames::TypeFunctionReturnStart,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::UnmatchedParenthesisInTuple, .line_start = 1, .column_start = 26, .line_end = 1, .column_end = 27}
                        },
                        .verifier = OneOf<TypeVerifier>(IsFunctionDef("ctx_func_multi_ret", {}, {},
                                                                      {IsTupleType(IsTupleType(IsType("int"), IsType("string"), IsType("float")), IsType("int"))}))
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamSingle,
                        .errors = std::vector<ParserExpectedError>{
                            PErr{.code = E::ExpectedRightParenAfterParameters, .line_start = 1, .column_start = 22, .line_end = 1, .column_end = 23}
                        }
                    },
                    ContextOverride<TypeVerifier>{
                        .context_name = ContextNames::TypeFunctionParamMiddle,
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
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle
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
                    ContextNames::TypeTupleTypeSingle,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                    ContextNames::TypeGenericTypeSingle,
                    ContextNames::TypeFunctionReturnStart,
                    ContextNames::TypeFunctionReturnMiddle,
                    ContextNames::TypeFunctionReturnEnd
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
                        .context_name = ContextNames::TypeFunctionReturnStart,
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
        TestNameGenerator{}
    );
}
