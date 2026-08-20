#pragma once

#include <vector>
#include <span>
#include <type_traits>
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/core/ast_node_registry.h"

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

    template <typename Tuple>
    struct NodeDispatcher;

    template <typename... Types>
    struct NodeDispatcher<std::tuple<Types...>>
    {
        template <typename Fn>
        static bool dispatch(AstKind kind, Fn&& fn)
        {
            return ((kind == Types::KIND ? (fn.template operator()<Types>(), true) : false) || ...);
        }
    };

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

        virtual void walk_children(MaybeConst<IsConst, Program>& node);
        virtual void walk_children(MaybeConst<IsConst, ImportStatement>& node);
        virtual void walk_children(MaybeConst<IsConst, Directive>& node);
        virtual void walk_children(MaybeConst<IsConst, FunctionDefinition>& node);
        virtual void walk_children(MaybeConst<IsConst, StructDefinition>& node);
        virtual void walk_children(MaybeConst<IsConst, EnumDefinition>& node);
        virtual void walk_children(MaybeConst<IsConst, TypeAliasDefinition>& node);
        virtual void walk_children(MaybeConst<IsConst, ExtensionDefinition>& node);
        virtual void walk_children(MaybeConst<IsConst, Assignment>& node);
        virtual void walk_children(MaybeConst<IsConst, Reassignment>& node);
        virtual void walk_children(MaybeConst<IsConst, ExpressionStatement>& node);
        virtual void walk_children(MaybeConst<IsConst, ReturnStatement>& node);
        virtual void walk_children(MaybeConst<IsConst, BinaryExpression>& node);
        virtual void walk_children(MaybeConst<IsConst, UnaryExpression>& node);
        virtual void walk_children(MaybeConst<IsConst, GroupingExpression>& node);
        virtual void walk_children(MaybeConst<IsConst, ConditionalExpression>& node);
        virtual void walk_children(MaybeConst<IsConst, FunctionCall>& node);
        virtual void walk_children(MaybeConst<IsConst, DictLiteral>& node);
        virtual void walk_children(MaybeConst<IsConst, TensorLiteral>& node);
        virtual void walk_children(MaybeConst<IsConst, TupleLiteral>& node);
        virtual void walk_children(MaybeConst<IsConst, BracketAccess>& node);
        virtual void walk_children(MaybeConst<IsConst, DotAccess>& node);
        virtual void walk_children(MaybeConst<IsConst, SwitchExpression>& node);
        virtual void walk_children(MaybeConst<IsConst, TypeAnnotation>& node);
        virtual void walk_children(MaybeConst<IsConst, TupleTypeAnnotation>& node);
        virtual void walk_children(MaybeConst<IsConst, Modifier>& node);
        virtual void walk_children(MaybeConst<IsConst, CallArgument>& node);
        virtual void walk_children(MaybeConst<IsConst, FunctionParameter>& node);
        virtual void walk_children(MaybeConst<IsConst, StructField>& node);
        virtual void walk_children(MaybeConst<IsConst, EnumCase>& node);
        virtual void walk_children(MaybeConst<IsConst, AssignmentTarget>& node);
        virtual void walk_children(MaybeConst<IsConst, DictItem>& node);
        virtual void walk_children(MaybeConst<IsConst, SwitchCase>& node);

        virtual void walk_children(MaybeConst<IsConst, NumberLiteral>&)
        {
        }

        virtual void walk_children(MaybeConst<IsConst, PercentageLiteral>&)
        {
        }

        virtual void walk_children(MaybeConst<IsConst, StringLiteral>&)
        {
        }

        virtual void walk_children(MaybeConst<IsConst, BooleanLiteral>&)
        {
        }

        virtual void walk_children(MaybeConst<IsConst, IdentifierAccess>&)
        {
        }

        virtual void walk_children(MaybeConst<IsConst, SelfExpression>&)
        {
        }

        virtual void walk_children(MaybeConst<IsConst, Comment>&)
        {
        }

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

        template <AstElement T>
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

            NodeDispatcher<AllAstNodeTypes>::dispatch(node.kind, [&]<typename T>() {
                bool should_continue = CategoryDispatcher<AstCategoryTypes, T>::dispatch_enter([&]<typename Cat>() {
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
            NodeDispatcher<AllAstNodeTypes>::dispatch(node.kind, [&]<typename T>() {
                walk_children(static_cast<MaybeConst<IsConst, T>&>(node));
            });
        }

        void dispatch_leave(MaybeConst<IsConst, AstNode>& node)
        {
            NodeDispatcher<AllAstNodeTypes>::dispatch(node.kind, [&]<typename T>() {
                leave(static_cast<MaybeConst<IsConst, T>&>(node));

                CategoryDispatcher<AstCategoryTypes, T>::dispatch_leave([&]<typename Cat>() {
                    leave(static_cast<MaybeConst<IsConst, Cat>&>(node));
                });
            });
        }

        std::vector<NodePtr> ancestor_stack_;
        bool should_stop_ = false;
    };

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, Program>& node)
    {
        for (auto& comment : node.comments)
        {
            if (should_stop_) return;
            walk(comment);
        }
        for (auto& imp : node.import_statements)
        {
            if (should_stop_) return;
            walk(imp.get());
        }
        for (auto& dir : node.directives)
        {
            if (should_stop_) return;
            walk(dir.get());
        }
        for (auto& st : node.struct_definitions)
        {
            if (should_stop_) return;
            walk(st.get());
        }
        for (auto& en : node.enum_definitions)
        {
            if (should_stop_) return;
            walk(en.get());
        }
        for (auto& ta : node.type_aliases)
        {
            if (should_stop_) return;
            walk(ta.get());
        }
        for (auto& fn : node.function_definitions)
        {
            if (should_stop_) return;
            walk(fn.get());
        }
        for (auto& ext : node.extension_definitions)
        {
            if (should_stop_) return;
            walk(ext.get());
        }
        for (auto& step : node.execution_steps)
        {
            if (should_stop_) return;
            walk(step.get());
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, ImportStatement>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, Directive>& node)
    {
        if (node.value) walk(node.value.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, FunctionDefinition>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        for (auto& param : node.parameters)
        {
            if (should_stop_) return;
            walk(param);
        }
        for (auto& ret : node.return_types)
        {
            if (should_stop_) return;
            walk(ret.get());
        }
        for (auto& stmt : node.body)
        {
            if (should_stop_) return;
            walk(stmt.get());
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, StructDefinition>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        for (auto& field : node.fields)
        {
            if (should_stop_) return;
            walk(field);
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, EnumDefinition>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.underlying_type) walk(node.underlying_type.get());
        for (auto& item : node.cases)
        {
            if (should_stop_) return;
            walk(item);
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, TypeAliasDefinition>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.target_type) walk(node.target_type.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, ExtensionDefinition>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.target_type) walk(node.target_type.get());
        for (auto& st : node.struct_definitions)
        {
            if (should_stop_) return;
            walk(st.get());
        }
        for (auto& en : node.enum_definitions)
        {
            if (should_stop_) return;
            walk(en.get());
        }
        for (auto& ta : node.type_aliases)
        {
            if (should_stop_) return;
            walk(ta.get());
        }
        for (auto& fn : node.function_definitions)
        {
            if (should_stop_) return;
            walk(fn.get());
        }
        for (auto& step : node.execution_steps)
        {
            if (should_stop_) return;
            walk(step.get());
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, Assignment>& node)
    {
        for (auto& target : node.targets)
        {
            if (should_stop_) return;
            walk(target);
        }
        if (node.value) walk(node.value.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, Reassignment>& node)
    {
        if (node.target) walk(node.target.get());
        if (node.value) walk(node.value.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, ExpressionStatement>& node)
    {
        if (node.expr) walk(node.expr.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, ReturnStatement>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        for (auto& val : node.values)
        {
            if (should_stop_) return;
            walk(val.get());
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, BinaryExpression>& node)
    {
        if (node.left) walk(node.left.get());
        if (node.right) walk(node.right.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, UnaryExpression>& node)
    {
        if (node.right) walk(node.right.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, GroupingExpression>& node)
    {
        if (node.expression) walk(node.expression.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, ConditionalExpression>& node)
    {
        if (node.condition) walk(node.condition.get());
        if (node.then_branch) walk(node.then_branch.get());
        if (node.else_branch) walk(node.else_branch.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, FunctionCall>& node)
    {
        if (node.target) walk(node.target.get());
        for (auto& arg : node.arguments)
        {
            if (should_stop_) return;
            walk(arg);
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, DictLiteral>& node)
    {
        for (auto& item : node.elements)
        {
            if (should_stop_) return;
            walk(item);
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, TensorLiteral>& node)
    {
        for (auto& elem : node.elements)
        {
            if (should_stop_) return;
            walk(elem.get());
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, TupleLiteral>& node)
    {
        for (auto& elem : node.elements)
        {
            if (should_stop_) return;
            walk(elem.get());
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, BracketAccess>& node)
    {
        if (node.target) walk(node.target.get());
        if (node.index) walk(node.index.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, DotAccess>& node)
    {
        if (node.target) walk(node.target.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, SwitchExpression>& node)
    {
        if (node.target) walk(node.target.get());
        for (auto& sc : node.cases)
        {
            if (should_stop_) return;
            walk(sc);
        }
        for (auto& mod : node.default_modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.default_case) walk(node.default_case.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, TypeAnnotation>& node)
    {
        for (auto& arg : node.generic_args)
        {
            if (should_stop_) return;
            walk(arg.get());
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, TupleTypeAnnotation>& node)
    {
        for (auto& elem : node.element_types)
        {
            if (should_stop_) return;
            walk(elem.get());
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, Modifier>& node)
    {
        for (auto& arg : node.arguments)
        {
            if (should_stop_) return;
            walk(arg);
        }
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, CallArgument>& node)
    {
        if (node.value) walk(node.value.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, FunctionParameter>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.type) walk(node.type.get());
        if (node.default_value) walk(node.default_value.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, StructField>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.type) walk(node.type.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, EnumCase>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.value) walk(node.value.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, AssignmentTarget>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.type) walk(node.type.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, DictItem>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.value) walk(node.value.get());
    }

    template <bool IsConst>
    inline void BasicAstWalker<IsConst>::walk_children(MaybeConst<IsConst, SwitchCase>& node)
    {
        for (auto& mod : node.modifiers)
        {
            if (should_stop_) return;
            walk(mod);
        }
        if (node.result) walk(node.result.get());
    }

    using ConstAstWalker = BasicAstWalker<true>;
    using AstWalker = BasicAstWalker<false>;
}
