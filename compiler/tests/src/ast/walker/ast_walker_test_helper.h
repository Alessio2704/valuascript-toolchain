#pragma once

#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <tuple>
#include <utility>
#include <type_traits>
#include <ostream>

#include "ast/walker/ast_walker.h"
#include "ast/factory/ast_factory.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/categories/ast_category_types.h"
#include "utils/traits/tuple_traits.h"
#include "../factory/ast_factory_test_reflection.h"

namespace valuascript::compiler::test
{
    struct AstWalkerTestDescriptor
    {
        std::string name;
        std::function<void()> run_test;

        friend std::ostream& operator<<(std::ostream& os, const AstWalkerTestDescriptor& desc)
        {
            return os << desc.name;
        }
    };

    template <typename Tuple, template <typename> class TestBuilder>
    struct AstWalkerDescriptorCollector;

    template <template <typename> class TestBuilder, typename... Types>
    struct AstWalkerDescriptorCollector<std::tuple<Types...>, TestBuilder>
    {
        static std::vector<AstWalkerTestDescriptor> collect()
        {
            return { TestBuilder<Types>::build()... };
        }
    };

    struct AstWalkerTestNameGenerator
    {
        std::string operator()(const testing::TestParamInfo<AstWalkerTestDescriptor>& info) const
        {
            return info.param.name;
        }
    };
}
