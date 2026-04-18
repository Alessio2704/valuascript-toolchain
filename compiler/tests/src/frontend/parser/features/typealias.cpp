#include <gtest/gtest.h>

#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class TypealiasSuccessPathTest : public ParserTestBase
    {
    };

    TEST_F(TypealiasSuccessPathTest, Simple)
    {
        ExpectValidParse(
            "typealias Identifier = string",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("Identifier", {},
                                IsType("string")
                    )
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, Generic)
    {
        ExpectValidParse(
            "typealias StringList = vector<string>",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("StringList", {},
                                IsType("vector", {
                                           IsType("string")
                                       }
                                )
                    )
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, NestedGeneric)
    {
        ExpectValidParse(
            "typealias Matrix = map<string, vector<decimal>>",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("Matrix", {},
                                IsType("map", {
                                           IsType("string"),
                                           IsType("vector", {
                                                      IsType("decimal")
                                                  })
                                       }
                                )

                    )
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, Tuple)
    {
        ExpectValidParse(
            "typealias ConfigTuple = (string, integer, bool)",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("ConfigTuple", {},
                                IsTupleType({
                                    IsType("string"),
                                    IsType("integer"),
                                    IsType("bool")
                                }))
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, DeeplyNestedGenerics)
    {
        ExpectValidParse(
            "typealias DeepNesting = Box<Box<Box<scalar>>>",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("DeepNesting", {},
                                IsType(
                                    "Box",
                                    {
                                        IsType(
                                            "Box",
                                            {
                                                IsType("Box",
                                                       {
                                                           IsType("scalar")
                                                       }
                                                )
                                            }
                                        )
                                    }
                                )
                    )
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, TuplesContainingGenerics)
    {
        ExpectValidParse(
            "typealias ExecutionContext = (map<string, any>, vector<string>)",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("ExecutionContext", {},
                                IsTupleType({
                                    IsType("map", {
                                               IsType("string"),
                                               IsType("any"),
                                           }),
                                    IsType("vector", {
                                               IsType("string"),
                                           })
                                })
                    )
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, GenericsContainingTuples)
    {
        ExpectValidParse(
            "typealias Graph = map<string, vector<(integer, integer)>>",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("Graph", {},
                                IsType("map", {
                                           IsType("string"),
                                           IsType("vector", {
                                                      IsTupleType({
                                                          IsType("integer"),
                                                          IsType("integer"),
                                                      })
                                                  })
                                       })
                    )
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, MultipleTypeAliases)
    {
        ExpectValidParse(
            "typealias ID = string\n"
            "typealias Callback = (string, string)\n"
            "typealias Registry = map<ID, Callback>",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("ID", {},
                                IsType("string")
                    ),
                    IsTypeAlias("Callback", {},
                                IsTupleType({
                                    IsType("string"),
                                    IsType("string")
                                })
                    ),
                    IsTypeAlias("Registry", {},
                                IsType("map", {
                                           IsType("ID"),
                                           IsType("Callback")
                                       })
                    ),
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, WithModifiers)
    {
        ExpectValidParse(
            "@export\n"
            "@meta(version: 2)\n"
            "@other @inline(opt: true) typealias PublicState = string",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("PublicState",
                                {
                                    {"export"},
                                    {
                                        "meta", {
                                            {"version", IsNumber("2")}
                                        }
                                    },
                                    {"other"},
                                    {
                                        "inline", {
                                            {"opt", IsBoolean(true)}
                                        }
                                    },
                                },
                                IsType("string")
                    ),
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, MultilineFormatting)
    {
        ExpectValidParse(
            "typealias\n"
            "Data\n "
            "= \n"
            "vector<scalar>\n",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("Data", {},
                                IsType("vector", {
                                           IsType("scalar")
                                       })
                    ),
                }
            }
        );
    }

    TEST_F(TypealiasSuccessPathTest, MultipleModifiersWithStringArg)
    {
        ExpectValidParse(
            "@export @deprecated(arg: \"use V2\") typealias OldStruct = map<string, any>",
            ProgramSpec{
                .type_aliases = {
                    IsTypeAlias("OldStruct", {
                                    {"export"},
                                    {
                                        "deprecated", {
                                            {"arg", IsString("\"use V2\"")}
                                        }
                                    }
                                },
                                IsType("map", {
                                           IsType("string"),
                                           IsType("any"),
                                       })
                    )
                }
            }
        );
    }
}
