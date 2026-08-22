#pragma once

#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <tuple>
#include <memory>

#include "ast_sample_factory.h"
#include "ast/core/ast_node_registry.h"
#include "ast/validity/ast_validity.h"

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
            EXPECT_TRUE(ast_is_valid(sample));
            if constexpr (std::same_as<decltype(sample), std::unique_ptr<T>>)
            {
                EXPECT_EQ(sample->kind, T::KIND);
            }
            else
            {
                EXPECT_EQ(sample.kind, T::KIND);
            }
        }

        auto seq1 = create_sample<T>();
        auto seq2 = create_sample<T>();
        if constexpr (std::same_as<decltype(seq1), std::unique_ptr<T>>)
        {
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

        EXPECT_TRUE(ast_is_valid(reset1));
        EXPECT_TRUE(ast_is_valid(reset2));
        if constexpr (std::same_as<decltype(reset1), std::unique_ptr<T>>)
        {
            EXPECT_EQ(reset1->span, reset2->span);
            EXPECT_NE(reset1.get(), reset2.get());
        }
        else
        {
            EXPECT_EQ(reset1.span, reset2.span);
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
