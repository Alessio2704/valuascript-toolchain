#include <gtest/gtest.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <tuple>
#include <type_traits>

#include "ast/factory/ast_factory.h"
#include "ast/factory/ast_factory_config.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast_factory_test_reflection.h"

namespace valuascript::compiler::test
{
    struct AstHierarchicalTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstHierarchicalTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    inline void test_node_hierarchical_cascading()
    {
        AstFactoryConfig cfg{};
        cfg.general.expression_kind = ExpressionKind::from<StringLiteral>();
        cfg.general.reassignment_target_kind = ReassignmentTargetKind::from<IdentifierAccess>();
        cfg.string_literal.value.value = "hier_string_val";
        cfg.modifier.name.prefix = "hier_mod";
        cfg.call_argument.name.prefix = "hier_arg";
        cfg.struct_field.name.prefix = "hier_field";
        cfg.enum_case.name.prefix = "hier_case";
        cfg.function_parameter.name.prefix = "hier_param";
        cfg.assignment_target.name.prefix = "hier_target";
        cfg.dict_item.key.prefix = "hier_key";

        auto sample = create_sample<T>(0, cfg);
        const auto& node_ref = unwrap_node(sample);
        EXPECT_TRUE(node_ref.is_valid());

        for_each_ast_member(node_ref, [&](const auto& member)
        {
            using MemberType = std::remove_cvref_t<decltype(member)>;
            if constexpr (is_std_vector_v<MemberType>)
            {
                for (const auto& item : member)
                {
                    if constexpr (requires { item->is_valid(); })
                    {
                        ASSERT_NE(item, nullptr);
                        EXPECT_TRUE(item->is_valid());
                    }
                    else if constexpr (requires { item.is_valid(); })
                    {
                        EXPECT_TRUE(item.is_valid());
                    }
                }
            }
        });
    }

    template <typename T>
    AstHierarchicalTestDescriptor make_ast_hierarchical_test_descriptor()
    {
        return AstHierarchicalTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []()
            {
                test_node_hierarchical_cascading<T>();
            }
        };
    }

    inline std::vector<AstHierarchicalTestDescriptor> get_all_ast_hierarchical_test_descriptors()
    {
        return collect_test_descriptors<AstHierarchicalTestDescriptor, AllAstNodeTypes>([]<typename T>()
        {
            return make_ast_hierarchical_test_descriptor<T>();
        });
    }

    class AstFactoryHierarchicalParameterizedTest : public testing::TestWithParam<AstHierarchicalTestDescriptor>
    {
    };

    TEST_P(AstFactoryHierarchicalParameterizedTest, MetadataDrivenChildCascadingInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Hierarchical Configuration Cascading for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstHierarchicalTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstHierarchicalTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstFactoryHierarchicalParameterizedTest,
        testing::ValuesIn(get_all_ast_hierarchical_test_descriptors()),
        AstHierarchicalTestNameGenerator{}
    );
}
