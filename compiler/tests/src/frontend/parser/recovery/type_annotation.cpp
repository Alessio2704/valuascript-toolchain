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
                          const std::vector<std::string_view>& skip_contexts = {})
            {
                ErrorRegistry::add(n, c, errs, v, skip_contexts);
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

            reg("GenericMissingClosingBracket", "vector<int",
                {
                    {E::UnmatchedBracketAfterGenericArgs, 1, 10, 1, 11},
                },
                IsType("vector", {
                           IsType("int")
                       }
                ),
                {
                    ContextNames::TypeMultiAssignmentTarget1,
                    ContextNames::TypeFunctionMultiReturn,
                    ContextNames::TypeTupleTypeStart,
                    ContextNames::TypeTupleTypeMiddle,
                    ContextNames::TypeTupleTypeEnd,
                    ContextNames::TypeGenericTypeStart,
                    ContextNames::TypeGenericTypeMiddle,
                    ContextNames::TypeGenericTypeEnd,
                }
            );

            return true;
        }();
    }

    TEST_P(TypeAnnotationErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier, skip_contexts] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectTypeAnnotationErrors(code, errors, verifier, skip_contexts);
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
