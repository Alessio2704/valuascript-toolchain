#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <ostream>

#include "ast_clone_test_helper.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler::test
{
    struct AstCloneTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstCloneTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    AstCloneTestDescriptor make_ast_clone_test_descriptor()
    {
        return AstCloneTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []() {
                AstCloneSampleFactory<T>::run_full_clone_test();
            }
        };
    }

    template <typename Tuple>
    struct AstCloneTestDescriptorCollector;

    template <typename... Types>
    struct AstCloneTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstCloneTestDescriptor> collect()
        {
            return { make_ast_clone_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstCloneTestDescriptor> get_all_ast_clone_test_descriptors()
    {
        return AstCloneTestDescriptorCollector<AllAstNodeTypes>::collect();
    }

    class AstCloneTest : public testing::TestWithParam<AstCloneTestDescriptor>
    {
    };

    TEST_P(AstCloneTest, DeepCloneAndInvariants)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing AST Node Clone Engine for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstCloneTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstCloneTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstCloneTest,
        testing::ValuesIn(get_all_ast_clone_test_descriptors()),
        AstCloneTestNameGenerator{}
    );
}
