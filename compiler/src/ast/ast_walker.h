#pragma once

#include <vector>
#include <span>
#include <memory>
#include <type_traits>
#include "ast_core.h"
#include "ast_type.h"
#include "ast_expr.h"
#include "ast_stmt.h"
#include "ast_decl.h"
#include "ast_node_registry.h"
#include "token/comment_token.h"

namespace valuascript::compiler
{
    enum class TraversalAction : uint8_t
    {
        Continue,     // Process current node and traverse into its children
        SkipChildren, // Process current node, but prune (do not visit children)
        Stop          // Terminate the entire AST traversal immediately
    };

    template <bool IsConst, typename T>
    using MaybeConst = std::conditional_t<IsConst, const T, T>;

    // =========================================================================
    // BasicAstWalker<IsConst>: Unified Read-Only / Mutable AST Traversal
    // =========================================================================
    template <bool IsConst>
    class BasicAstWalker
    {
    public:
        using NodePtr = MaybeConst<IsConst, AstNode>*;
        using ProgramRef = MaybeConst<IsConst, Program>&;

    private:
        std::vector<NodePtr> ancestor_stack_;
        bool should_stop_ = false;

    public:
        virtual ~BasicAstWalker() = default;

        // ---------------------------------------------------------------------
        // Context & Ancestor Stack Introspection
        // ---------------------------------------------------------------------
        [[nodiscard]] NodePtr parent() const noexcept
        {
            if (ancestor_stack_.size() >= 2)
            {
                return ancestor_stack_[ancestor_stack_.size() - 2];
            }
            return nullptr;
        }

        [[nodiscard]] size_t depth() const noexcept
        {
            return ancestor_stack_.empty() ? 0 : ancestor_stack_.size() - 1;
        }

        [[nodiscard]] std::span<NodePtr const> ancestor_stack() const noexcept
        {
            return ancestor_stack_;
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

        [[nodiscard]] MaybeConst<IsConst, FunctionDefinition>* enclosing_function() const noexcept
        {
            return find_ancestor<FunctionDefinition>();
        }

        [[nodiscard]] MaybeConst<IsConst, StructDefinition>* enclosing_struct() const noexcept
        {
            return find_ancestor<StructDefinition>();
        }

        [[nodiscard]] MaybeConst<IsConst, EnumDefinition>* enclosing_enum() const noexcept
        {
            return find_ancestor<EnumDefinition>();
        }

        [[nodiscard]] MaybeConst<IsConst, ExtensionDefinition>* enclosing_extension() const noexcept
        {
            return find_ancestor<ExtensionDefinition>();
        }

        [[nodiscard]] MaybeConst<IsConst, Statement>* enclosing_statement() const noexcept
        {
            return find_ancestor<Statement>();
        }

        [[nodiscard]] MaybeConst<IsConst, SwitchExpression>* enclosing_switch() const noexcept
        {
            return find_ancestor<SwitchExpression>();
        }

        [[nodiscard]] MaybeConst<IsConst, Program>* enclosing_program() const noexcept
        {
            return find_ancestor<Program>();
        }

        // ---------------------------------------------------------------------
        // Universal Lifecycle Hooks (Called for EVERY node)
        // ---------------------------------------------------------------------
        virtual TraversalAction enter_node(MaybeConst<IsConst, AstNode>&) { return TraversalAction::Continue; }
        virtual void leave_node(MaybeConst<IsConst, AstNode>&) {}

        // ---------------------------------------------------------------------
        // Category Hierarchy Hooks
        // ---------------------------------------------------------------------
        virtual TraversalAction enter_declaration(MaybeConst<IsConst, AstNode>&) { return TraversalAction::Continue; }
        virtual void leave_declaration(MaybeConst<IsConst, AstNode>&) {}

        virtual TraversalAction enter_statement(MaybeConst<IsConst, Statement>&) { return TraversalAction::Continue; }
        virtual void leave_statement(MaybeConst<IsConst, Statement>&) {}

        virtual TraversalAction enter_expression(MaybeConst<IsConst, Expression>&) { return TraversalAction::Continue; }
        virtual void leave_expression(MaybeConst<IsConst, Expression>&) {}

        virtual TraversalAction enter_type(MaybeConst<IsConst, TypeAnnotation>&) { return TraversalAction::Continue; }
        virtual void leave_type(MaybeConst<IsConst, TypeAnnotation>&) {}

        // ---------------------------------------------------------------------
        // Non-Node Grammar Struct & Trivia Hooks
        // ---------------------------------------------------------------------
        virtual TraversalAction visit(MaybeConst<IsConst, Modifier>& s) { return visit_modifier(s); }
        virtual TraversalAction visit(MaybeConst<IsConst, CallArgument>& s) { return visit_call_argument(s); }
        virtual TraversalAction visit(MaybeConst<IsConst, FunctionParameter>& s) { return visit_parameter(s); }
        virtual TraversalAction visit(MaybeConst<IsConst, StructField>& s) { return visit_struct_field(s); }
        virtual TraversalAction visit(MaybeConst<IsConst, EnumCase>& s) { return visit_enum_case(s); }
        virtual TraversalAction visit(MaybeConst<IsConst, DictItem>& s) { return visit_dict_item(s); }
        virtual TraversalAction visit(MaybeConst<IsConst, SwitchCase>& s) { return visit_switch_case(s); }
        virtual TraversalAction visit(MaybeConst<IsConst, AssignmentTarget>& s) { return visit_assignment_target(s); }
        virtual TraversalAction visit(MaybeConst<IsConst, CommentToken>& s) { return visit_comment(s); }

        virtual TraversalAction visit_modifier(MaybeConst<IsConst, Modifier>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_call_argument(MaybeConst<IsConst, CallArgument>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_parameter(MaybeConst<IsConst, FunctionParameter>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_struct_field(MaybeConst<IsConst, StructField>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_enum_case(MaybeConst<IsConst, EnumCase>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_dict_item(MaybeConst<IsConst, DictItem>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_switch_case(MaybeConst<IsConst, SwitchCase>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_switch_default_case(MaybeConst<IsConst, ExprPtr>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_assignment_target(MaybeConst<IsConst, AssignmentTarget>&) { return TraversalAction::Continue; }
        virtual TraversalAction visit_comment(MaybeConst<IsConst, CommentToken>&) { return TraversalAction::Continue; }

        // ---------------------------------------------------------------------
        // Concrete Typed Pre-Order & Post-Order Hooks
        // ---------------------------------------------------------------------
        // Top-Level
        virtual TraversalAction enter_program(MaybeConst<IsConst, Program>&) { return TraversalAction::Continue; }
        virtual void leave_program(MaybeConst<IsConst, Program>&) {}

        // Declarations
        virtual TraversalAction enter_import_statement(MaybeConst<IsConst, ImportStatement>&) { return TraversalAction::Continue; }
        virtual void leave_import_statement(MaybeConst<IsConst, ImportStatement>&) {}

        virtual TraversalAction enter_directive(MaybeConst<IsConst, Directive>&) { return TraversalAction::Continue; }
        virtual void leave_directive(MaybeConst<IsConst, Directive>&) {}

        virtual TraversalAction enter_function_definition(MaybeConst<IsConst, FunctionDefinition>&) { return TraversalAction::Continue; }
        virtual void leave_function_definition(MaybeConst<IsConst, FunctionDefinition>&) {}

        virtual TraversalAction enter_struct_definition(MaybeConst<IsConst, StructDefinition>&) { return TraversalAction::Continue; }
        virtual void leave_struct_definition(MaybeConst<IsConst, StructDefinition>&) {}

        virtual TraversalAction enter_enum_definition(MaybeConst<IsConst, EnumDefinition>&) { return TraversalAction::Continue; }
        virtual void leave_enum_definition(MaybeConst<IsConst, EnumDefinition>&) {}

        virtual TraversalAction enter_type_alias_definition(MaybeConst<IsConst, TypeAliasDefinition>&) { return TraversalAction::Continue; }
        virtual void leave_type_alias_definition(MaybeConst<IsConst, TypeAliasDefinition>&) {}

        virtual TraversalAction enter_extension_definition(MaybeConst<IsConst, ExtensionDefinition>&) { return TraversalAction::Continue; }
        virtual void leave_extension_definition(MaybeConst<IsConst, ExtensionDefinition>&) {}

        // Statements
        virtual TraversalAction enter_assignment(MaybeConst<IsConst, Assignment>&) { return TraversalAction::Continue; }
        virtual void leave_assignment(MaybeConst<IsConst, Assignment>&) {}

        virtual TraversalAction enter_reassignment(MaybeConst<IsConst, Reassignment>&) { return TraversalAction::Continue; }
        virtual void leave_reassignment(MaybeConst<IsConst, Reassignment>&) {}

        virtual TraversalAction enter_expression_statement(MaybeConst<IsConst, ExpressionStatement>&) { return TraversalAction::Continue; }
        virtual void leave_expression_statement(MaybeConst<IsConst, ExpressionStatement>&) {}

        virtual TraversalAction enter_return_statement(MaybeConst<IsConst, ReturnStatement>&) { return TraversalAction::Continue; }
        virtual void leave_return_statement(MaybeConst<IsConst, ReturnStatement>&) {}

        // Expressions
        virtual TraversalAction enter_number_literal(MaybeConst<IsConst, NumberLiteral>&) { return TraversalAction::Continue; }
        virtual void leave_number_literal(MaybeConst<IsConst, NumberLiteral>&) {}

        virtual TraversalAction enter_percentage_literal(MaybeConst<IsConst, PercentageLiteral>&) { return TraversalAction::Continue; }
        virtual void leave_percentage_literal(MaybeConst<IsConst, PercentageLiteral>&) {}

        virtual TraversalAction enter_string_literal(MaybeConst<IsConst, StringLiteral>&) { return TraversalAction::Continue; }
        virtual void leave_string_literal(MaybeConst<IsConst, StringLiteral>&) {}

        virtual TraversalAction enter_boolean_literal(MaybeConst<IsConst, BooleanLiteral>&) { return TraversalAction::Continue; }
        virtual void leave_boolean_literal(MaybeConst<IsConst, BooleanLiteral>&) {}

        virtual TraversalAction enter_identifier_access(MaybeConst<IsConst, IdentifierAccess>&) { return TraversalAction::Continue; }
        virtual void leave_identifier_access(MaybeConst<IsConst, IdentifierAccess>&) {}

        virtual TraversalAction enter_self_expression(MaybeConst<IsConst, SelfExpression>&) { return TraversalAction::Continue; }
        virtual void leave_self_expression(MaybeConst<IsConst, SelfExpression>&) {}

        virtual TraversalAction enter_binary_expression(MaybeConst<IsConst, BinaryExpression>&) { return TraversalAction::Continue; }
        virtual void leave_binary_expression(MaybeConst<IsConst, BinaryExpression>&) {}

        virtual TraversalAction enter_unary_expression(MaybeConst<IsConst, UnaryExpression>&) { return TraversalAction::Continue; }
        virtual void leave_unary_expression(MaybeConst<IsConst, UnaryExpression>&) {}

        virtual TraversalAction enter_grouping_expression(MaybeConst<IsConst, GroupingExpression>&) { return TraversalAction::Continue; }
        virtual void leave_grouping_expression(MaybeConst<IsConst, GroupingExpression>&) {}

        virtual TraversalAction enter_conditional_expression(MaybeConst<IsConst, ConditionalExpression>&) { return TraversalAction::Continue; }
        virtual void leave_conditional_expression(MaybeConst<IsConst, ConditionalExpression>&) {}

        virtual TraversalAction enter_function_call(MaybeConst<IsConst, FunctionCall>&) { return TraversalAction::Continue; }
        virtual void leave_function_call(MaybeConst<IsConst, FunctionCall>&) {}

        virtual TraversalAction enter_dict_literal(MaybeConst<IsConst, DictLiteral>&) { return TraversalAction::Continue; }
        virtual void leave_dict_literal(MaybeConst<IsConst, DictLiteral>&) {}

        virtual TraversalAction enter_tensor_literal(MaybeConst<IsConst, TensorLiteral>&) { return TraversalAction::Continue; }
        virtual void leave_tensor_literal(MaybeConst<IsConst, TensorLiteral>&) {}

        virtual TraversalAction enter_tuple_literal(MaybeConst<IsConst, TupleLiteral>&) { return TraversalAction::Continue; }
        virtual void leave_tuple_literal(MaybeConst<IsConst, TupleLiteral>&) {}

        virtual TraversalAction enter_bracket_access(MaybeConst<IsConst, BracketAccess>&) { return TraversalAction::Continue; }
        virtual void leave_bracket_access(MaybeConst<IsConst, BracketAccess>&) {}

        virtual TraversalAction enter_dot_access(MaybeConst<IsConst, DotAccess>&) { return TraversalAction::Continue; }
        virtual void leave_dot_access(MaybeConst<IsConst, DotAccess>&) {}

        virtual TraversalAction enter_switch_expression(MaybeConst<IsConst, SwitchExpression>&) { return TraversalAction::Continue; }
        virtual void leave_switch_expression(MaybeConst<IsConst, SwitchExpression>&) {}

        // Types
        virtual TraversalAction enter_type_annotation(MaybeConst<IsConst, TypeAnnotation>&) { return TraversalAction::Continue; }
        virtual void leave_type_annotation(MaybeConst<IsConst, TypeAnnotation>&) {}

        virtual TraversalAction enter_tuple_type_annotation(MaybeConst<IsConst, TupleTypeAnnotation>&) { return TraversalAction::Continue; }
        virtual void leave_tuple_type_annotation(MaybeConst<IsConst, TupleTypeAnnotation>&) {}

        // ---------------------------------------------------------------------
        // Overridable Children Traversal
        // ---------------------------------------------------------------------
        virtual void walk_children(MaybeConst<IsConst, Program>& node)
        {
            for (auto& comment : node.comments)
            {
                if (should_stop_) return;
                if (visit_comment(comment) == TraversalAction::Stop) { should_stop_ = true; return; }
            }
            for (auto& imp : node.import_statements) { if (should_stop_) return; walk(imp.get()); }
            for (auto& dir : node.directives) { if (should_stop_) return; walk(dir.get()); }
            for (auto& st : node.struct_definitions) { if (should_stop_) return; walk(st.get()); }
            for (auto& en : node.enum_definitions) { if (should_stop_) return; walk(en.get()); }
            for (auto& ta : node.type_aliases) { if (should_stop_) return; walk(ta.get()); }
            for (auto& fn : node.function_definitions) { if (should_stop_) return; walk(fn.get()); }
            for (auto& ext : node.extension_definitions) { if (should_stop_) return; walk(ext.get()); }
            for (auto& step : node.execution_steps) { if (should_stop_) return; walk(step.get()); }
        }

        virtual void walk_children(MaybeConst<IsConst, ImportStatement>& node)
        {
            for (auto& mod : node.modifiers) { if (should_stop_) return; walk_modifier(mod); }
        }

        virtual void walk_children(MaybeConst<IsConst, Directive>& node)
        {
            if (node.value) walk(node.value.get());
        }

        virtual void walk_children(MaybeConst<IsConst, FunctionDefinition>& node)
        {
            for (auto& mod : node.modifiers) { if (should_stop_) return; walk_modifier(mod); }
            for (auto& param : node.parameters)
            {
                if (should_stop_) return;
                if (visit_parameter(param) == TraversalAction::Stop) { should_stop_ = true; return; }
                for (auto& mod : param.modifiers) { if (should_stop_) return; walk_modifier(mod); }
                if (param.type) walk(param.type.get());
                if (param.default_value) walk(param.default_value.get());
            }
            for (auto& ret : node.return_types) { if (should_stop_) return; walk(ret.get()); }
            for (auto& stmt : node.body) { if (should_stop_) return; walk(stmt.get()); }
        }

        virtual void walk_children(MaybeConst<IsConst, StructDefinition>& node)
        {
            for (auto& mod : node.modifiers) { if (should_stop_) return; walk_modifier(mod); }
            for (auto& field : node.fields)
            {
                if (should_stop_) return;
                if (visit_struct_field(field) == TraversalAction::Stop) { should_stop_ = true; return; }
                for (auto& mod : field.modifiers) { if (should_stop_) return; walk_modifier(mod); }
                if (field.type) walk(field.type.get());
            }
        }

        virtual void walk_children(MaybeConst<IsConst, EnumDefinition>& node)
        {
            for (auto& mod : node.modifiers) { if (should_stop_) return; walk_modifier(mod); }
            if (node.underlying_type) walk(node.underlying_type.get());
            for (auto& enum_case : node.cases)
            {
                if (should_stop_) return;
                if (visit_enum_case(enum_case) == TraversalAction::Stop) { should_stop_ = true; return; }
                for (auto& mod : enum_case.modifiers) { if (should_stop_) return; walk_modifier(mod); }
                if (enum_case.value) walk(enum_case.value.get());
            }
        }

        virtual void walk_children(MaybeConst<IsConst, TypeAliasDefinition>& node)
        {
            for (auto& mod : node.modifiers) { if (should_stop_) return; walk_modifier(mod); }
            if (node.target_type) walk(node.target_type.get());
        }

        virtual void walk_children(MaybeConst<IsConst, ExtensionDefinition>& node)
        {
            for (auto& mod : node.modifiers) { if (should_stop_) return; walk_modifier(mod); }
            if (node.target_type) walk(node.target_type.get());
            for (auto& st : node.struct_definitions) { if (should_stop_) return; walk(st.get()); }
            for (auto& en : node.enum_definitions) { if (should_stop_) return; walk(en.get()); }
            for (auto& ta : node.type_aliases) { if (should_stop_) return; walk(ta.get()); }
            for (auto& fn : node.function_definitions) { if (should_stop_) return; walk(fn.get()); }
            for (auto& step : node.execution_steps) { if (should_stop_) return; walk(step.get()); }
        }

        virtual void walk_children(MaybeConst<IsConst, Assignment>& node)
        {
            for (auto& target : node.targets)
            {
                if (should_stop_) return;
                if (visit_assignment_target(target) == TraversalAction::Stop) { should_stop_ = true; return; }
                for (auto& mod : target.modifiers) { if (should_stop_) return; walk_modifier(mod); }
                if (target.type) walk(target.type.get());
            }
            if (node.value) walk(node.value.get());
        }

        virtual void walk_children(MaybeConst<IsConst, Reassignment>& node)
        {
            if (node.target) walk(node.target.get());
            if (node.value) walk(node.value.get());
        }

        virtual void walk_children(MaybeConst<IsConst, ExpressionStatement>& node)
        {
            if (node.expr) walk(node.expr.get());
        }

        virtual void walk_children(MaybeConst<IsConst, ReturnStatement>& node)
        {
            for (auto& mod : node.modifiers) { if (should_stop_) return; walk_modifier(mod); }
            for (auto& val : node.values) { if (should_stop_) return; walk(val.get()); }
        }

        virtual void walk_children(MaybeConst<IsConst, BinaryExpression>& node)
        {
            if (node.left) walk(node.left.get());
            if (node.right) walk(node.right.get());
        }

        virtual void walk_children(MaybeConst<IsConst, UnaryExpression>& node)
        {
            if (node.right) walk(node.right.get());
        }

        virtual void walk_children(MaybeConst<IsConst, GroupingExpression>& node)
        {
            if (node.expression) walk(node.expression.get());
        }

        virtual void walk_children(MaybeConst<IsConst, ConditionalExpression>& node)
        {
            if (node.condition) walk(node.condition.get());
            if (node.then_branch) walk(node.then_branch.get());
            if (node.else_branch) walk(node.else_branch.get());
        }

        virtual void walk_children(MaybeConst<IsConst, FunctionCall>& node)
        {
            if (node.target) walk(node.target.get());
            for (auto& arg : node.arguments)
            {
                if (should_stop_) return;
                if (visit_call_argument(arg) == TraversalAction::Stop) { should_stop_ = true; return; }
                if (arg.value) walk(arg.value.get());
            }
        }

        virtual void walk_children(MaybeConst<IsConst, DictLiteral>& node)
        {
            for (auto& item : node.elements)
            {
                if (should_stop_) return;
                if (visit_dict_item(item) == TraversalAction::Stop) { should_stop_ = true; return; }
                for (auto& mod : item.modifiers) { if (should_stop_) return; walk_modifier(mod); }
                if (item.value) walk(item.value.get());
            }
        }

        virtual void walk_children(MaybeConst<IsConst, TensorLiteral>& node)
        {
            for (auto& elem : node.elements) { if (should_stop_) return; walk(elem.get()); }
        }

        virtual void walk_children(MaybeConst<IsConst, TupleLiteral>& node)
        {
            for (auto& elem : node.elements) { if (should_stop_) return; walk(elem.get()); }
        }

        virtual void walk_children(MaybeConst<IsConst, BracketAccess>& node)
        {
            if (node.target) walk(node.target.get());
            if (node.index) walk(node.index.get());
        }

        virtual void walk_children(MaybeConst<IsConst, DotAccess>& node)
        {
            if (node.target) walk(node.target.get());
        }

        virtual void walk_children(MaybeConst<IsConst, SwitchExpression>& node)
        {
            if (node.target) walk(node.target.get());
            for (auto& sc : node.cases)
            {
                if (should_stop_) return;
                if (visit_switch_case(sc) == TraversalAction::Stop) { should_stop_ = true; return; }
                for (auto& mod : sc.modifiers) { if (should_stop_) return; walk_modifier(mod); }
                if (sc.result) walk(sc.result.get());
            }
            for (auto& def_mod : node.default_modifiers)
            {
                if (should_stop_) return;
                walk_modifier(def_mod);
            }
            if (node.default_case)
            {
                if (visit_switch_default_case(node.default_case) == TraversalAction::Stop) { should_stop_ = true; return; }
                walk(node.default_case.get());
            }
        }

        virtual void walk_children(MaybeConst<IsConst, TypeAnnotation>& node)
        {
            for (auto& arg : node.generic_args) { if (should_stop_) return; walk(arg.get()); }
        }

        virtual void walk_children(MaybeConst<IsConst, NumberLiteral>&) {}
        virtual void walk_children(MaybeConst<IsConst, PercentageLiteral>&) {}
        virtual void walk_children(MaybeConst<IsConst, StringLiteral>&) {}
        virtual void walk_children(MaybeConst<IsConst, BooleanLiteral>&) {}
        virtual void walk_children(MaybeConst<IsConst, IdentifierAccess>&) {}
        virtual void walk_children(MaybeConst<IsConst, SelfExpression>&) {}

        virtual void walk_children(MaybeConst<IsConst, TupleTypeAnnotation>& node)
        {
            for (auto& elem : node.element_types) { if (should_stop_) return; walk(elem.get()); }
        }

        // ---------------------------------------------------------------------
        // Entry Points
        // ---------------------------------------------------------------------
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

            // 1. Universal node pre-hook
            if (enter_node(*node) == TraversalAction::Stop)
            {
                should_stop_ = true;
                ancestor_stack_.pop_back();
                return;
            }

            // 2. Category & Typed Dispatch
            TraversalAction action = dispatch_enter(*node);
            if (action == TraversalAction::Stop)
            {
                should_stop_ = true;
                ancestor_stack_.pop_back();
                return;
            }

            // 3. Children Traversal (if not pruned)
            if (action == TraversalAction::Continue && !should_stop_)
            {
                dispatch_children(*node);
            }

            // 4. Category & Typed Post-Hooks
            if (!should_stop_)
            {
                dispatch_leave(*node);
                leave_node(*node);
            }

            ancestor_stack_.pop_back();
        }

    private:
        void walk_modifier(MaybeConst<IsConst, Modifier>& mod)
        {
            if (visit_modifier(mod) == TraversalAction::Stop) { should_stop_ = true; return; }
            for (auto& arg : mod.arguments)
            {
                if (should_stop_) return;
                if (visit_call_argument(arg) == TraversalAction::Stop) { should_stop_ = true; return; }
                if (arg.value) walk(arg.value.get());
            }
        }

        TraversalAction dispatch_enter(MaybeConst<IsConst, AstNode>& node)
        {
            using StmtType = MaybeConst<IsConst, Statement>;
            using ExprType = MaybeConst<IsConst, Expression>;
            using TypeAnnType = MaybeConst<IsConst, TypeAnnotation>;

            switch (node.kind)
            {
                case AstKind::Program:
                    return enter_program(static_cast<MaybeConst<IsConst, Program>&>(node));

                // Declarations
                case AstKind::ImportStatement:
                    if (enter_declaration(node) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_import_statement(static_cast<MaybeConst<IsConst, ImportStatement>&>(node));
                case AstKind::Directive:
                    if (enter_declaration(node) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_directive(static_cast<MaybeConst<IsConst, Directive>&>(node));
                case AstKind::FunctionDefinition:
                    if (enter_declaration(node) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_function_definition(static_cast<MaybeConst<IsConst, FunctionDefinition>&>(node));
                case AstKind::StructDefinition:
                    if (enter_declaration(node) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_struct_definition(static_cast<MaybeConst<IsConst, StructDefinition>&>(node));
                case AstKind::EnumDefinition:
                    if (enter_declaration(node) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_enum_definition(static_cast<MaybeConst<IsConst, EnumDefinition>&>(node));
                case AstKind::TypeAliasDefinition:
                    if (enter_declaration(node) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_type_alias_definition(static_cast<MaybeConst<IsConst, TypeAliasDefinition>&>(node));
                case AstKind::ExtensionDefinition:
                    if (enter_declaration(node) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_extension_definition(static_cast<MaybeConst<IsConst, ExtensionDefinition>&>(node));

                // Statements
                case AstKind::Assignment:
                    if (enter_statement(static_cast<StmtType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_assignment(static_cast<MaybeConst<IsConst, Assignment>&>(node));
                case AstKind::Reassignment:
                    if (enter_statement(static_cast<StmtType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_reassignment(static_cast<MaybeConst<IsConst, Reassignment>&>(node));
                case AstKind::ExpressionStatement:
                    if (enter_statement(static_cast<StmtType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_expression_statement(static_cast<MaybeConst<IsConst, ExpressionStatement>&>(node));
                case AstKind::ReturnStatement:
                    if (enter_statement(static_cast<StmtType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_return_statement(static_cast<MaybeConst<IsConst, ReturnStatement>&>(node));

                // Expressions
                case AstKind::NumberLiteral:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_number_literal(static_cast<MaybeConst<IsConst, NumberLiteral>&>(node));
                case AstKind::PercentageLiteral:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_percentage_literal(static_cast<MaybeConst<IsConst, PercentageLiteral>&>(node));
                case AstKind::StringLiteral:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_string_literal(static_cast<MaybeConst<IsConst, StringLiteral>&>(node));
                case AstKind::BooleanLiteral:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_boolean_literal(static_cast<MaybeConst<IsConst, BooleanLiteral>&>(node));
                case AstKind::IdentifierAccess:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_identifier_access(static_cast<MaybeConst<IsConst, IdentifierAccess>&>(node));
                case AstKind::SelfExpression:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_self_expression(static_cast<MaybeConst<IsConst, SelfExpression>&>(node));
                case AstKind::BinaryExpression:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_binary_expression(static_cast<MaybeConst<IsConst, BinaryExpression>&>(node));
                case AstKind::UnaryExpression:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_unary_expression(static_cast<MaybeConst<IsConst, UnaryExpression>&>(node));
                case AstKind::GroupingExpression:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_grouping_expression(static_cast<MaybeConst<IsConst, GroupingExpression>&>(node));
                case AstKind::ConditionalExpression:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_conditional_expression(static_cast<MaybeConst<IsConst, ConditionalExpression>&>(node));
                case AstKind::FunctionCall:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_function_call(static_cast<MaybeConst<IsConst, FunctionCall>&>(node));
                case AstKind::DictLiteral:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_dict_literal(static_cast<MaybeConst<IsConst, DictLiteral>&>(node));
                case AstKind::TensorLiteral:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_tensor_literal(static_cast<MaybeConst<IsConst, TensorLiteral>&>(node));
                case AstKind::TupleLiteral:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_tuple_literal(static_cast<MaybeConst<IsConst, TupleLiteral>&>(node));
                case AstKind::BracketAccess:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_bracket_access(static_cast<MaybeConst<IsConst, BracketAccess>&>(node));
                case AstKind::DotAccess:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_dot_access(static_cast<MaybeConst<IsConst, DotAccess>&>(node));
                case AstKind::SwitchExpression:
                    if (enter_expression(static_cast<ExprType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_switch_expression(static_cast<MaybeConst<IsConst, SwitchExpression>&>(node));

                // Types
                case AstKind::TupleTypeAnnotation:
                    if (enter_type(static_cast<TypeAnnType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_tuple_type_annotation(static_cast<MaybeConst<IsConst, TupleTypeAnnotation>&>(node));
                case AstKind::TypeAnnotation:
                    if (enter_type(static_cast<TypeAnnType&>(node)) == TraversalAction::Stop) return TraversalAction::Stop;
                    return enter_type_annotation(static_cast<MaybeConst<IsConst, TypeAnnotation>&>(node));
                case AstKind::Unknown:
                    break;
            }
            return TraversalAction::Continue;
        }

        void dispatch_children(MaybeConst<IsConst, AstNode>& node)
        {
            switch (node.kind)
            {
                case AstKind::Program: walk_children(static_cast<MaybeConst<IsConst, Program>&>(node)); break;
                case AstKind::ImportStatement: walk_children(static_cast<MaybeConst<IsConst, ImportStatement>&>(node)); break;
                case AstKind::Directive: walk_children(static_cast<MaybeConst<IsConst, Directive>&>(node)); break;
                case AstKind::FunctionDefinition: walk_children(static_cast<MaybeConst<IsConst, FunctionDefinition>&>(node)); break;
                case AstKind::StructDefinition: walk_children(static_cast<MaybeConst<IsConst, StructDefinition>&>(node)); break;
                case AstKind::EnumDefinition: walk_children(static_cast<MaybeConst<IsConst, EnumDefinition>&>(node)); break;
                case AstKind::TypeAliasDefinition: walk_children(static_cast<MaybeConst<IsConst, TypeAliasDefinition>&>(node)); break;
                case AstKind::ExtensionDefinition: walk_children(static_cast<MaybeConst<IsConst, ExtensionDefinition>&>(node)); break;
                case AstKind::Assignment: walk_children(static_cast<MaybeConst<IsConst, Assignment>&>(node)); break;
                case AstKind::Reassignment: walk_children(static_cast<MaybeConst<IsConst, Reassignment>&>(node)); break;
                case AstKind::ExpressionStatement: walk_children(static_cast<MaybeConst<IsConst, ExpressionStatement>&>(node)); break;
                case AstKind::ReturnStatement: walk_children(static_cast<MaybeConst<IsConst, ReturnStatement>&>(node)); break;
                case AstKind::NumberLiteral: walk_children(static_cast<MaybeConst<IsConst, NumberLiteral>&>(node)); break;
                case AstKind::PercentageLiteral: walk_children(static_cast<MaybeConst<IsConst, PercentageLiteral>&>(node)); break;
                case AstKind::StringLiteral: walk_children(static_cast<MaybeConst<IsConst, StringLiteral>&>(node)); break;
                case AstKind::BooleanLiteral: walk_children(static_cast<MaybeConst<IsConst, BooleanLiteral>&>(node)); break;
                case AstKind::IdentifierAccess: walk_children(static_cast<MaybeConst<IsConst, IdentifierAccess>&>(node)); break;
                case AstKind::SelfExpression: walk_children(static_cast<MaybeConst<IsConst, SelfExpression>&>(node)); break;
                case AstKind::BinaryExpression: walk_children(static_cast<MaybeConst<IsConst, BinaryExpression>&>(node)); break;
                case AstKind::UnaryExpression: walk_children(static_cast<MaybeConst<IsConst, UnaryExpression>&>(node)); break;
                case AstKind::GroupingExpression: walk_children(static_cast<MaybeConst<IsConst, GroupingExpression>&>(node)); break;
                case AstKind::ConditionalExpression: walk_children(static_cast<MaybeConst<IsConst, ConditionalExpression>&>(node)); break;
                case AstKind::FunctionCall: walk_children(static_cast<MaybeConst<IsConst, FunctionCall>&>(node)); break;
                case AstKind::DictLiteral: walk_children(static_cast<MaybeConst<IsConst, DictLiteral>&>(node)); break;
                case AstKind::TensorLiteral: walk_children(static_cast<MaybeConst<IsConst, TensorLiteral>&>(node)); break;
                case AstKind::TupleLiteral: walk_children(static_cast<MaybeConst<IsConst, TupleLiteral>&>(node)); break;
                case AstKind::BracketAccess: walk_children(static_cast<MaybeConst<IsConst, BracketAccess>&>(node)); break;
                case AstKind::DotAccess: walk_children(static_cast<MaybeConst<IsConst, DotAccess>&>(node)); break;
                case AstKind::SwitchExpression: walk_children(static_cast<MaybeConst<IsConst, SwitchExpression>&>(node)); break;
                case AstKind::TypeAnnotation: walk_children(static_cast<MaybeConst<IsConst, TypeAnnotation>&>(node)); break;
                case AstKind::TupleTypeAnnotation: walk_children(static_cast<MaybeConst<IsConst, TupleTypeAnnotation>&>(node)); break;
                case AstKind::Unknown: break;
            }
        }

        void dispatch_leave(MaybeConst<IsConst, AstNode>& node)
        {
            using StmtType = MaybeConst<IsConst, Statement>;
            using ExprType = MaybeConst<IsConst, Expression>;
            using TypeAnnType = MaybeConst<IsConst, TypeAnnotation>;

            switch (node.kind)
            {
                case AstKind::Program: leave_program(static_cast<MaybeConst<IsConst, Program>&>(node)); break;
                case AstKind::ImportStatement:
                    leave_import_statement(static_cast<MaybeConst<IsConst, ImportStatement>&>(node));
                    leave_declaration(node);
                    break;
                case AstKind::Directive:
                    leave_directive(static_cast<MaybeConst<IsConst, Directive>&>(node));
                    leave_declaration(node);
                    break;
                case AstKind::FunctionDefinition:
                    leave_function_definition(static_cast<MaybeConst<IsConst, FunctionDefinition>&>(node));
                    leave_declaration(node);
                    break;
                case AstKind::StructDefinition:
                    leave_struct_definition(static_cast<MaybeConst<IsConst, StructDefinition>&>(node));
                    leave_declaration(node);
                    break;
                case AstKind::EnumDefinition:
                    leave_enum_definition(static_cast<MaybeConst<IsConst, EnumDefinition>&>(node));
                    leave_declaration(node);
                    break;
                case AstKind::TypeAliasDefinition:
                    leave_type_alias_definition(static_cast<MaybeConst<IsConst, TypeAliasDefinition>&>(node));
                    leave_declaration(node);
                    break;
                case AstKind::ExtensionDefinition:
                    leave_extension_definition(static_cast<MaybeConst<IsConst, ExtensionDefinition>&>(node));
                    leave_declaration(node);
                    break;
                case AstKind::Assignment:
                    leave_assignment(static_cast<MaybeConst<IsConst, Assignment>&>(node));
                    leave_statement(static_cast<StmtType&>(node));
                    break;
                case AstKind::Reassignment:
                    leave_reassignment(static_cast<MaybeConst<IsConst, Reassignment>&>(node));
                    leave_statement(static_cast<StmtType&>(node));
                    break;
                case AstKind::ExpressionStatement:
                    leave_expression_statement(static_cast<MaybeConst<IsConst, ExpressionStatement>&>(node));
                    leave_statement(static_cast<StmtType&>(node));
                    break;
                case AstKind::ReturnStatement:
                    leave_return_statement(static_cast<MaybeConst<IsConst, ReturnStatement>&>(node));
                    leave_statement(static_cast<StmtType&>(node));
                    break;
                case AstKind::NumberLiteral:
                    leave_number_literal(static_cast<MaybeConst<IsConst, NumberLiteral>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::PercentageLiteral:
                    leave_percentage_literal(static_cast<MaybeConst<IsConst, PercentageLiteral>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::StringLiteral:
                    leave_string_literal(static_cast<MaybeConst<IsConst, StringLiteral>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::BooleanLiteral:
                    leave_boolean_literal(static_cast<MaybeConst<IsConst, BooleanLiteral>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::IdentifierAccess:
                    leave_identifier_access(static_cast<MaybeConst<IsConst, IdentifierAccess>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::SelfExpression:
                    leave_self_expression(static_cast<MaybeConst<IsConst, SelfExpression>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::BinaryExpression:
                    leave_binary_expression(static_cast<MaybeConst<IsConst, BinaryExpression>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::UnaryExpression:
                    leave_unary_expression(static_cast<MaybeConst<IsConst, UnaryExpression>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::GroupingExpression:
                    leave_grouping_expression(static_cast<MaybeConst<IsConst, GroupingExpression>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::ConditionalExpression:
                    leave_conditional_expression(static_cast<MaybeConst<IsConst, ConditionalExpression>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::FunctionCall:
                    leave_function_call(static_cast<MaybeConst<IsConst, FunctionCall>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::DictLiteral:
                    leave_dict_literal(static_cast<MaybeConst<IsConst, DictLiteral>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::TensorLiteral:
                    leave_tensor_literal(static_cast<MaybeConst<IsConst, TensorLiteral>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::TupleLiteral:
                    leave_tuple_literal(static_cast<MaybeConst<IsConst, TupleLiteral>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::BracketAccess:
                    leave_bracket_access(static_cast<MaybeConst<IsConst, BracketAccess>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::DotAccess:
                    leave_dot_access(static_cast<MaybeConst<IsConst, DotAccess>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::SwitchExpression:
                    leave_switch_expression(static_cast<MaybeConst<IsConst, SwitchExpression>&>(node));
                    leave_expression(static_cast<ExprType&>(node));
                    break;
                case AstKind::TupleTypeAnnotation:
                    leave_tuple_type_annotation(static_cast<MaybeConst<IsConst, TupleTypeAnnotation>&>(node));
                    leave_type(static_cast<TypeAnnType&>(node));
                    break;
                case AstKind::TypeAnnotation:
                    leave_type_annotation(static_cast<MaybeConst<IsConst, TypeAnnotation>&>(node));
                    leave_type(static_cast<TypeAnnType&>(node));
                    break;
                case AstKind::Unknown:
                    break;
            }
        }
    };

    // Concrete standard aliases
    using ConstAstWalker = BasicAstWalker<true>;
    using AstWalker      = BasicAstWalker<false>;

    // =========================================================================
    // Compile-Time Exhaustiveness Static Checks
    // =========================================================================
    template <typename Walker, typename Tuple>
    struct ValidateWalkerCompleteness;

    template <typename Walker, typename... Types>
    struct ValidateWalkerCompleteness<Walker, std::tuple<Types...>>
    {
        static constexpr bool value = (
            (requires(Walker& w, MaybeConst<std::is_const_v<std::remove_pointer_t<typename Walker::NodePtr>>, Types>& n) {
                { w.walk_children(n) } -> std::same_as<void>;
            }) && ...
        );
    };

    static_assert(ValidateWalkerCompleteness<ConstAstWalker, AllAstNodeTypes>::value,
        "ConstAstWalker is missing walk_children overloads for one or more registered AST node types");
    static_assert(ValidateWalkerCompleteness<AstWalker, AllAstNodeTypes>::value,
        "AstWalker is missing walk_children overloads for one or more registered AST node types");

    // Grammar struct hooks verification (via fold expression on master registry AllGrammarStructs)
    template <typename Walker, typename Tuple>
    struct ValidateGrammarStructVisitor;

    template <typename Walker, typename... Structs>
    struct ValidateGrammarStructVisitor<Walker, std::tuple<Structs...>>
    {
        static constexpr bool value = (
            (requires(Walker& w, MaybeConst<std::is_const_v<std::remove_pointer_t<typename Walker::NodePtr>>, Structs>& s) {
                { w.visit(s) } -> std::same_as<TraversalAction>;
            }) && ...
        );
    };

    static_assert(ValidateGrammarStructVisitor<ConstAstWalker, AllGrammarStructs>::value,
        "ConstAstWalker is missing a visit(T&) overload for one or more registered grammar structs");
    static_assert(ValidateGrammarStructVisitor<AstWalker, AllGrammarStructs>::value,
        "AstWalker is missing a visit(T&) overload for one or more registered grammar structs");

    // Category hooks verification
    static_assert(requires(ConstAstWalker& w, const AstNode& n) { { w.enter_declaration(n) } -> std::same_as<TraversalAction>; { w.leave_declaration(n) } -> std::same_as<void>; });
    static_assert(requires(ConstAstWalker& w, const Statement& s) { { w.enter_statement(s) } -> std::same_as<TraversalAction>; { w.leave_statement(s) } -> std::same_as<void>; });
    static_assert(requires(ConstAstWalker& w, const Expression& e) { { w.enter_expression(e) } -> std::same_as<TraversalAction>; { w.leave_expression(e) } -> std::same_as<void>; });
    static_assert(requires(ConstAstWalker& w, const TypeAnnotation& t) { { w.enter_type(t) } -> std::same_as<TraversalAction>; { w.leave_type(t) } -> std::same_as<void>; });
}
