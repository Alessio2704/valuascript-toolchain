#pragma once

#include <optional>
#include <utility>
#include "ast/ast.h"
#include "span_assertions.h"
#include "matcher_concepts.h"

namespace valuascript::compiler::test
{
    template <typename M>
    struct FluentNodeMatcher : public SpanSpecMixin<FluentNodeMatcher<M>>
    {
        using node_type = typename std::decay_t<M>::node_type;

        M matcher;

        constexpr FluentNodeMatcher(M m) : matcher(std::move(m))
        {
        }

        void operator()(node_type* node) const
        {
            if (!node) return;
            if (this->span.has_value())
            {
                AssertSpanMatch(node->span, *this->span);
            }
            if (this->name_span.has_value())
            {
                if (const auto* name_span_ptr = get_node_name_span(*node))
                {
                    AssertSpanMatch(*name_span_ptr, *this->name_span);
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
