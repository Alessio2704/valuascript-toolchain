#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class TypeAnnotationSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(TypeAnnotationSuccessPathTest, Simple)
    {
        ExpectValidTypeAnnotation("string", IsType("string"));
    }

    TEST_F(TypeAnnotationSuccessPathTest, Generic)
    {
        ExpectValidTypeAnnotation("vector<string>",
                                  IsType("vector", {IsType("string")})
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, NestedGeneric)
    {
        ExpectValidTypeAnnotation("map<string, vector<decimal>>",
                                  IsType("map", {
                                             IsType("string"),
                                             IsType("vector", {IsType("decimal")})
                                         })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, Tuple)
    {
        ExpectValidTypeAnnotation("(string, integer, bool)",
                                  IsTupleType({
                                      IsType("string"),
                                      IsType("integer"),
                                      IsType("bool")
                                  })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, DeeplyNestedGenerics)
    {
        ExpectValidTypeAnnotation("Box<Box<Box<scalar>>>",
                                  IsType("Box", {
                                             IsType("Box", {
                                                        IsType("Box", {IsType("scalar")})
                                                    })
                                         })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, TuplesContainingGenerics)
    {
        ExpectValidTypeAnnotation("(map<string, any>, vector<string>)",
                                  IsTupleType({
                                      IsType("map", {IsType("string"), IsType("any")}),
                                      IsType("vector", {IsType("string")})
                                  })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, GenericsContainingTuples)
    {
        ExpectValidTypeAnnotation("map<string, vector<(integer, integer)>>",
                                  IsType("map", {
                                             IsType("string"),
                                             IsType("vector", {
                                                        IsTupleType({IsType("integer"), IsType("integer")})
                                                    })
                                         })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, DeeplyNestedTuples)
    {
        ExpectValidTypeAnnotation("((int, int), (string, (bool, bool)))",
                                  IsTupleType({
                                      IsTupleType({IsType("int"), IsType("int")}),
                                      IsTupleType({
                                          IsType("string"),
                                          IsTupleType({IsType("bool"), IsType("bool")})
                                      })
                                  })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, HighArityGenerics)
    {
        ExpectValidTypeAnnotation("multi_map<int, string, bool, float, any>",
                                  IsType("multi_map", {
                                             IsType("int"),
                                             IsType("string"),
                                             IsType("bool"),
                                             IsType("float"),
                                             IsType("any")
                                         })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, ExoticTypeIdentifiers)
    {
        ExpectValidTypeAnnotation("_InternalType", IsType("_InternalType"));
        ExpectValidTypeAnnotation("Type42", IsType("Type42"));
        ExpectValidTypeAnnotation("__extreme__", IsType("__extreme__"));
    }

    TEST_F(TypeAnnotationSuccessPathTest, WhitespaceInsensitivity)
    {
        ExpectValidTypeAnnotation("vector  < \n  map  < \n    int  , \n    string \n  > \n >",
                                  IsType("vector", {
                                             IsType("map", {
                                                        IsType("int"),
                                                        IsType("string")
                                                    })
                                         })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, ExtremeHybridNesting)
    {
        ExpectValidTypeAnnotation("map<(string, int), Box<vector<(bool, any)>>>",
                                  IsType("map", {
                                             IsTupleType({IsType("string"), IsType("int")}),
                                             IsType("Box", {
                                                        IsType("vector", {
                                                                   IsTupleType({IsType("bool"), IsType("any")})
                                                               })
                                                    })
                                         })
        );
    }

    TEST_F(TypeAnnotationSuccessPathTest, TripleNestedGenericsWithTuples)
    {
        ExpectValidTypeAnnotation("A<B<C<(int, int)>>>",
                                  IsType("A", {
                                             IsType("B", {
                                                        IsType("C", {
                                                                   IsTupleType({IsType("int"), IsType("int")})
                                                               })
                                                    })
                                         })
        );
    }
}
