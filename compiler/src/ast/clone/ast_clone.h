#pragma once

#include <memory>
#include <vector>
#include <concepts>
#include <tuple>
#include <utility>
#include <type_traits>

#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/core/ast_concepts.h"
#include "ast/walker/ast_walker_concepts.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/equality/ast_equality.h"
#include "ast/equality/ast_disjoint.h"
#include "ast/utils/ast_cast.h"

namespace valuascript::compiler
{
    template <AstElement T>
    [[nodiscard]] inline std::unique_ptr<T> clone_node(const T* node);

    template <AstElement T>
    [[nodiscard]] inline std::unique_ptr<T> clone_node(const std::unique_ptr<T>& ptr);

    template <typename T>
    [[nodiscard]] inline std::vector<T> clone_nodes(const std::vector<T>& vec);

    template <AstElement T>
    [[nodiscard]] inline std::vector<std::unique_ptr<T>> clone_nodes(const std::vector<std::unique_ptr<T>>& vec);

    [[nodiscard]] inline Comment clone_node(const Comment& comment);
    [[nodiscard]] inline CallArgument clone_node(const CallArgument& arg);
    [[nodiscard]] inline Modifier clone_node(const Modifier& mod);
    [[nodiscard]] inline FunctionParameter clone_node(const FunctionParameter& param);
    [[nodiscard]] inline StructField clone_node(const StructField& field);
    [[nodiscard]] inline EnumCase clone_node(const EnumCase& item);
    [[nodiscard]] inline DictItem clone_node(const DictItem& item);
    [[nodiscard]] inline SwitchCase clone_node(const SwitchCase& item);
    [[nodiscard]] inline AssignmentTarget clone_node(const AssignmentTarget& target);

    namespace detail
    {
        template <typename MemberT>
        inline auto clone_member(const MemberT& member)
        {
            if constexpr (IsUniquePtrOfAstNode<MemberT>)
            {
                return clone_node(member.get());
            }
            else if constexpr (IsVectorOfUniquePtrOfAstNode<MemberT>)
            {
                return clone_nodes(member);
            }
            else if constexpr (IsInnerAstNode<MemberT>)
            {
                return clone_node(member);
            }
            else if constexpr (IsVectorOfInnerAstNode<MemberT>)
            {
                return clone_nodes(member);
            }
            else if constexpr (IsOptionalAstFieldOfAstNode<MemberT>)
            {
                if (member.has_value())
                {
                    using ValT = typename MemberT::value_type;
                    if constexpr (is_unique_ptr_v<ValT>)
                    {
                        return MemberT(clone_node(member.get()));
                    }
                    else
                    {
                        return MemberT(clone_node(member.value()));
                    }
                }
                return MemberT{};
            }
            else
            {
                return member;
            }
        }

        template <typename NodeT>
        inline std::unique_ptr<NodeT> clone_node_reflected(const NodeT* node)
        {
            if (!node) return nullptr;
            auto copy = std::make_unique<NodeT>();
            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                (((*copy).*std::get<Is>(AstNodeSchema<NodeT>::members) = clone_member(node->*std::get<Is>(AstNodeSchema<NodeT>::members))), ...);
            }(std::make_index_sequence<N>{});
            return copy;
        }

        template <typename NodeT>
        requires valuascript::shared::tuple_contains_type_v<NodeT, AllInnerNodeTypes>
        inline NodeT clone_inner_node_reflected(const NodeT& node)
        {
            NodeT copy{};
            constexpr size_t N = std::tuple_size_v<decltype(AstNodeSchema<NodeT>::members)>;
            [&]<size_t... Is>(std::index_sequence<Is...>) {
                ((copy.*std::get<Is>(AstNodeSchema<NodeT>::members) = clone_member(node.*std::get<Is>(AstNodeSchema<NodeT>::members))), ...);
            }(std::make_index_sequence<N>{});
            return copy;
        }

        [[nodiscard]] inline ExprPtr clone_expression(const Expression* expr)
        {
            if (!expr) return nullptr;
            switch (expr->kind)
            {
            case AstKind::NumberLiteral: return clone_node_reflected(static_cast<const NumberLiteral*>(expr));
            case AstKind::PercentageLiteral: return clone_node_reflected(static_cast<const PercentageLiteral*>(expr));
            case AstKind::StringLiteral: return clone_node_reflected(static_cast<const StringLiteral*>(expr));
            case AstKind::BooleanLiteral: return clone_node_reflected(static_cast<const BooleanLiteral*>(expr));
            case AstKind::IdentifierAccess: return clone_node_reflected(static_cast<const IdentifierAccess*>(expr));
            case AstKind::SelfExpression: return clone_node_reflected(static_cast<const SelfExpression*>(expr));
            case AstKind::UnaryExpression: return clone_node_reflected(static_cast<const UnaryExpression*>(expr));
            case AstKind::BinaryExpression: return clone_node_reflected(static_cast<const BinaryExpression*>(expr));
            case AstKind::GroupingExpression: return clone_node_reflected(static_cast<const GroupingExpression*>(expr));
            case AstKind::ConditionalExpression: return clone_node_reflected(static_cast<const ConditionalExpression*>(expr));
            case AstKind::FunctionCall: return clone_node_reflected(static_cast<const FunctionCall*>(expr));
            case AstKind::DictLiteral: return clone_node_reflected(static_cast<const DictLiteral*>(expr));
            case AstKind::TensorLiteral: return clone_node_reflected(static_cast<const TensorLiteral*>(expr));
            case AstKind::TupleLiteral: return clone_node_reflected(static_cast<const TupleLiteral*>(expr));
            case AstKind::BracketAccess: return clone_node_reflected(static_cast<const BracketAccess*>(expr));
            case AstKind::DotAccess: return clone_node_reflected(static_cast<const DotAccess*>(expr));
            case AstKind::SwitchExpression: return clone_node_reflected(static_cast<const SwitchExpression*>(expr));
            default: return nullptr;
            }
        }

        [[nodiscard]] inline StmtPtr clone_statement(const Statement* stmt)
        {
            if (!stmt) return nullptr;
            switch (stmt->kind)
            {
            case AstKind::Assignment: return clone_node_reflected(static_cast<const Assignment*>(stmt));
            case AstKind::Reassignment: return clone_node_reflected(static_cast<const Reassignment*>(stmt));
            case AstKind::ExpressionStatement: return clone_node_reflected(static_cast<const ExpressionStatement*>(stmt));
            case AstKind::ReturnStatement: return clone_node_reflected(static_cast<const ReturnStatement*>(stmt));
            case AstKind::EnumDefinition: return clone_node_reflected(static_cast<const EnumDefinition*>(stmt));
            default: return nullptr;
            }
        }

        [[nodiscard]] inline TypeAnnPtr clone_type_annotation(const TypeAnnotation* type)
        {
            if (!type) return nullptr;
            switch (type->kind)
            {
            case AstKind::TypeAnnotation: return clone_node_reflected(static_cast<const TypeAnnotation*>(type));
            case AstKind::TupleTypeAnnotation: return clone_node_reflected(static_cast<const TupleTypeAnnotation*>(type));
            default: return nullptr;
            }
        }

        [[nodiscard]] inline std::unique_ptr<AstNode> clone_ast_node(const AstNode* node)
        {
            if (!node) return nullptr;
            switch (node->kind)
            {
            case AstKind::Directive: return clone_node_reflected(static_cast<const Directive*>(node));
            case AstKind::ImportStatement: return clone_node_reflected(static_cast<const ImportStatement*>(node));
            case AstKind::FunctionDefinition: return clone_node_reflected(static_cast<const FunctionDefinition*>(node));
            case AstKind::StructDefinition: return clone_node_reflected(static_cast<const StructDefinition*>(node));
            case AstKind::TypeAliasDefinition: return clone_node_reflected(static_cast<const TypeAliasDefinition*>(node));
            case AstKind::ExtensionDefinition: return clone_node_reflected(static_cast<const ExtensionDefinition*>(node));
            case AstKind::Program: return clone_node_reflected(static_cast<const Program*>(node));
            case AstKind::Comment: return std::make_unique<Comment>(clone_inner_node_reflected(*static_cast<const Comment*>(node)));
            case AstKind::CallArgument: return std::make_unique<CallArgument>(clone_inner_node_reflected(*static_cast<const CallArgument*>(node)));
            case AstKind::Modifier: return std::make_unique<Modifier>(clone_inner_node_reflected(*static_cast<const Modifier*>(node)));
            case AstKind::FunctionParameter: return std::make_unique<FunctionParameter>(clone_inner_node_reflected(*static_cast<const FunctionParameter*>(node)));
            case AstKind::StructField: return std::make_unique<StructField>(clone_inner_node_reflected(*static_cast<const StructField*>(node)));
            case AstKind::EnumCase: return std::make_unique<EnumCase>(clone_inner_node_reflected(*static_cast<const EnumCase*>(node)));
            case AstKind::DictItem: return std::make_unique<DictItem>(clone_inner_node_reflected(*static_cast<const DictItem*>(node)));
            case AstKind::SwitchCase: return std::make_unique<SwitchCase>(clone_inner_node_reflected(*static_cast<const SwitchCase*>(node)));
            case AstKind::AssignmentTarget: return std::make_unique<AssignmentTarget>(clone_inner_node_reflected(*static_cast<const AssignmentTarget*>(node)));
            default:
                if (auto expr = dynamic_cast<const Expression*>(node)) return clone_expression(expr);
                if (auto stmt = dynamic_cast<const Statement*>(node)) return clone_statement(stmt);
                if (auto type = dynamic_cast<const TypeAnnotation*>(node)) return clone_type_annotation(type);
                return nullptr;
            }
        }
    }

    inline Comment clone_node(const Comment& comment) { return detail::clone_inner_node_reflected(comment); }
    inline CallArgument clone_node(const CallArgument& arg) { return detail::clone_inner_node_reflected(arg); }
    inline Modifier clone_node(const Modifier& mod) { return detail::clone_inner_node_reflected(mod); }
    inline FunctionParameter clone_node(const FunctionParameter& param) { return detail::clone_inner_node_reflected(param); }
    inline StructField clone_node(const StructField& field) { return detail::clone_inner_node_reflected(field); }
    inline EnumCase clone_node(const EnumCase& item) { return detail::clone_inner_node_reflected(item); }
    inline DictItem clone_node(const DictItem& item) { return detail::clone_inner_node_reflected(item); }
    inline SwitchCase clone_node(const SwitchCase& item) { return detail::clone_inner_node_reflected(item); }
    inline AssignmentTarget clone_node(const AssignmentTarget& target) { return detail::clone_inner_node_reflected(target); }

    template <AstElement T>
    [[nodiscard]] std::unique_ptr<T> clone_node(const T* node)
    {
        if (!node) return nullptr;

        if constexpr (std::same_as<T, Expression>)
        {
            return detail::clone_expression(node);
        }
        else if constexpr (std::same_as<T, Statement>)
        {
            return detail::clone_statement(node);
        }
        else if constexpr (std::same_as<T, TypeAnnotation>)
        {
            return detail::clone_type_annotation(node);
        }
        else if constexpr (std::same_as<T, AstNode>)
        {
            return detail::clone_ast_node(node);
        }
        else if constexpr (ExpressionNode<T>)
        {
            auto cloned = detail::clone_expression(node);
            return ast_cast_unique<T>(std::move(cloned));
        }
        else if constexpr (StatementNode<T>)
        {
            auto cloned = detail::clone_statement(node);
            return ast_cast_unique<T>(std::move(cloned));
        }
        else if constexpr (TypeAnnNode<T>)
        {
            auto cloned = detail::clone_type_annotation(node);
            return ast_cast_unique<T>(std::move(cloned));
        }
        else
        {
            auto cloned = detail::clone_ast_node(node);
            return ast_cast_unique<T>(std::move(cloned));
        }
    }

    template <AstElement T>
    [[nodiscard]] std::unique_ptr<T> clone_node(const std::unique_ptr<T>& ptr)
    {
        return clone_node(ptr.get());
    }

    template <typename T>
    [[nodiscard]] std::vector<T> clone_nodes(const std::vector<T>& vec)
    {
        std::vector<T> result;
        result.reserve(vec.size());
        for (const auto& item : vec)
        {
            result.push_back(clone_node(item));
        }
        return result;
    }

    template <AstElement T>
    [[nodiscard]] std::vector<std::unique_ptr<T>> clone_nodes(const std::vector<std::unique_ptr<T>>& vec)
    {
        std::vector<std::unique_ptr<T>> result;
        result.reserve(vec.size());
        for (const auto& item : vec)
        {
            result.push_back(clone_node(item.get()));
        }
        return result;
    }

    template <typename T1, typename T2>
    [[nodiscard]] inline bool ast_is_clone_of(const T1& orig, const T2& clone) noexcept
    {
        return ast_equals(orig, clone) && ast_is_disjoint(orig, clone);
    }

    template <typename T>
    concept CloneableAstNode = ConcreteAstNode<T> && (
        requires(const T* node) { { clone_node(node) } -> std::same_as<std::unique_ptr<T>>; } ||
        requires(const T& node) { { clone_node(node) } -> std::same_as<T>; }
    );
}
