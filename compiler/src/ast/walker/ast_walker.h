#pragma once

#include <vector>
#include <span>
#include <type_traits>
#include <memory>
#include <concepts>

#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/walker/ast_walker_concepts.h"

namespace valuascript::compiler
{
    enum class TraversalAction : uint8_t
    {
        Continue,
        SkipChildren,
        Stop
    };

    template <bool IsConst, typename T>
    using MaybeConst = std::conditional_t<IsConst, const T, T>;

    template <typename Tuple, typename T>
    struct CategoryDispatcher;

    template <typename... Categories, typename T>
    struct CategoryDispatcher<std::tuple<Categories...>, T>
    {
        template <typename Action>
        static bool dispatch_enter(Action&& action)
        {
            return (
                (
                    (std::derived_from<T, Categories> && !std::is_same_v<T, Categories>)
                        ? action.template operator()<Categories>()
                        : true
                ) && ...
            );
        }

        template <typename Action>
        static void dispatch_leave(Action&& action)
        {
            (
                (
                    (std::derived_from<T, Categories> && !std::is_same_v<T, Categories>)
                        ? (action.template operator()<Categories>(), 0)
                        : 0
                ), ...
            );
        }
    };

    template <bool IsConst>
    class BasicAstWalker
    {
    public:
        using NodePtr = MaybeConst<IsConst, AstNode>*;
        using NodeSpan = std::span<NodePtr const>;
        using ProgramRef = MaybeConst<IsConst, Program>&;

        virtual ~BasicAstWalker() = default;

        [[nodiscard]] NodePtr parent() const noexcept
        {
            if (ancestor_stack_.size() < 2) return nullptr;
            return ancestor_stack_[ancestor_stack_.size() - 2];
        }

        [[nodiscard]] size_t depth() const noexcept
        {
            return ancestor_stack_.size();
        }

        [[nodiscard]] NodeSpan ancestor_stack() const noexcept
        {
            return NodeSpan(ancestor_stack_.data(), ancestor_stack_.size());
        }

        [[nodiscard]] bool is_root() const noexcept
        {
            return ancestor_stack_.size() <= 1;
        }

        template <typename T>
        [[nodiscard]] MaybeConst<IsConst, T>* find_ancestor() const noexcept
        {
            if (ancestor_stack_.empty()) return nullptr;
            for (size_t i = ancestor_stack_.size(); i > 0; --i)
            {
                NodePtr candidate = ancestor_stack_[i - 1];
                if (auto* typed = ast_cast<MaybeConst<IsConst, T>>(candidate))
                {
                    return typed;
                }
            }
            return nullptr;
        }

        virtual TraversalAction enter_node(MaybeConst<IsConst, AstNode>&) { return TraversalAction::Continue; }

        virtual void leave_node(MaybeConst<IsConst, AstNode>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, Statement>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, Statement>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, Expression>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, Expression>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, TypeAnnotation>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, TypeAnnotation>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, Modifier>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, Modifier>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, CallArgument>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, CallArgument>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, FunctionParameter>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, FunctionParameter>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, StructField>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, StructField>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, EnumCase>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, EnumCase>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, DictItem>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, DictItem>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, SwitchCase>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, SwitchCase>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, AssignmentTarget>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, AssignmentTarget>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, Comment>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, Comment>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, Program>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, Program>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, ImportStatement>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, ImportStatement>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, Directive>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, Directive>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, FunctionDefinition>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, FunctionDefinition>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, StructDefinition>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, StructDefinition>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, EnumDefinition>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, EnumDefinition>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, TypeAliasDefinition>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, TypeAliasDefinition>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, ExtensionDefinition>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, ExtensionDefinition>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, Assignment>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, Assignment>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, Reassignment>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, Reassignment>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, ExpressionStatement>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, ExpressionStatement>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, ReturnStatement>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, ReturnStatement>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, NumberLiteral>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, NumberLiteral>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, PercentageLiteral>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, PercentageLiteral>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, StringLiteral>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, StringLiteral>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, BooleanLiteral>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, BooleanLiteral>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, IdentifierAccess>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, IdentifierAccess>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, SelfExpression>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, SelfExpression>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, BinaryExpression>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, BinaryExpression>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, UnaryExpression>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, UnaryExpression>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, GroupingExpression>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, GroupingExpression>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, ConditionalExpression>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, ConditionalExpression>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, FunctionCall>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, FunctionCall>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, DictLiteral>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, DictLiteral>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, TensorLiteral>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, TensorLiteral>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, TupleLiteral>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, TupleLiteral>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, BracketAccess>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, BracketAccess>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, DotAccess>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, DotAccess>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, SwitchExpression>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, SwitchExpression>&)
        {
        }

        virtual TraversalAction enter(MaybeConst<IsConst, TupleTypeAnnotation>&) { return TraversalAction::Continue; }

        virtual void leave(MaybeConst<IsConst, TupleTypeAnnotation>&)
        {
        }

        template <typename T>
        void auto_walk_children(T& node)
        {
            for_each_ast_member(node, [&](auto& member)
            {
                using MemberT = std::remove_cvref_t<decltype(member)>;
                if (should_stop_) return;

                if constexpr (IsUniquePtrOfAstNode<MemberT>)
                {
                    if (member) walk(member.get());
                }
                else if constexpr (IsVectorOfUniquePtrOfAstNode<MemberT>)
                {
                    for (auto& item : member)
                    {
                        if (should_stop_) return;
                        if (item) walk(item.get());
                    }
                }
                else if constexpr (IsInnerAstNode<MemberT>)
                {
                    walk(member);
                }
                else if constexpr (IsVectorOfInnerAstNode<MemberT>)
                {
                    for (auto& item : member)
                    {
                        if (should_stop_) return;
                        walk(item);
                    }
                }
                else if constexpr (IsOptionalAstFieldOfAstNode<MemberT>)
                {
                    if (member.has_value())
                    {
                        using ValT = typename MemberT::value_type;
                        if constexpr (is_unique_ptr_v<ValT>)
                        {
                            if (member.get()) walk(member.get());
                        }
                        else
                        {
                            walk(*member);
                        }
                    }
                }
                else if constexpr (IsStdOptionalOfAstNode<MemberT>)
                {
                    if (member.has_value())
                    {
                        using ValT = typename MemberT::value_type;
                        if constexpr (is_unique_ptr_v<ValT>)
                        {
                            if (*member) walk((*member).get());
                        }
                        else
                        {
                            walk(*member);
                        }
                    }
                }
            });
        }

        virtual void walk_children(MaybeConst<IsConst, Program>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, ImportStatement>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, Directive>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, FunctionDefinition>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, StructDefinition>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, EnumDefinition>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, TypeAliasDefinition>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, ExtensionDefinition>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, Assignment>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, Reassignment>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, ExpressionStatement>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, ReturnStatement>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, BinaryExpression>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, UnaryExpression>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, GroupingExpression>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, ConditionalExpression>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, FunctionCall>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, DictLiteral>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, TensorLiteral>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, TupleLiteral>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, BracketAccess>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, DotAccess>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, SwitchExpression>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, TypeAnnotation>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, TupleTypeAnnotation>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, Modifier>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, CallArgument>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, FunctionParameter>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, StructField>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, EnumCase>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, AssignmentTarget>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, DictItem>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, SwitchCase>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, NumberLiteral>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, PercentageLiteral>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, StringLiteral>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, BooleanLiteral>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, IdentifierAccess>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, SelfExpression>& node) { auto_walk_children(node); }
        virtual void walk_children(MaybeConst<IsConst, Comment>& node) { auto_walk_children(node); }

        void walk(ProgramRef program)
        {
            should_stop_ = false;
            ancestor_stack_.clear();
            walk(static_cast<NodePtr>(&program));
        }

        void walk(NodePtr node)
        {
            if (!node || should_stop_) return;

            ancestor_stack_.push_back(node);

            if (enter_node(*node) == TraversalAction::Stop)
            {
                should_stop_ = true;
                ancestor_stack_.pop_back();
                return;
            }

            TraversalAction action = dispatch_enter(*node);
            if (action == TraversalAction::Stop)
            {
                should_stop_ = true;
                ancestor_stack_.pop_back();
                return;
            }

            if (action == TraversalAction::Continue && !should_stop_)
            {
                dispatch_children(*node);
            }

            if (!should_stop_)
            {
                dispatch_leave(*node);
                leave_node(*node);
            }

            ancestor_stack_.pop_back();
        }

        template <typename T>
            requires std::derived_from<T, AstNode> && (!std::is_same_v<T, AstNode>)
        void walk(MaybeConst<IsConst, T>* node)
        {
            walk(static_cast<NodePtr>(node));
        }

        template <typename T>
            requires std::derived_from<T, AstNode> && (!std::is_pointer_v<T>)
        void walk(T& node)
        {
            if (should_stop_) return;

            ancestor_stack_.push_back(&node);

            if (enter_node(node) == TraversalAction::Stop)
            {
                should_stop_ = true;
                ancestor_stack_.pop_back();
                return;
            }

            TraversalAction action = enter(node);
            if (action == TraversalAction::Stop)
            {
                should_stop_ = true;
                ancestor_stack_.pop_back();
                return;
            }

            if (action == TraversalAction::Continue && !should_stop_)
            {
                walk_children(node);
            }

            if (!should_stop_)
            {
                leave(node);
                leave_node(node);
            }

            ancestor_stack_.pop_back();
        }

    private:
        TraversalAction dispatch_enter(MaybeConst<IsConst, AstNode>& node)
        {
            TraversalAction result = TraversalAction::Continue;

            NodeDispatcher<AllAstNodeTypes>::dispatch(node.kind, [&]<typename T>()
            {
                bool should_continue = CategoryDispatcher<AstCategoryTypes, T>::dispatch_enter([&]<typename Cat>()
                {
                    if (enter(static_cast<MaybeConst<IsConst, Cat>&>(node)) == TraversalAction::Stop)
                    {
                        result = TraversalAction::Stop;
                        return false;
                    }
                    return true;
                });

                if (should_continue)
                {
                    result = enter(static_cast<MaybeConst<IsConst, T>&>(node));
                }
            });

            return result;
        }

        void dispatch_children(MaybeConst<IsConst, AstNode>& node)
        {
            NodeDispatcher<AllAstNodeTypes>::dispatch(node.kind, [&]<typename T>()
            {
                walk_children(static_cast<MaybeConst<IsConst, T>&>(node));
            });
        }

        void dispatch_leave(MaybeConst<IsConst, AstNode>& node)
        {
            NodeDispatcher<AllAstNodeTypes>::dispatch(node.kind, [&]<typename T>()
            {
                leave(static_cast<MaybeConst<IsConst, T>&>(node));

                CategoryDispatcher<AstCategoryTypes, T>::dispatch_leave([&]<typename Cat>()
                {
                    leave(static_cast<MaybeConst<IsConst, Cat>&>(node));
                });
            });
        }

        std::vector<NodePtr> ancestor_stack_;
        bool should_stop_ = false;
    };

    using ConstAstWalker = BasicAstWalker<true>;
    using AstWalker = BasicAstWalker<false>;
}
