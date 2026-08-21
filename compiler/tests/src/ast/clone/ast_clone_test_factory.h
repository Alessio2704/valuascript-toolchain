#pragma once

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <concepts>
#include <tuple>

#include "token/source_span.h"
#include "token/token_type.h"
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/core/ast_clone.h"
#include "ast/core/ast_node_registry.h"
#include "ast/core/ast_node_schema.h"

namespace valuascript::compiler::test
{
    inline SourceSpan make_test_span(
        size_t l1 = 1, size_t c1 = 1,
        size_t l2 = 1, size_t c2 = 10,
        size_t off = 0, size_t len = 10,
        std::string_view file = "test_sample.vs")
    {
        SourceSpan sp{};
        sp.line_start = l1;
        sp.column_start = c1;
        sp.line_end = l2;
        sp.column_end = c2;
        sp.start_offset = off;
        sp.length = len;
        sp.file_path = std::make_shared<const std::string>(file);
        return sp;
    }

    inline void assert_spans_equal(const SourceSpan& orig, const SourceSpan& clone)
    {
        EXPECT_EQ(orig.line_start, clone.line_start);
        EXPECT_EQ(orig.column_start, clone.column_start);
        EXPECT_EQ(orig.line_end, clone.line_end);
        EXPECT_EQ(orig.column_end, clone.column_end);
        EXPECT_EQ(orig.start_offset, clone.start_offset);
        EXPECT_EQ(orig.length, clone.length);
        EXPECT_EQ(orig.path(), clone.path());
        EXPECT_EQ(orig, clone);
    }

    inline void assert_node_names_equal(const NodeName& orig, const NodeName& clone)
    {
        EXPECT_EQ(orig.value, clone.value);
        assert_spans_equal(orig.span, clone.span);
    }

    inline void assert_deep_equal(const std::string& orig, const std::string& clone)
    {
        EXPECT_EQ(orig, clone);
    }

    inline void assert_deep_equal(bool orig, bool clone)
    {
        EXPECT_EQ(orig, clone);
    }

    inline void assert_deep_equal(TokenType orig, TokenType clone)
    {
        EXPECT_EQ(orig, clone);
    }

    inline void assert_deep_equal(AstKind orig, AstKind clone)
    {
        EXPECT_EQ(orig, clone);
    }

    inline void assert_deep_equal(const std::optional<std::string>& orig, const std::optional<std::string>& clone)
    {
        EXPECT_EQ(orig, clone);
    }

    inline void assert_deep_equal(const SourceSpan& orig, const SourceSpan& clone)
    {
        assert_spans_equal(orig, clone);
    }

    inline void assert_deep_equal(const NodeName& orig, const NodeName& clone)
    {
        assert_node_names_equal(orig, clone);
    }

    template <typename T>
    inline void assert_deep_equal(const T& orig, const T& clone);

    template <typename T>
    inline void assert_ptr_disjoint_and_deep_equal(const std::unique_ptr<T>& orig, const std::unique_ptr<T>& clone)
    {
        if (!orig)
        {
            EXPECT_EQ(clone, nullptr);
            return;
        }
        ASSERT_NE(clone, nullptr);
        EXPECT_NE(clone.get(), orig.get());
        assert_deep_equal(*orig, *clone);
    }

    template <typename T>
    inline void assert_vector_deep_equal(const std::vector<T>& orig_vec, const std::vector<T>& clone_vec)
    {
        ASSERT_EQ(orig_vec.size(), clone_vec.size());
        for (size_t i = 0; i < orig_vec.size(); ++i)
        {
            assert_deep_equal(orig_vec[i], clone_vec[i]);
        }
    }

    template <typename T>
    inline void assert_vector_ptr_disjoint_and_deep_equal(
        const std::vector<std::unique_ptr<T>>& orig_vec,
        const std::vector<std::unique_ptr<T>>& clone_vec)
    {
        ASSERT_EQ(orig_vec.size(), clone_vec.size());
        for (size_t i = 0; i < orig_vec.size(); ++i)
        {
            assert_ptr_disjoint_and_deep_equal(orig_vec[i], clone_vec[i]);
        }
    }

    template <typename T>
    inline void assert_deep_equal(const std::unique_ptr<T>& orig, const std::unique_ptr<T>& clone)
    {
        assert_ptr_disjoint_and_deep_equal(orig, clone);
    }

    template <typename T>
    inline void assert_deep_equal(const std::vector<T>& orig, const std::vector<T>& clone)
    {
        assert_vector_deep_equal(orig, clone);
    }

    template <AstElement T>
    inline void assert_deep_equal(const std::vector<std::unique_ptr<T>>& orig,
                                  const std::vector<std::unique_ptr<T>>& clone)
    {
        assert_vector_ptr_disjoint_and_deep_equal(orig, clone);
    }

    template <HasAstNodeSchema T>
    inline void assert_deep_equal_node(const T& orig, const T& clone)
    {
        EXPECT_EQ(orig.kind, clone.kind);
        for_each_ast_member_pair(orig, clone, [](const auto& orig_val, const auto& clone_val)
        {
            assert_deep_equal(orig_val, clone_val);
        });
    }

    template <typename T>
    inline void assert_deep_equal(const T& orig, const T& clone)
    {
        if constexpr (HasAstNodeSchema<T>)
        {
            assert_deep_equal_node(orig, clone);
        }
        else if constexpr (std::is_base_of_v<AstNode, T>)
        {
            EXPECT_EQ(orig.kind, clone.kind);
            assert_spans_equal(orig.span, clone.span);
        }
        else
        {
            EXPECT_EQ(orig, clone);
        }
    }

    template <typename T>
    struct AstCloneSampleFactory;

    template <typename T>
    concept HasValidCloneSampleFactory = requires
    {
        { AstCloneSampleFactory<T>::create_sample() };
        { AstCloneSampleFactory<T>::run_full_clone_test() } -> std::same_as<void>;
    };
}
