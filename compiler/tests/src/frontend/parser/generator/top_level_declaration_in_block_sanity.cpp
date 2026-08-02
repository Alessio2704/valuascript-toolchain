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
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 2, .column_start = 5, .line_end = 5, .column_end = 6}
            },
            ProgramSpec{
                .functions = {
                    IsFunctionDef("main", {}, {}, {IsType("void")}, {
                                      IsAssignment({AssignmentTargetSpec{.name = "a"}}, IsNumber("1"))
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
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 2, .column_start = 5, .line_end = 4, .column_end = 6},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 6, .column_start = 5, .line_end = 9, .column_end = 6},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 11, .column_start = 5, .line_end = 13, .column_end = 6},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 15, .column_start = 5, .line_end = 18, .column_end = 6},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 20, .column_start = 5, .line_end = 23, .column_end = 6},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 25, .column_start = 5, .line_end = 29, .column_end = 6},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 31, .column_start = 5, .line_end = 31, .column_end = 20},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 33, .column_start = 5, .line_end = 34, .column_end = 20},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 36, .column_start = 5, .line_end = 36, .column_end = 7},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 38, .column_start = 5, .line_end = 38, .column_end = 13},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 40, .column_start = 5, .line_end = 40, .column_end = 13},

                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 43, .column_start = 10, .line_end = 45, .column_end = 11},
                PErr{.code = E::TopLevelDeclarationNotAllowedHere, .line_start = 47, .column_start = 9, .line_end = 49, .column_end = 11}
            },
            ProgramSpec{
                .execution_steps = {
                    IsAssignment({AssignmentTargetSpec{.name = "a"}}),
                    IsAssignment({AssignmentTargetSpec{.name = "a"}})
                },
                .functions = {
                    IsFunctionDef("a", {}, {}, {IsType("void")})
                }
            }
        );
    }
}
