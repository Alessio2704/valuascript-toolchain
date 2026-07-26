#include "frontend/parser/helpers/parser_test_base.h"

namespace valuascript::compiler::test
{
    class SanityCheckAnchorTest : public ParserTestBase
    {
    };

    using E = ParserErrorCode;

    TEST_F(SanityCheckAnchorTest, HardcodedMultiLineIllegalTopLevelDeclaration)
    {
        std::string source =
            "func main() -> void {\n"
            "    @modifier\n"
            "    struct Point {\n"
            "        x: int\n"
            "    }\n"
            "    let a = 1\n"
            "}\n";

        ExpectParseErrors(
            source,
            {
                {
                    E::TopLevelDeclarationNotAllowedHere,
                    2, 5,
                    5, 6
                }
            },
            ProgramSpec{
                .functions = {
                    IsFunctionDef("main", {}, {}, {IsType("void")}, {
                                      IsAssignment({{"a"}}, IsNumber("1"))
                                  })
                }
            }
        );
    }

    TEST_F(SanityCheckAnchorTest, StressTestNestedSuppressedTopLevelDeclarations)
    {
        std::string source =
            "func a() -> void {\n"
            "    func b() -> void {\n"
            "        let a = .\n"
            "    }\n"
            "\n"
            "    @mod1\n"
            "    func b() -> void {\n"
            "        let a = .\n"
            "    }\n"
            "\n"
            "    struct A {\n"
            "        a\n"
            "    }\n"
            "\n"
            "    @mod1\n"
            "    struct A {\n"
            "        a\n"
            "    }\n"
            "\n"
            "    enum A {\n"
            "        .,\n"
            "        B\n"
            "    }\n"
            "\n"
            "    @mod1\n"
            "    enum A {\n"
            "        .,\n"
            "        B\n"
            "    }\n"
            "\n"
            "    typealias A = .\n"
            "\n"
            "    @mod1\n"
            "    typealias A = .\n"
            "\n"
            "    #.\n"
            "\n"
            "    #dir = .\n"
            "\n"
            "    import .\n"
            "}\n"
            "\n"
            "let a = (func b() -> void {\n"
            "             let a = .\n"
            "         })\n"
            "\n"
            "let a =[@mod1 func b() -> void {\n"
            "             let a = .\n"
            "         }]\n";

        ExpectParseErrors(
            source,
            {
                {E::TopLevelDeclarationNotAllowedHere, 2, 5, 4, 6},
                {E::TopLevelDeclarationNotAllowedHere, 6, 5, 9, 6},
                {E::TopLevelDeclarationNotAllowedHere, 11, 5, 13, 6},
                {E::TopLevelDeclarationNotAllowedHere, 15, 5, 18, 6},
                {E::TopLevelDeclarationNotAllowedHere, 20, 5, 23, 6},
                {E::TopLevelDeclarationNotAllowedHere, 25, 5, 29, 6},
                {E::TopLevelDeclarationNotAllowedHere, 31, 5, 31, 20},
                {E::TopLevelDeclarationNotAllowedHere, 33, 5, 34, 20},
                {E::TopLevelDeclarationNotAllowedHere, 36, 5, 36, 7},
                {E::TopLevelDeclarationNotAllowedHere, 38, 5, 38, 13},
                {E::TopLevelDeclarationNotAllowedHere, 40, 5, 40, 13},

                {E::TopLevelDeclarationNotAllowedHere, 43, 10, 45, 11},
                {E::TopLevelDeclarationNotAllowedHere, 47, 9, 49, 11}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({{"a"}}),
                    IsAssignment({{"a"}})
                },
                .functions = {
                    IsFunctionDef("a", {}, {}, {IsType("void")})
                }
            }
        );
    }
}
