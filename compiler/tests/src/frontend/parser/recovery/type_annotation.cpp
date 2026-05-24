#include "frontend/parser/helpers/parser_test_base.h"

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
                          const OneOf<TypeVerifier>& v)
            {
                ErrorRegistry::add(n, c, errs, v);
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

            return true;
        }();
    }

    TEST_P(TypeAnnotationErrorRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, errors, verifier] = GetParam();
        SCOPED_TRACE("Running Error Registry Test Case: " + name);

        ExpectTypeAnnotationErrors(code, errors, verifier);
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
