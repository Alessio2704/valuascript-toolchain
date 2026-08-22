#pragma once

#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <ostream>
#include <tuple>
#include <memory>

#include "ast_sample_factory.h"
#include "ast/core/ast_node_registry.h"
#include "ast/core/ast_equality.h"
#include "ast/core/ast_disjoint.h"

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
        auto sample = create_sample<T>();
        if constexpr (std::same_as<decltype(sample), std::unique_ptr<T>>)
        {
            ASSERT_NE(sample, nullptr);
            EXPECT_EQ(sample->kind, T::KIND);
            EXPECT_GT(sample->span.line_start, 0);
            EXPECT_GT(sample->span.column_start, 0);
            EXPECT_GT(sample->span.length, 0);
            EXPECT_NE(sample->span.file_path, nullptr);
        }
        else
        {
            EXPECT_EQ(sample.kind, T::KIND);
            EXPECT_GT(sample.span.line_start, 0);
            EXPECT_GT(sample.span.column_start, 0);
            EXPECT_GT(sample.span.length, 0);
            EXPECT_NE(sample.span.file_path, nullptr);
        }
    }

    template <typename T>
    AstSampleTestDescriptor make_ast_sample_test_descriptor()
    {
        return AstSampleTestDescriptor{
            .node_name = std::string(to_string(T::KIND)),
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
