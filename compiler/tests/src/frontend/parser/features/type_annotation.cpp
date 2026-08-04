#include "frontend/parser/helpers/parser_test_base.h"
#include "frontend/parser/helpers/construct_registry.h"

namespace valuascript::compiler::test
{
    class TypeAnnotationRegistryRunner : public ParserTestBase,
                                         public testing::WithParamInterface<RegistryEntry<TypeVerifier>>
    {
    };

    namespace
    {
        const bool _ = []()
        {
            auto reg = [](const ConstructCase<TypeVerifier>& spec) { ConstructRegistry::add(spec); };

            reg({
                .name = "Simple",
                .code = "string",
                .verifier = IsType("string")
            });

            reg({
                .name = "Generic",
                .code = "vector<string>",
                .verifier = IsType("vector", IsType("string"))
            });

            reg({
                .name = "NestedGeneric",
                .code = "map<string, vector<decimal>>",
                .verifier = IsType("map",
                    IsType("string"),
                    IsType("vector", IsType("decimal"))
                )
            });

            reg({
                .name = "Tuple",
                .code = "(string, integer, bool)",
                .verifier = IsTupleType(
                    IsType("string"),
                    IsType("integer"),
                    IsType("bool")
                )
            });

            reg({
                .name = "DeeplyNestedGenerics",
                .code = "Box<Box<Box<scalar>>>",
                .verifier = IsType("Box",
                    IsType("Box",
                        IsType("Box", IsType("scalar"))
                    )
                )
            });

            reg({
                .name = "TuplesContainingGenerics",
                .code = "(map<string, any>, vector<string>)",
                .verifier = IsTupleType(
                    IsType("map", IsType("string"), IsType("any")),
                    IsType("vector", IsType("string"))
                )
            });

            reg({
                .name = "GenericsContainingTuples",
                .code = "map<string, vector<(integer, integer)>>",
                .verifier = IsType("map",
                    IsType("string"),
                    IsType("vector",
                        IsTupleType(IsType("integer"), IsType("integer"))
                    )
                )
            });

            reg({
                .name = "DeeplyNestedTuples",
                .code = "((int, int), (string, (bool, bool)))",
                .verifier = IsTupleType(
                    IsTupleType(IsType("int"), IsType("int")),
                    IsTupleType(
                        IsType("string"),
                        IsTupleType(IsType("bool"), IsType("bool"))
                    )
                )
            });

            reg({
                .name = "HighArityGenerics",
                .code = "multi_map<int, string, bool, float, any>",
                .verifier = IsType("multi_map",
                    IsType("int"),
                    IsType("string"),
                    IsType("bool"),
                    IsType("float"),
                    IsType("any")
                )
            });

            reg({
                .name = "ExoticTypeIdentifiers_InternalType",
                .code = "_InternalType",
                .verifier = IsType("_InternalType")
            });

            reg({
                .name = "ExoticTypeIdentifiers_Type42",
                .code = "Type42",
                .verifier = IsType("Type42")
            });

            reg({
                .name = "ExoticTypeIdentifiers_Extreme",
                .code = "__extreme__",
                .verifier = IsType("__extreme__")
            });

            reg({
                .name = "WhitespaceInsensitivity",
                .code = "vector  < \n  map  < \n    int  , \n    string \n  > \n >",
                .verifier = IsType("vector",
                    IsType("map",
                        IsType("int"),
                        IsType("string")
                    )
                )
            });

            reg({
                .name = "ExtremeHybridNesting",
                .code = "map<(string, int), Box<vector<(bool, any)>>>",
                .verifier = IsType("map",
                    IsTupleType(IsType("string"), IsType("int")),
                    IsType("Box",
                        IsType("vector",
                            IsTupleType(IsType("bool"), IsType("any"))
                        )
                    )
                )
            });

            reg({
                .name = "TripleNestedGenericsWithTuples",
                .code = "A<B<C<(int, int)>>>",
                .verifier = IsType("A",
                    IsType("B",
                        IsType("C",
                            IsTupleType(IsType("int"), IsType("int"))
                        )
                    )
                )
            });

            return true;
        }();
    }

    TEST_P(TypeAnnotationRegistryRunner, ValidatesInAllContexts)
    {
        const auto& entry = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + entry.test_name);

        ExpectValidTypeAnnotation(entry.code, entry.verifier, entry.skip_contexts);
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAnnotation,
        TypeAnnotationRegistryRunner,
        testing::ValuesIn(ConstructRegistry::type_annotations()),
        TestNameGenerator{}
    );
}
