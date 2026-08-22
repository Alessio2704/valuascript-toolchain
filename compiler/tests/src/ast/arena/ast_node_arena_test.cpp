#include <gtest/gtest.h>
#include <memory>
#include <memory_resource>
#include <string>
#include <vector>
#include <functional>
#include <ostream>
#include <cstdint>

#include "ast/core/ast_core.h"
#include "ast/core/ast_node_registry.h"
#include "ast/arena/ast_arena.h"
#include "ast/sample/ast_sample_factory.h"

namespace valuascript::compiler::test
{
    template <typename T>
    void test_node_allocated_in_arena()
    {
        alignas(std::max_align_t) char arena_buffer[256 * 1024];
        std::pmr::monotonic_buffer_resource fixed_arena(
            arena_buffer, sizeof(arena_buffer), std::pmr::null_memory_resource()
        );

        auto* prev = AstArena::current();
        AstArena::set_current(&fixed_arena);

        std::unique_ptr<T> node = nullptr;
        if constexpr (std::same_as<decltype(create_sample<T>()), std::unique_ptr<T>>)
        {
            node = create_sample<T>();
        }
        else
        {
            node = std::make_unique<T>(create_sample<T>());
        }

        ASSERT_NE(node, nullptr);
        EXPECT_TRUE(node->is_valid());

        uintptr_t node_addr = reinterpret_cast<uintptr_t>(node.get());
        uintptr_t buf_start = reinterpret_cast<uintptr_t>(arena_buffer);
        uintptr_t buf_end = buf_start + sizeof(arena_buffer);

        EXPECT_GE(node_addr, buf_start) << "Node " << get_ast_node_name<T>() << " address is below arena buffer bounds!";
        EXPECT_LT(node_addr, buf_end) << "Node " << get_ast_node_name<T>() << " address is above arena buffer bounds!";

        node.reset();
        AstArena::set_current(prev);
    }

    struct AstNodeArenaTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstNodeArenaTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    AstNodeArenaTestDescriptor make_ast_node_arena_test_descriptor()
    {
        return AstNodeArenaTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []()
            {
                test_node_allocated_in_arena<T>();
            }
        };
    }

    template <typename Tuple>
    struct AstNodeArenaTestDescriptorCollector;

    template <typename... Types>
    struct AstNodeArenaTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstNodeArenaTestDescriptor> collect()
        {
            return { make_ast_node_arena_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstNodeArenaTestDescriptor> get_all_ast_node_arena_test_descriptors()
    {
        return AstNodeArenaTestDescriptorCollector<AllAstNodeTypes>::collect();
    }

    class AstNodeArenaParameterizedTest : public testing::TestWithParam<AstNodeArenaTestDescriptor>
    {
    };

    TEST_P(AstNodeArenaParameterizedTest, NodeAllocationLandsInsideArenaBuffer)
    {
        const auto& descriptor = GetParam();
        SCOPED_TRACE("Testing Arena Placement for: " + descriptor.node_name);
        ASSERT_NE(descriptor.run_test, nullptr);
        descriptor.run_test();
    }

    struct AstNodeArenaTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstNodeArenaTestDescriptor>& info) const
        {
            return info.param.node_name;
        }
    };

    INSTANTIATE_TEST_SUITE_P(
        AllAstNodes,
        AstNodeArenaParameterizedTest,
        testing::ValuesIn(get_all_ast_node_arena_test_descriptors()),
        AstNodeArenaTestNameGenerator{}
    );
}
