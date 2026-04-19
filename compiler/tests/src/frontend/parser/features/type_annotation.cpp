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
        static const bool _ = []()
        {
            auto reg = [](auto n, auto c, auto v) { ConstructRegistry::add(n, c, v); };

            reg("Simple",
                "string",
                IsType("string"));

            reg("Generic",
                "vector<string>",
                IsType("vector", {IsType("string")}));

            reg("NestedGeneric",
                "map<string, vector<decimal>>",
                IsType("map", {
                           IsType("string"),
                           IsType("vector", {IsType("decimal")})
                       }));

            reg("Tuple",
                "(string, integer, bool)",
                IsTupleType({
                    IsType("string"),
                    IsType("integer"),
                    IsType("bool")
                }));

            reg("DeeplyNestedGenerics",
                "Box<Box<Box<scalar>>>",
                IsType("Box", {
                           IsType("Box", {
                                      IsType("Box", {IsType("scalar")})
                                  })
                       }));

            reg("TuplesContainingGenerics",
                "(map<string, any>, vector<string>)",
                IsTupleType({
                    IsType("map", {IsType("string"), IsType("any")}),
                    IsType("vector", {IsType("string")})
                }));

            reg("GenericsContainingTuples",
                "map<string, vector<(integer, integer)>>",
                IsType("map", {
                           IsType("string"),
                           IsType("vector", {
                                      IsTupleType({IsType("integer"), IsType("integer")})
                                  })
                       }));

            reg("DeeplyNestedTuples",
                "((int, int), (string, (bool, bool)))",
                IsTupleType({
                    IsTupleType({IsType("int"), IsType("int")}),
                    IsTupleType({
                        IsType("string"),
                        IsTupleType({IsType("bool"), IsType("bool")})
                    })
                }));

            reg("HighArityGenerics",
                "multi_map<int, string, bool, float, any>",
                IsType("multi_map", {
                           IsType("int"),
                           IsType("string"),
                           IsType("bool"),
                           IsType("float"),
                           IsType("any")
                       }));

            reg("ExoticTypeIdentifiers_InternalType",
                "_InternalType",
                IsType("_InternalType"));

            reg("ExoticTypeIdentifiers_Type42",
                "Type42",
                IsType("Type42"));

            reg("ExoticTypeIdentifiers_Extreme",
                "__extreme__",
                IsType("__extreme__"));

            reg("WhitespaceInsensitivity",
                "vector  < \n  map  < \n    int  , \n    string \n  > \n >",
                IsType("vector", {
                           IsType("map", {
                                      IsType("int"),
                                      IsType("string")
                                  })
                       }));

            reg("ExtremeHybridNesting",
                "map<(string, int), Box<vector<(bool, any)>>>",
                IsType("map", {
                           IsTupleType({IsType("string"), IsType("int")}),
                           IsType("Box", {
                                      IsType("vector", {
                                                 IsTupleType({IsType("bool"), IsType("any")})
                                             })
                                  })
                       }));

            reg("TripleNestedGenericsWithTuples",
                "A<B<C<(int, int)>>>",
                IsType("A", {
                           IsType("B", {
                                      IsType("C", {
                                                 IsTupleType({IsType("int"), IsType("int")})
                                             })
                                  })
                       }));

            return true;
        }();
    }

    TEST_P(TypeAnnotationRegistryRunner, ValidatesInAllContexts)
    {
        const auto& [name, code, verifier] = GetParam();
        SCOPED_TRACE("Running Registry Test Case: " + name);

        ExpectValidTypeAnnotation(code, verifier);
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAnnotation,
        TypeAnnotationRegistryRunner,
        testing::ValuesIn(ConstructRegistry::type_annotations()),
        [](const testing::TestParamInfo<RegistryEntry<TypeVerifier>>& info) {
        return info.param.test_name;
        }
    );
}
