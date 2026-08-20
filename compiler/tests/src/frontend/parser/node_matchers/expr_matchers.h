#pragma once

#include "core.h"
#include "specs.h"
#include "expect_functions.h"

namespace valuascript::compiler::test
{
    template <void (*ExpectFn)(AstNode*, std::string_view)>
    struct SingleValueMatcher
    {
        using node_type = Expression;
        StringStorage value;
        void operator()(Expression* node) const { ExpectFn(node, value.get()); }
    };

    using NumberMatcher = SingleValueMatcher<ExpectNumber>;
    using StringMatcher = SingleValueMatcher<ExpectString>;
    using PercentageMatcher = SingleValueMatcher<ExpectPercentage>;
    using IdentifierMatcher = SingleValueMatcher<ExpectIdentifier>;

    inline FluentNodeMatcher<NumberMatcher> IsNumber(StringStorage value)
    {
        return FluentNodeMatcher<NumberMatcher>{NumberMatcher{std::move(value)}};
    }

    inline FluentNodeMatcher<StringMatcher> IsString(StringStorage val)
    {
        return FluentNodeMatcher<StringMatcher>{StringMatcher{std::move(val)}};
    }

    struct BooleanMatcher
    {
        using node_type = Expression;
        bool value;
        void operator()(Expression* node) const { ExpectBoolean(node, value); }
    };

    inline FluentNodeMatcher<BooleanMatcher> IsBoolean(bool val)
    {
        return FluentNodeMatcher<BooleanMatcher>{BooleanMatcher{val}};
    }

    inline FluentNodeMatcher<PercentageMatcher> IsPercentage(StringStorage val)
    {
        return FluentNodeMatcher<PercentageMatcher>{PercentageMatcher{std::move(val)}};
    }

    inline FluentNodeMatcher<IdentifierMatcher> IsIdentifier(StringStorage val)
    {
        return FluentNodeMatcher<IdentifierMatcher>{IdentifierMatcher{std::move(val)}};
    }

    struct SelfMatcher
    {
        using node_type = Expression;
        void operator()(Expression* node) const { ExpectSelf(node); }
    };

    inline FluentNodeMatcher<SelfMatcher> IsSelf()
    {
        return FluentNodeMatcher<SelfMatcher>{SelfMatcher{}};
    }

    template <ExprMatcher L = AnyMatcher, ExprMatcher R = AnyMatcher>
    struct BinaryMatcher
    {
        using node_type = Expression;
        TokenType op;
        MatcherStorage<L> left_v;
        MatcherStorage<R> right_v;

        void operator()(Expression* node) const
        {
            if (auto b = ExpectNode<BinaryExpression>(node))
            {
                EXPECT_EQ(b->op, op) << "Binary expression operator mismatch.";
                left_v(b->left.get());
                right_v(b->right.get());
            }
        }
    };

    template <ExprMatcher L = AnyMatcher, ExprMatcher R = AnyMatcher>
    inline auto IsBinary(TokenType op, L&& l = {}, R&& r = {})
    {
        return FluentNodeMatcher<BinaryMatcher<std::decay_t<L>, std::decay_t<R>>>{
            BinaryMatcher<std::decay_t<L>, std::decay_t<R>>{
                op, std::forward<L>(l), std::forward<R>(r)
            }
        };
    }

    template <ExprMatcher R = AnyMatcher>
    struct UnaryMatcher
    {
        using node_type = Expression;
        TokenType op;
        MatcherStorage<R> right_v;

        void operator()(Expression* node) const
        {
            if (auto u = ExpectNode<UnaryExpression>(node))
            {
                EXPECT_EQ(u->op, op) << "Unary expression operator mismatch.";
                right_v(u->right.get());
            }
        }
    };

    template <ExprMatcher R = AnyMatcher>
    inline auto IsUnary(TokenType op, R&& r = {})
    {
        return FluentNodeMatcher<UnaryMatcher<std::decay_t<R>>>{
            UnaryMatcher<std::decay_t<R>>{op, std::forward<R>(r)}
        };
    }

    template <ExprMatcher I = AnyMatcher>
    struct GroupingMatcher
    {
        using node_type = Expression;
        MatcherStorage<I> inner_v;

        void operator()(Expression* node) const
        {
            if (auto g = ExpectNode<GroupingExpression>(node))
            {
                inner_v(g->expression.get());
            }
        }
    };

    template <ExprMatcher I = AnyMatcher>
    inline auto IsGrouping(I&& inner = {})
    {
        return FluentNodeMatcher<GroupingMatcher<std::decay_t<I>>>{
            GroupingMatcher<std::decay_t<I>>{std::forward<I>(inner)}
        };
    }

    template <ExprMatcher C = AnyMatcher, ExprMatcher T = AnyMatcher, ExprMatcher E = AnyMatcher>
    struct ConditionalMatcher
    {
        using node_type = Expression;
        MatcherStorage<C> cond_v;
        MatcherStorage<T> then_v;
        MatcherStorage<E> else_v;

        void operator()(Expression* node) const
        {
            if (auto cond = ExpectNode<ConditionalExpression>(node))
            {
                cond_v(cond->condition.get());
                then_v(cond->then_branch.get());
                else_v(cond->else_branch.get());
            }
        }
    };

    template <ExprMatcher C = AnyMatcher, ExprMatcher T = AnyMatcher, ExprMatcher E = AnyMatcher>
    inline auto IsConditional(C&& condition = {}, T&& then_expr = {}, E&& else_expr = {})
    {
        return FluentNodeMatcher<ConditionalMatcher<std::decay_t<C>, std::decay_t<T>, std::decay_t<E>>>{
            ConditionalMatcher<std::decay_t<C>, std::decay_t<T>, std::decay_t<E>>{
                std::forward<C>(condition), std::forward<T>(then_expr), std::forward<E>(else_expr)
            }
        };
    }

    template <ExprMatcher T = AnyMatcher>
    struct CallMatcher
    {
        using node_type = Expression;
        MatcherStorage<T> target_v;
        std::vector<ArgSpec> args;

        void operator()(Expression* node) const
        {
            if (auto c = ExpectNode<FunctionCall>(node))
            {
                target_v(c->target.get());
                ExpectArguments(c->arguments, args);
            }
        }
    };

    template <typename T, typename U>
    ExprVerifier IsCall(T&&, std::initializer_list<U>) = delete;

    template <ExprMatcher T = AnyMatcher>
    inline auto IsCall(T&& target, std::vector<ArgSpec> args = {})
    {
        return FluentNodeMatcher<CallMatcher<std::decay_t<T>>>{
            CallMatcher<std::decay_t<T>>{std::forward<T>(target), std::move(args)}
        };
    }

    template <ExprMatcher T = AnyMatcher, typename... ArgSpecs>
        requires (sizeof...(ArgSpecs) > 0 && !(sizeof...(ArgSpecs) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<ArgSpecs...>>>, std::vector<ArgSpec>>))
    inline auto IsCall(T&& target, ArgSpecs&&... args)
    {
        std::vector<ArgSpec> arg_list = {std::forward<ArgSpecs>(args)...};
        return FluentNodeMatcher<CallMatcher<std::decay_t<T>>>{
            CallMatcher<std::decay_t<T>>{std::forward<T>(target), std::move(arg_list)}
        };
    }

    template <ExprMatcher T = AnyMatcher, ExprMatcher I = AnyMatcher>
    struct BracketMatcher
    {
        using node_type = Expression;
        MatcherStorage<T> target_v;
        MatcherStorage<I> index_v;

        void operator()(Expression* node) const
        {
            if (auto b = ExpectNode<BracketAccess>(node))
            {
                target_v(b->target.get());
                index_v(b->index.get());
            }
        }
    };

    template <ExprMatcher T = AnyMatcher, ExprMatcher I = AnyMatcher>
    inline auto IsBracket(T&& target, I&& index)
    {
        return FluentNodeMatcher<BracketMatcher<std::decay_t<T>, std::decay_t<I>>>{
            BracketMatcher<std::decay_t<T>, std::decay_t<I>>{
                std::forward<T>(target), std::forward<I>(index)
            }
        };
    }

    template <ExprMatcher T = AnyMatcher>
    struct DotMatcher
    {
        using node_type = Expression;
        MatcherStorage<T> target_v;
        StringStorage property;

        void operator()(Expression* node) const
        {
            if (auto d = ExpectNode<DotAccess>(node))
            {
                target_v(d->target.get());
                EXPECT_EQ(d->property_name, property.get()) << "Dot access property name mismatch.";
            }
        }
    };

    template <ExprMatcher T = AnyMatcher>
    inline auto IsDot(T&& target, StringStorage property)
    {
        return FluentNodeMatcher<DotMatcher<std::decay_t<T>>>{
            DotMatcher<std::decay_t<T>>{std::forward<T>(target), std::move(property)}
        };
    }

    template <ExprMatcher T = AnyMatcher, ExprMatcher D = AnyMatcher>
    struct SwitchMatcher
    {
        using node_type = Expression;
        MatcherStorage<T> target_v;
        std::vector<SwitchCaseSpec> cases;
        std::vector<ModifierSpec> default_mods;
        MatcherStorage<D> default_v;

        void operator()(Expression* node) const
        {
            ExpectSwitch(node, target_v, cases, default_mods, default_v);
        }
    };

    template <typename T, typename U>
    ExprVerifier IsSwitch(T&&, std::initializer_list<U>) = delete;

    template <ExprMatcher T = AnyMatcher, ExprMatcher D = AnyMatcher>
    inline auto IsSwitch(T&& t, std::vector<SwitchCaseSpec> cases, std::vector<ModifierSpec> default_mods,
                         D&& default_expr = {})
    {
        return FluentNodeMatcher<SwitchMatcher<std::decay_t<T>, std::decay_t<D>>>{
            SwitchMatcher<std::decay_t<T>, std::decay_t<D>>{
                std::forward<T>(t), std::move(cases), std::move(default_mods), std::forward<D>(default_expr)
            }
        };
    }

    template <ExprMatcher T = AnyMatcher, ExprMatcher D = AnyMatcher>
    inline auto IsSwitch(T&& t, std::vector<SwitchCaseSpec> cases, D&& default_expr = {})
    {
        return IsSwitch(std::forward<T>(t), std::move(cases), {}, std::forward<D>(default_expr));
    }

    template <ExprMatcher T = AnyMatcher, typename... Cases>
        requires (sizeof...(Cases) > 0 && !(sizeof...(Cases) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Cases...>>>, std::vector<SwitchCaseSpec>>))
    inline auto IsSwitch(T&& t, Cases&&... cases)
    {
        std::vector<SwitchCaseSpec> case_list = {std::forward<Cases>(cases)...};
        return FluentNodeMatcher<SwitchMatcher<std::decay_t<T>, AnyMatcher>>{
            SwitchMatcher<std::decay_t<T>, AnyMatcher>{
                std::forward<T>(t), std::move(case_list), {}, AnyMatcher{}
            }
        };
    }

    template <typename ASTNodeT, typename... Matchers>
    struct SequenceVariadicMatcher
    {
        using node_type = Expression;
        std::tuple<Matchers...> elements;

        void operator()(Expression* node) const
        {
            if (auto t = ExpectNode<ASTNodeT>(node))
            {
                ASSERT_EQ(t->elements.size(), sizeof...(Matchers)) << get_ast_node_name<ASTNodeT>() <<
                    " elements count mismatch.";
                size_t idx = 0;
                std::apply([&](const auto&... m)
                {
                    ((m(t->elements[idx++].get())), ...);
                }, elements);
            }
        }
    };

    template <typename... Matchers>
    using TensorVariadicMatcher = SequenceVariadicMatcher<TensorLiteral, Matchers...>;

    template <typename T = ExprVerifier>
    ExprVerifier IsTensor(std::initializer_list<T>) = delete;

    struct TensorVectorMatcher
    {
        using node_type = Expression;
        std::vector<ExprVerifier> elements;

        void operator()(Expression* node) const
        {
            ExpectTensor(node, elements);
        }
    };

    inline FluentNodeMatcher<TensorVectorMatcher> IsTensor(std::vector<ExprVerifier> elements)
    {
        return FluentNodeMatcher<TensorVectorMatcher>{TensorVectorMatcher{std::move(elements)}};
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<ExprVerifier>>))
    inline auto IsTensor(Matchers&&... matchers)
    {
        return FluentNodeMatcher<TensorVariadicMatcher<std::decay_t<Matchers>...>>{
            TensorVariadicMatcher<std::decay_t<Matchers>...>{
                std::make_tuple(std::forward<Matchers>(matchers)...)
            }
        };
    }

    inline FluentNodeMatcher<TensorVectorMatcher> IsTensor()
    {
        return FluentNodeMatcher<TensorVectorMatcher>{TensorVectorMatcher{}};
    }

    template <typename... Matchers>
    using TupleVariadicMatcher = SequenceVariadicMatcher<TupleLiteral, Matchers...>;

    template <typename T = ExprVerifier>
    ExprVerifier IsTuple(std::initializer_list<T>) = delete;

    struct TupleVectorMatcher
    {
        using node_type = Expression;
        std::vector<ExprVerifier> elements;

        void operator()(Expression* node) const
        {
            ExpectTuple(node, elements);
        }
    };

    inline FluentNodeMatcher<TupleVectorMatcher> IsTuple(std::vector<ExprVerifier> elements)
    {
        return FluentNodeMatcher<TupleVectorMatcher>{TupleVectorMatcher{std::move(elements)}};
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<ExprVerifier>>))
    inline auto IsTuple(Matchers&&... matchers)
    {
        return FluentNodeMatcher<TupleVariadicMatcher<std::decay_t<Matchers>...>>{
            TupleVariadicMatcher<std::decay_t<Matchers>...>{
                std::make_tuple(std::forward<Matchers>(matchers)...)
            }
        };
    }

    inline FluentNodeMatcher<TupleVectorMatcher> IsTuple()
    {
        return FluentNodeMatcher<TupleVectorMatcher>{TupleVectorMatcher{}};
    }

    template <typename T = DictItemSpec>
    ExprVerifier IsDict(std::initializer_list<T>) = delete;

    template <typename... Matchers>
    struct DictVariadicMatcher
    {
        using node_type = Expression;
        std::tuple<Matchers...> items;

        void operator()(Expression* node) const
        {
            std::vector<DictItemSpec> item_vec;
            item_vec.reserve(sizeof...(Matchers));
            std::apply([&](const auto&... m)
            {
                (item_vec.push_back(DictItemSpec(m)), ...);
            }, items);
            ExpectDict(node, item_vec);
        }
    };

    struct DictMatcher
    {
        using node_type = Expression;
        std::vector<DictItemSpec> items;
        void operator()(Expression* node) const { ExpectDict(node, items); }
    };

    inline FluentNodeMatcher<DictMatcher> IsDict(std::vector<DictItemSpec> items)
    {
        return FluentNodeMatcher<DictMatcher>{DictMatcher{std::move(items)}};
    }

    inline FluentNodeMatcher<DictMatcher> IsDict()
    {
        return FluentNodeMatcher<DictMatcher>{DictMatcher{}};
    }

    template <typename... Matchers>
        requires (sizeof...(Matchers) > 0 && !(sizeof...(Matchers) == 1 && std::same_as<
            std::decay_t<std::tuple_element_t<0, std::tuple<Matchers...>>>, std::vector<DictItemSpec>>))
    inline auto IsDict(Matchers&&... matchers)
    {
        return FluentNodeMatcher<DictVariadicMatcher<std::decay_t<Matchers>...>>{
            DictVariadicMatcher<std::decay_t<Matchers>...>{
                std::make_tuple(std::forward<Matchers>(matchers)...)
            }
        };
    }
}
