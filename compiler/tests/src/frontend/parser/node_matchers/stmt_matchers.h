#pragma once

#include "core.h"
#include "specs.h"
#include "expect_functions.h"

namespace valuascript::compiler::test
{
    template <typename V = AnyMatcher>
    struct AssignmentMatcher
    {
        using node_type = Assignment;
        std::vector<AssignmentTargetSpec> targets;
        MatcherStorage<V> value;

        void operator()(Statement* s) const
        {
            if (auto a = ExpectNode<Assignment>(s))
            {
                ASSERT_EQ(a->targets.size(), targets.size()) << "Assignment targets count mismatch.";
                for (size_t i = 0; i < targets.size(); i++)
                {
                    ExpectModifiers(a->targets[i].modifiers, targets[i].modifiers);
                    EXPECT_EQ(a->targets[i].name, targets[i].name.get()) << "Assignment target name mismatch at index "
                        << i << ".";
                    if (targets[i].type_v) targets[i].type_v(a->targets[i].type.get());
                }
                value(a->value.get());
            }
        }
    };

    template <typename V = AnyMatcher>
    inline AssignmentVerifier IsAssignment(std::vector<AssignmentTargetSpec> targets, V&& value = {})
    {
        return AssignmentVerifier(AssignmentMatcher<std::decay_t<V>>{std::move(targets), std::forward<V>(value)});
    }

    template <typename T = AnyMatcher, typename V = AnyMatcher>
    struct ReassignmentMatcher
    {
        using node_type = Reassignment;
        MatcherStorage<T> target_v;
        MatcherStorage<V> val_v;

        void operator()(Statement* s) const
        {
            if (auto r = ExpectNode<Reassignment>(s))
            {
                target_v(r->target.get());
                val_v(r->value.get());
            }
        }
    };

    template <typename T = AnyMatcher, typename V = AnyMatcher>
    inline ReassignmentVerifier IsReassignment(T&& target = {}, V&& value = {})
    {
        return ReassignmentVerifier(ReassignmentMatcher<std::decay_t<T>, std::decay_t<V>>{
            std::forward<T>(target), std::forward<V>(value)
        });
    }

    template <typename T = ExprVerifier>
    ReturnVerifier IsReturn(std::initializer_list<T>) = delete;

    template <typename... Matchers>
    struct ReturnVariadicMatcher
    {
        using node_type = ReturnStatement;
        std::tuple<Matchers...> values;

        void operator()(Statement* s) const
        {
            if (auto r = ExpectNode<ReturnStatement>(s))
            {
                ASSERT_EQ(r->values.size(), sizeof...(Matchers)) << "Return values count mismatch.";
                size_t idx = 0;
                std::apply([&](const auto&... m)
                {
                    ((m(r->values[idx++].get())), ...);
                }, values);
            }
        }
    };

    struct ReturnMatcher
    {
        using node_type = ReturnStatement;
        std::vector<ModifierSpec> modifiers;
        std::vector<ExprVerifier> values;

        void operator()(Statement* s) const
        {
            if (auto r = ExpectNode<ReturnStatement>(s))
            {
                ExpectReturn(r, modifiers, values);
            }
        }
    };

    inline ReturnVerifier IsReturn(std::vector<ModifierSpec> modifiers, std::vector<ExprVerifier> values)
    {
        return ReturnVerifier(ReturnMatcher{.modifiers = std::move(modifiers), .values = std::move(values)});
    }

    inline ReturnVerifier IsReturn(std::vector<ExprVerifier> values = {})
    {
        return IsReturn({}, std::move(values));
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<ExprVerifier>>))
    inline ReturnVerifier IsReturn(Matchers&&... matchers)
    {
        return ReturnVerifier(ReturnVariadicMatcher<std::decay_t<Matchers>...>{
            std::make_tuple(std::forward<Matchers>(matchers)...)
        });
    }

    template <typename E = AnyMatcher>
    struct ExprStmtMatcher
    {
        using node_type = ExpressionStatement;
        MatcherStorage<E> expr_v;

        void operator()(Statement* s) const
        {
            if (auto es = ExpectNode<ExpressionStatement>(s))
            {
                expr_v(es->expr.get());
            }
        }
    };

    template <typename E = AnyMatcher>
    inline ExprStmtVerifier IsExprStmt(E&& expr = {})
    {
        return ExprStmtVerifier(ExprStmtMatcher<std::decay_t<E>>{std::forward<E>(expr)});
    }
}
