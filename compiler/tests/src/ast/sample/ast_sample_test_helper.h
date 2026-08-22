#pragma once

#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <memory>

#include "ast_sample_factory.h"
#include "ast/core/ast_node_registry.h"

namespace valuascript::compiler::test
{
    struct AstSampleTestDescriptor
    {
        std::string node_name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstSampleTestDescriptor& desc)
        {
            return os << desc.node_name;
        }
    };

    template <typename T>
    inline void test_single_sample_node()
    {
        for (int depth = 0; depth <= 3; ++depth)
        {
            auto sample = create_sample<T>(depth);
            if constexpr (std::same_as<decltype(sample), std::unique_ptr<T>>)
            {
                ASSERT_NE(sample, nullptr);
                EXPECT_EQ(sample->kind, T::KIND);
                EXPECT_TRUE(sample->is_valid());
            }
            else
            {
                EXPECT_EQ(sample.kind, T::KIND);
                EXPECT_TRUE(sample.is_valid());
            }
        }

        auto seq1 = create_sample<T>();
        auto seq2 = create_sample<T>();
        if constexpr (std::same_as<decltype(seq1), std::unique_ptr<T>>)
        {
            ASSERT_NE(seq1, nullptr);
            ASSERT_NE(seq2, nullptr);
            EXPECT_NE(seq1->span.start_offset, seq2->span.start_offset);
            EXPECT_NE(seq1.get(), seq2.get());
        }
        else
        {
            EXPECT_NE(seq1.span.start_offset, seq2.span.start_offset);
        }

        reset_sample_generator_state(777);
        auto reset1 = create_sample<T>();
        reset_sample_generator_state(777);
        auto reset2 = create_sample<T>();

        if constexpr (std::same_as<decltype(reset1), std::unique_ptr<T>>)
        {
            ASSERT_NE(reset1, nullptr);
            ASSERT_NE(reset2, nullptr);
            EXPECT_EQ(reset1->span, reset2->span);
            EXPECT_NE(reset1.get(), reset2.get());
            EXPECT_EQ(reset1->kind, T::KIND);
            EXPECT_EQ(reset2->kind, T::KIND);
            EXPECT_TRUE(reset1->is_valid());
            EXPECT_TRUE(reset2->is_valid());
        }
        else
        {
            EXPECT_EQ(reset1.span, reset2.span);
            EXPECT_EQ(reset1.kind, T::KIND);
            EXPECT_EQ(reset2.kind, T::KIND);
            EXPECT_TRUE(reset1.is_valid());
            EXPECT_TRUE(reset2.is_valid());
        }
    }

    template <typename T>
    AstSampleTestDescriptor make_ast_sample_test_descriptor()
    {
        return AstSampleTestDescriptor{
            .node_name = std::string(get_ast_node_name<T>()),
            .run_test = []() {
                test_single_sample_node<T>();
            }
        };
    }

    template <typename Tuple>
    struct AstSampleTestDescriptorCollector;

    template <typename... Types>
    struct AstSampleTestDescriptorCollector<std::tuple<Types...>>
    {
        static std::vector<AstSampleTestDescriptor> collect()
        {
            return { make_ast_sample_test_descriptor<Types>()... };
        }
    };

    inline std::vector<AstSampleTestDescriptor> get_all_ast_sample_test_descriptors()
    {
        return AstSampleTestDescriptorCollector<AllAstNodeTypes>::collect();
    }
}
