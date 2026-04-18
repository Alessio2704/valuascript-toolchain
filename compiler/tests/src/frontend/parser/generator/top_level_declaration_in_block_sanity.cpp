#include <gtest/gtest.h>
#include "frontend/parser/helpers/parser_errors_synchronization_base.h"

namespace valuascript::compiler::test {
    class SanityCheckAnchorTest : public ParserErrorsSynchronizationBase {
    };

    TEST_F(SanityCheckAnchorTest, HardcodedMultiLineIllegalTopLevelDeclaration) {
        // Line 1: 123456789...
        std::string source =
                "func main() -> void {\n" // Line 1
                "    @modifier\n" // Line 2, starts at col 5
                "    struct Point {\n" // Line 3
                "        x: int\n" // Line 4
                "    }\n" // Line 5, '}' is at col 5
                "    let a = 1\n"
                "}\n"; // Line 7

        ParserErrorsSynchronizationTestCase test_case{
            "hardcoded_anchor_test",
            source,
            {
                {
                    Err::TopLevelDeclarationNotAllowedHere,
                    2, 5, // Starts exactly at '@' on line 2, col 5
                    5, 6 // Ends exactly after '}' on line 5 (col 5 + 1 length = 6)
                }
            },
            [](const Program &p) {
                ASSERT_EQ(p.function_definitions.size(), 1);
                EXPECT_EQ(p.function_definitions[0]->name, "main");
                EXPECT_EQ(p.function_definitions[0]->body.size(), 1);
                auto assign = dynamic_cast<Assignment *>(p.function_definitions[0]->body.front().get());
                ASSERT_NE(assign, nullptr);
                EXPECT_EQ(assign->modifiers.size(), 0);
                EXPECT_EQ(assign->targets[0].first, "a");
                auto value = dynamic_cast<NumberLiteral *>(assign->value.get());
                EXPECT_EQ(value->value, "1");
            }
        };

        run_parser_and_check_errors(test_case);
    }

    TEST_F(SanityCheckAnchorTest, StressTestNestedSuppressedTopLevelDeclarations) {
        std::string source =
                "func a() -> void {\n" // Line 1
                "    func b() -> void {\n" // Line 2
                "        let a = .\n" // Line 3
                "    }\n" // Line 4
                "\n" // Line 5
                "    @mod1\n" // Line 6
                "    func b() -> void {\n" // Line 7
                "        let a = .\n" // Line 8
                "    }\n" // Line 9
                "\n" // Line 10
                "    struct A {\n" // Line 11
                "        a\n" // Line 12
                "    }\n" // Line 13
                "\n" // Line 14
                "    @mod1\n" // Line 15
                "    struct A {\n" // Line 16
                "        a\n" // Line 17
                "    }\n" // Line 18
                "\n" // Line 19
                "    enum A {\n" // Line 20
                "        .,\n" // Line 21
                "        B\n" // Line 22
                "    }\n" // Line 23
                "\n" // Line 24
                "    @mod1\n" // Line 25
                "    enum A {\n" // Line 26
                "        .,\n" // Line 27
                "        B\n" // Line 28
                "    }\n" // Line 29
                "\n" // Line 30
                "    typealias A = .\n" // Line 31
                "\n" // Line 32
                "    @mod1\n" // Line 33
                "    typealias A = .\n" // Line 34
                "\n" // Line 35
                "    #.\n" // Line 36
                "\n" // Line 37
                "    #dir = .\n" // Line 38
                "\n" // Line 39
                "    import .\n" // Line 40
                "}\n" // Line 41
                "\n" // Line 42
                "let a = (func b() -> void {\n" // Line 43
                "             let a = .\n" // Line 44
                "         })\n" // Line 45
                "\n" // Line 46
                "let a =[@mod1 func b() -> void {\n" // Line 47
                "             let a = .\n" // Line 48
                "         }]\n"; // Line 49

        ParserErrorsSynchronizationTestCase test_case{
            "stress_test_nested_suppressed_top_level_declarations",
            source,
            {
                {Err::TopLevelDeclarationNotAllowedHere, 2, 5, 4, 6},
                {Err::TopLevelDeclarationNotAllowedHere, 6, 5, 9, 6},
                {Err::TopLevelDeclarationNotAllowedHere, 11, 5, 13, 6},
                {Err::TopLevelDeclarationNotAllowedHere, 15, 5, 18, 6},
                {Err::TopLevelDeclarationNotAllowedHere, 20, 5, 23, 6},
                {Err::TopLevelDeclarationNotAllowedHere, 25, 5, 29, 6},
                {Err::TopLevelDeclarationNotAllowedHere, 31, 5, 31, 20},
                {Err::TopLevelDeclarationNotAllowedHere, 33, 5, 34, 20},
                {Err::TopLevelDeclarationNotAllowedHere, 36, 5, 36, 7},
                {Err::TopLevelDeclarationNotAllowedHere, 38, 5, 38, 13},
                {Err::TopLevelDeclarationNotAllowedHere, 40, 5, 40, 13},

                {Err::TopLevelDeclarationNotAllowedHere, 43, 10, 45, 11},
                {Err::TopLevelDeclarationNotAllowedHere, 47, 9, 49, 11}
            },
            [](const Program &p) {
                ASSERT_EQ(p.function_definitions.size(), 1);
                EXPECT_EQ(p.function_definitions[0]->name, "a");

                ASSERT_EQ(p.execution_steps.size(), 2);

                auto assign1 = dynamic_cast<const Assignment *>(p.execution_steps[0].get());
                ASSERT_NE(assign1, nullptr);
                EXPECT_EQ(assign1->targets.size(), 1);
                EXPECT_EQ(assign1->targets[0].first, "a");

                auto assign2 = dynamic_cast<const Assignment *>(p.execution_steps[1].get());
                ASSERT_NE(assign2, nullptr);
                EXPECT_EQ(assign2->targets.size(), 1);
                EXPECT_EQ(assign2->targets[0].first, "a");
            }
        };

        run_parser_and_check_errors(test_case);
    }
}
