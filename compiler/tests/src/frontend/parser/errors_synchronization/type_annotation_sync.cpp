#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        void ExpectBaseType(const TypeAnnotation* type, const std::string& expected_name,
                            size_t expected_generics = 0)
        {
            ASSERT_NE(type, nullptr) << "Expected a TypeAnnotation, but got nullptr";
            EXPECT_EQ(type->name, expected_name);
            EXPECT_EQ(type->generic_args.size(), expected_generics);
        }

        void ExpectTupleType(const TypeAnnotation* type, size_t expected_elements)
        {
            ASSERT_NE(type, nullptr) << "Expected a TypeAnnotation, but got nullptr";
            EXPECT_EQ(type->name, "tuple");
            auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
            ASSERT_NE(tuple_type, nullptr) << "TypeAnnotation is not a TupleTypeAnnotation";
            EXPECT_EQ(tuple_type->element_types.size(), expected_elements);
        }

        auto ExpectAssignmentType(std::function<void(const TypeAnnotation*)> verifier)
        {
            return [v = std::move(verifier)](const Program& ast)
            {
                ASSERT_FALSE(ast.execution_steps.empty()) << "Expected at least one execution step";
                auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                ASSERT_NE(assign, nullptr) << "Expected first step to be an Assignment";
                ASSERT_FALSE(assign->targets.empty()) << "Expected at least one assignment target";
                v(assign->targets[0].type.get());
            };
        }
    }

    class TypeAnnotationParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(TypeAnnotationParserSynchronizationTest, CollectsMultipleSyntaxErrorsAtCorrectLocations)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TypeAnnotationStressTests,
        TypeAnnotationParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "generic_closed_with_wrong_bracket",
                .source_code = "let a: vector<int} = 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedBracketAfterGenericArgs, .line = 1, .column = 17}
                },
                .verify_ast = ExpectAssignmentType([](const TypeAnnotation* type) {
                    ExpectBaseType(type, "vector", 1);
                    ExpectBaseType(type->generic_args[0].get(), "int", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "tuple_type_closed_with_wrong_bracket",
                .source_code = "let a: (int, string] = 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedParenthesisInTuple, .line = 1, .column = 19}
                },
                .verify_ast = ExpectAssignmentType([](const TypeAnnotation* type) {
                    ExpectTupleType(type, 2);
                    auto tuple_type = dynamic_cast<const TupleTypeAnnotation*>(type);
                    ExpectBaseType(tuple_type->element_types[0].get(), "int", 0);
                    ExpectBaseType(tuple_type->element_types[1].get(), "string", 0);
                })
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "missing_base_type_before_generic_discards_assignment",
                .source_code = "let a: <int> = 1\nlet b = 2\n",
                .expected_errors = {
                    {.code = Err::MissingTypeAnnotation, .line = 1, .column = 8}
                },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.execution_steps.size(), 2);
                    auto assign = dynamic_cast<Assignment*>(ast.execution_steps[0].get());
                    ASSERT_NE(assign, nullptr);
                    EXPECT_EQ(assign->targets[0].name, "a");
                    EXPECT_EQ(assign->targets[0].type.get(), nullptr);
                }
            },
            ParserErrorsSynchronizationTestCase{
                .test_name = "multiple_return_types_trailing_comma_error",
                .source_code = "func f() -> int, string, {\n  return 1\n}\nlet a = 1\n",
                .expected_errors = { {.code = Err::TrailingComma, .line = 1, .column = 24} },
                .verify_ast = [](const Program& ast) {
                    ASSERT_EQ(ast.function_definitions.size(), 1);
                    EXPECT_EQ(ast.function_definitions[0]->return_types.size(), 2);
                }
            }
        ),
        TestNameGenerator{}
    );
}
