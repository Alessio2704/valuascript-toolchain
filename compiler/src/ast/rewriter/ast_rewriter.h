#pragma once

#include <memory>
#include <vector>
#include <concepts>
#include <type_traits>
#include <utility>

#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/metadata/ast_node_schema.h"
#include "ast/categories/ast_category_types.h"
#include "ast/walker/ast_walker_concepts.h"

namespace valuascript::compiler
{
    class AstRewriter
    {
    public:
        virtual ~AstRewriter() = default;

        template <typename T>
        void auto_rewrite_children(T& node)
        {
            for_each_ast_member(node, [&](auto& member) {
                using MemberT = std::remove_cvref_t<decltype(member)>;

                if constexpr (IsUniquePtrOfAstNode<MemberT>)
                {
                    if (member)
                    {
                        using ConcreteT = typename MemberT::element_type;
                        if constexpr (std::derived_from<ConcreteT, Expression>)
                        {
                            if constexpr (std::same_as<ConcreteT, Expression>)
                            {
                                member = rewrite(std::move(member));
                            }
                            else
                            {
                                member = ast_cast_unique<ConcreteT>(rewrite(std::move(member)));
                            }
                        }
                        else if constexpr (std::derived_from<ConcreteT, Statement>)
                        {
                            if constexpr (std::same_as<ConcreteT, Statement>)
                            {
                                member = rewrite(std::move(member));
                            }
                            else
                            {
                                member = ast_cast_unique<ConcreteT>(rewrite(std::move(member)));
                            }
                        }
                        else if constexpr (std::derived_from<ConcreteT, TypeAnnotation>)
                        {
                            if constexpr (std::same_as<ConcreteT, TypeAnnotation>)
                            {
                                member = rewrite(std::move(member));
                            }
                            else
                            {
                                member = ast_cast_unique<ConcreteT>(rewrite(std::move(member)));
                            }
                        }
                        else
                        {
                            member = rewrite(std::move(member));
                        }
                    }
                }
                else if constexpr (IsVectorOfUniquePtrOfAstNode<MemberT>)
                {
                    for (auto& item : member)
                    {
                        if (item)
                        {
                            using ConcreteT = typename MemberT::value_type::element_type;
                            if constexpr (std::derived_from<ConcreteT, Expression>)
                            {
                                if constexpr (std::same_as<ConcreteT, Expression>)
                                {
                                    item = rewrite(std::move(item));
                                }
                                else
                                {
                                    item = ast_cast_unique<ConcreteT>(rewrite(std::move(item)));
                                }
                            }
                            else if constexpr (std::derived_from<ConcreteT, Statement>)
                            {
                                if constexpr (std::same_as<ConcreteT, Statement>)
                                {
                                    item = rewrite(std::move(item));
                                }
                                else
                                {
                                    item = ast_cast_unique<ConcreteT>(rewrite(std::move(item)));
                                }
                            }
                            else if constexpr (std::derived_from<ConcreteT, TypeAnnotation>)
                            {
                                if constexpr (std::same_as<ConcreteT, TypeAnnotation>)
                                {
                                    item = rewrite(std::move(item));
                                }
                                else
                                {
                                    item = ast_cast_unique<ConcreteT>(rewrite(std::move(item)));
                                }
                            }
                            else
                            {
                                item = rewrite(std::move(item));
                            }
                        }
                    }
                }
                else if constexpr (IsInnerAstNode<MemberT>)
                {
                    member = rewrite(std::move(member));
                }
                else if constexpr (IsVectorOfInnerAstNode<MemberT>)
                {
                    for (auto& item : member)
                    {
                        item = rewrite(std::move(item));
                    }
                }
                else if constexpr (IsOptionalAstFieldOfAstNode<MemberT> || IsStdOptionalOfAstNode<MemberT>)
                {
                    if (member.has_value())
                    {
                        if constexpr (IsOptionalAstFieldOfAstNode<MemberT>)
                        {
                            using PtrT = std::remove_cvref_t<decltype(member.value())>;
                            using ConcreteT = typename PtrT::element_type;
                            if constexpr (std::derived_from<ConcreteT, Expression>)
                            {
                                if constexpr (std::same_as<ConcreteT, Expression>)
                                {
                                    member.value() = rewrite(std::move(member.value()));
                                }
                                else
                                {
                                    member.value() = ast_cast_unique<ConcreteT>(rewrite(std::move(member.value())));
                                }
                            }
                            else if constexpr (std::derived_from<ConcreteT, Statement>)
                            {
                                if constexpr (std::same_as<ConcreteT, Statement>)
                                {
                                    member.value() = rewrite(std::move(member.value()));
                                }
                                else
                                {
                                    member.value() = ast_cast_unique<ConcreteT>(rewrite(std::move(member.value())));
                                }
                            }
                            else if constexpr (std::derived_from<ConcreteT, TypeAnnotation>)
                            {
                                if constexpr (std::same_as<ConcreteT, TypeAnnotation>)
                                {
                                    member.value() = rewrite(std::move(member.value()));
                                }
                                else
                                {
                                    member.value() = ast_cast_unique<ConcreteT>(rewrite(std::move(member.value())));
                                }
                            }
                            else
                            {
                                member.value() = rewrite(std::move(member.value()));
                            }
                        }
                        else
                        {
                            using ValT = typename MemberT::value_type;
                            if constexpr (is_unique_ptr_v<ValT>)
                            {
                                using ConcreteT = typename ValT::element_type;
                                if constexpr (std::derived_from<ConcreteT, Expression>)
                                {
                                    if constexpr (std::same_as<ConcreteT, Expression>)
                                    {
                                        *member = rewrite(std::move(*member));
                                    }
                                    else
                                    {
                                        *member = ast_cast_unique<ConcreteT>(rewrite(std::move(*member)));
                                    }
                                }
                                else if constexpr (std::derived_from<ConcreteT, Statement>)
                                {
                                    if constexpr (std::same_as<ConcreteT, Statement>)
                                    {
                                        *member = rewrite(std::move(*member));
                                    }
                                    else
                                    {
                                        *member = ast_cast_unique<ConcreteT>(rewrite(std::move(*member)));
                                    }
                                }
                                else if constexpr (std::derived_from<ConcreteT, TypeAnnotation>)
                                {
                                    if constexpr (std::same_as<ConcreteT, TypeAnnotation>)
                                    {
                                        *member = rewrite(std::move(*member));
                                    }
                                    else
                                    {
                                        *member = ast_cast_unique<ConcreteT>(rewrite(std::move(*member)));
                                    }
                                }
                                else
                                {
                                    *member = rewrite(std::move(*member));
                                }
                            }
                            else
                            {
                                *member = rewrite(std::move(*member));
                            }
                        }
                    }
                }
            });
        }

        virtual std::unique_ptr<Program> rewrite(std::unique_ptr<Program> program)
        {
            if (!program) return nullptr;
            auto_rewrite_children(*program);
            return program;
        }

        virtual StmtPtr rewrite(StmtPtr stmt)
        {
            if (!stmt) return nullptr;

            StmtPtr result = nullptr;
            NodeDispatcher<AllStatementNodeTypes>::dispatch(stmt->kind, [&]<typename T>() {
                result = rewrite(ast_cast_unique<T>(std::move(stmt)));
            });

            return result ? std::move(result) : std::move(stmt);
        }

        virtual ExprPtr rewrite(ExprPtr expr)
        {
            if (!expr) return nullptr;

            ExprPtr result = nullptr;
            NodeDispatcher<AllExpressionNodeTypes>::dispatch(expr->kind, [&]<typename T>() {
                result = rewrite(ast_cast_unique<T>(std::move(expr)));
            });

            return result ? std::move(result) : std::move(expr);
        }

        virtual TypeAnnPtr rewrite(TypeAnnPtr type)
        {
            if (!type) return nullptr;

            if (type->kind == AstKind::TupleTypeAnnotation)
            {
                return rewrite(ast_cast_unique<TupleTypeAnnotation>(std::move(type)));
            }

            auto_rewrite_children(*type);
            return type;
        }

        virtual ImportPtr rewrite(ImportPtr node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual DirectivePtr rewrite(DirectivePtr node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual FuncDefPtr rewrite(FuncDefPtr node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual StructDefPtr rewrite(StructDefPtr node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual EnumDefPtr rewrite(EnumDefPtr node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual TypeAliasPtr rewrite(TypeAliasPtr node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExtensionDefPtr rewrite(ExtensionDefPtr node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual StmtPtr rewrite(std::unique_ptr<Assignment> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual StmtPtr rewrite(std::unique_ptr<Reassignment> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual StmtPtr rewrite(std::unique_ptr<ExpressionStatement> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual StmtPtr rewrite(std::unique_ptr<ReturnStatement> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<NumberLiteral> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<PercentageLiteral> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<StringLiteral> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<BooleanLiteral> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<IdentifierAccess> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<SelfExpression> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<BinaryExpression> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<UnaryExpression> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<GroupingExpression> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<ConditionalExpression> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<FunctionCall> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<DictLiteral> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<TensorLiteral> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<TupleLiteral> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<BracketAccess> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<DotAccess> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual ExprPtr rewrite(std::unique_ptr<SwitchExpression> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual TypeAnnPtr rewrite(std::unique_ptr<TupleTypeAnnotation> node)
        {
            if (!node) return nullptr;
            auto_rewrite_children(*node);
            return node;
        }

        virtual FunctionParameter rewrite(FunctionParameter node)
        {
            auto_rewrite_children(node);
            return node;
        }

        virtual StructField rewrite(StructField node)
        {
            auto_rewrite_children(node);
            return node;
        }

        virtual EnumCase rewrite(EnumCase node)
        {
            auto_rewrite_children(node);
            return node;
        }

        virtual SwitchCase rewrite(SwitchCase node)
        {
            auto_rewrite_children(node);
            return node;
        }

        virtual AssignmentTarget rewrite(AssignmentTarget node)
        {
            auto_rewrite_children(node);
            return node;
        }

        virtual Modifier rewrite(Modifier node)
        {
            auto_rewrite_children(node);
            return node;
        }

        virtual CallArgument rewrite(CallArgument node)
        {
            auto_rewrite_children(node);
            return node;
        }

        virtual DictItem rewrite(DictItem node)
        {
            auto_rewrite_children(node);
            return node;
        }

        virtual Comment rewrite(Comment node)
        {
            auto_rewrite_children(node);
            return node;
        }
    };
}
