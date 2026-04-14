#include <gtest/gtest.h>
#include "stages/frontend/parser/parser_errors_synchronization_base.h"
#include "stages/frontend/parser/language_constructs_provider.h"

namespace valuascript::compiler::test {
    namespace {
        std::vector<ParserErrorsSynchronizationTestCase> GenerateTopLevelInBlockTests() {
            std::vector<ParserErrorsSynchronizationTestCase> test_cases;
            auto constructs = LanguageConstructsProvider::build_all_test_variants();

            for (const auto &construct: constructs) {
                if (!construct.is_top_level_only) continue;

                std::string wrapper_start = "func wrapper() -> void {\n";
                std::string test_name = "illegal_top_level_" + construct.name + "_inside_function";
                std::string source = wrapper_start + construct.source + "let a = 1" + "}\n";

                auto span = calculate_expected_span(construct.source, 2, 1);

                test_cases.push_back({
                    test_name,
                    source,
                    {
                        {
                            Err::TopLevelDeclarationNotAllowedHere,
                            span.line_start,
                            span.column_start,
                            span.line_end,
                            span.column_end
                        }
                    },
                    [](const Program &p) {
                        ASSERT_EQ(p.function_definitions.size(), 1);
                        EXPECT_EQ(p.function_definitions[0]->name, "wrapper");
                        EXPECT_EQ(p.function_definitions[0]->body.size(), 1);
                        auto assign = dynamic_cast<Assignment *>(p.function_definitions[0]->body.front().get());
                        EXPECT_EQ(assign->modifiers.size(), 0);
                        EXPECT_EQ(assign->targets[0].first, "a");
                        auto value = dynamic_cast<NumberLiteral *>(assign->value.get());
                        EXPECT_EQ(value->value, "1");
                    }
                });
            }
            return test_cases;
        }
    }

    class TopLevelDeclarationInBlockTest : public ParserErrorsSynchronizationBase {
    };

    TEST_P(TopLevelDeclarationInBlockTest, RaisesErrorWithCorrectFullSpan) {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(ParserScope, TopLevelDeclarationInBlockTest,
                             ::testing::ValuesIn(GenerateTopLevelInBlockTests()),
                             [](const auto& info) { return info.param.test_name; });

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
                "    let a = 1"
                "}\n"; // Line 6

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
                EXPECT_EQ(assign->modifiers.size(), 0);
                EXPECT_EQ(assign->targets[0].first, "a");
                auto value = dynamic_cast<NumberLiteral *>(assign->value.get());
                EXPECT_EQ(value->value, "1");
            }
        };

        run_parser_and_check_errors(test_case);
    }
}
