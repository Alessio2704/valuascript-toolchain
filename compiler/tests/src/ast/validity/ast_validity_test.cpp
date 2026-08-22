#include <gtest/gtest.h>
#include <string>

#include "ast_validity_test_factory.h"

namespace valuascript::compiler::test
{
    class AstNodeValidityParameterizedTest : public testing::TestWithParam<AstValidityTestDescriptor>
    {
    };

    TEST_P(AstNodeValidityParameterizedTest, StructuralValidityInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing AST Node Validity for: " + descriptor.type_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstValidityTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstValidityTestDescriptor>& info) const
        {
            return info.param.type_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstNodeValidityParameterizedTest,
        testing::ValuesIn(get_all_ast_validity_test_descriptors()),
        AstValidityTestNameGenerator{}
    );

    class AstLeafValidityParameterizedTest : public testing::TestWithParam<AstValidityTestDescriptor>
    {
    };

    TEST_P(AstLeafValidityParameterizedTest, LeafValueValidityInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing AST Leaf Validity for: " + descriptor.type_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    INSTANTIATE_TEST_SUITE_P(
        AllLeafTypes,
        AstLeafValidityParameterizedTest,
        testing::ValuesIn(get_all_leaf_validity_test_descriptors()),
        AstValidityTestNameGenerator{}
    );
}
