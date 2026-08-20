#pragma once

#include <optional>
#include <utility>
#include "ast/ast.h"
#include "span_assertions.h"
#include "matcher_concepts.h"

namespace valuascript::compiler::test
{
    template <typename M>
    struct FluentNodeMatcher
    {
        using node_type = typename std::decay_t<M>::node_type;

        M matcher;
        std::optional<SourceSpan> expected_span;
        std::optional<SourceSpan> expected_name_span;

        constexpr FluentNodeMatcher(M m) : matcher(std::move(m))
        {
        }

        [[nodiscard]] FluentNodeMatcher with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                                  size_t start_offset, size_t length) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_span(const SourceSpan& span) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_span = span;
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                                       size_t start_offset, size_t length) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_name_span(const SourceSpan& span) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_name_span = span;
            return copy;
        }

        void operator()(node_type* node) const
        {
            if (!node) return;
            if (expected_span.has_value())
            {
                AssertSpanMatch(node->span, *expected_span);
            }
            if (expected_name_span.has_value())
            {
                if (const auto* name_span = get_node_name_span(*node))
                {
                    AssertSpanMatch(*name_span, *expected_name_span);
                }
            }
            matcher(node);
        }

        explicit operator bool() const
        {
            if constexpr (requires { static_cast<bool>(matcher); })
            {
                return static_cast<bool>(matcher);
            }
            return true;
        }
    };

    template <typename F>
    struct MatcherStorage
    {
        F verifier;
        bool has_value = true;

        MatcherStorage() : verifier{}, has_value(false)
        {
        }

        MatcherStorage(F v) : verifier(std::move(v)), has_value(true)
        {
        }

        MatcherStorage(std::nullptr_t) : verifier{}, has_value(false)
        {
        }

        template <typename NodeT>
        void operator()(NodeT* node) const
        {
            if (has_value) verifier(node);
        }

        explicit operator bool() const { return has_value; }
    };

    template <>
    struct MatcherStorage<AnyMatcher>
    {
        AnyMatcher verifier;
        bool has_value = false;

        MatcherStorage() : verifier{}, has_value(false)
        {
        }

        MatcherStorage(AnyMatcher v) : verifier(std::move(v)), has_value(false)
        {
        }

        MatcherStorage(std::nullptr_t) : verifier{}, has_value(false)
        {
        }

        template <typename NodeT>
        void operator()(NodeT*) const
        {
        }

        explicit operator bool() const { return false; }
    };
}
