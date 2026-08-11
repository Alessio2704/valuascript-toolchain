#include <gtest/gtest.h>
#include "parser_errors_synchronization_base.h"

namespace valuascript::compiler::test
{
    namespace
    {
        void ExpectTensorElements(const Program& ast, const std::vector<std::string>& expected_numbers)
        {
            ASSERT_FALSE(ast.execution_steps.empty()) << "AST has no execution steps.";

            auto* assign = dynamic_cast<Assignment*>(ast.execution_steps.front().get());
            ASSERT_NE(assign, nullptr) << "First statement is not an Assignment.";

            auto* tensor = dynamic_cast<TensorLiteral*>(assign->value.get());
            ASSERT_NE(tensor, nullptr) << "Assignment value is not a TensorLiteral.";

            ASSERT_EQ(tensor->elements.size(), expected_numbers.size()) << "Tensor element count mismatch!";

            for (size_t i = 0; i < expected_numbers.size(); ++i)
            {
                if (!expected_numbers[i].empty())
                {
                    auto* num = dynamic_cast<NumberLiteral*>(tensor->elements[i].get());
                    ASSERT_NE(num, nullptr) << "Expected number literal at index " << i;
                    EXPECT_EQ(num->value, expected_numbers[i]) << "Value mismatch at index " << i;
                }
            }
        }

        auto ExpectTensor(std::vector<std::string> numbers)
        {
            return [n = std::move(numbers)](const Program& ast)
            {
                ExpectTensorElements(ast, n);
                EXPECT_GT(ast.execution_steps.size(), 1) << "Expected recovery statement not found.";
            };
        }
    }

    class TensorLiteralParserSynchronizationTest : public ParserErrorsSynchronizationBase
    {
    };

    TEST_P(TensorLiteralParserSynchronizationTest, SynchronizesTensorLiteralErrors)
    {
        run_parser_and_check_errors(GetParam());
    }

    INSTANTIATE_TEST_SUITE_P(
        TensorLiteralStressTests,
        TensorLiteralParserSynchronizationTest,
        ::testing::Values(
            ParserErrorsSynchronizationTestCase{
                .test_name = "tensor_closed_with_wrong_brace",
                .source_code = "let a =[ 1, 2 \nlet recovery = 1\n",
                .expected_errors = {
                    {.code = Err::UnmatchedBracketAfterTensorElements, .line = 1, .column = 13}
                },
                .verify_ast = ExpectTensor({"1", "2"})
            }
        ),
        TestNameGenerator{}
    );
}
