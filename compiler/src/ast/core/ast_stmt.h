#pragma once
#include "ast_core.h"
#include "ast_expr.h"
#include "ast_type.h"

namespace valuascript::compiler
{
    class AssignmentTarget : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::AssignmentTarget;
        std::vector<Modifier> modifiers;
        NodeName name;
        TypeAnnPtr type;

        AssignmentTarget() : AstNode(KIND) {}
        AssignmentTarget(std::vector<Modifier> mods,
                         NodeName n,
                         TypeAnnPtr t,
                         SourceSpan sp = {})
            : AstNode(KIND), modifiers(std::move(mods)), name(std::move(n)), type(std::move(t))
        {
            span = sp;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(modifiers) && (!type || type->is_valid());
        }
    };

    class Assignment : public Statement
    {
    public:
        static constexpr AstKind KIND = AstKind::Assignment;
        std::vector<AssignmentTarget> targets;
        ExprPtr value;

        Assignment(std::vector<AssignmentTarget> tar,
                   ExprPtr val)
            : Statement(KIND), targets(std::move(tar)), value(std::move(val))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return value && value->is_valid() && are_all_valid(targets) && span.is_valid();
        }
    };

    class Reassignment : public Statement
    {
    public:
        static constexpr AstKind KIND = AstKind::Reassignment;
        ExprPtr target;
        ExprPtr value;

        Reassignment(ExprPtr tar, ExprPtr val)
            : Statement(KIND), target(std::move(tar)), value(std::move(val))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return target && target->is_valid() && value && value->is_valid() && span.is_valid();
        }
    };

    class ExpressionStatement : public Statement
    {
    public:
        static constexpr AstKind KIND = AstKind::ExpressionStatement;
        ExprPtr expr;

        explicit ExpressionStatement(ExprPtr e)
            : Statement(KIND), expr(std::move(e))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return expr && expr->is_valid() && span.is_valid();
        }
    };

    class ReturnStatement : public Statement
    {
    public:
        static constexpr AstKind KIND = AstKind::ReturnStatement;
        std::vector<Modifier> modifiers;
        std::vector<ExprPtr> values;

        explicit ReturnStatement(std::vector<Modifier> mods, std::vector<ExprPtr> return_values)
            : Statement(KIND), modifiers(std::move(mods)), values(std::move(return_values))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return are_all_valid(modifiers) && are_all_valid(values) && span.is_valid();
        }
    };

    class EnumCase : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::EnumCase;
        std::vector<Modifier> modifiers;
        NodeName name;
        ExprPtr value = nullptr;

        EnumCase() : AstNode(KIND) {}
        EnumCase(std::vector<Modifier> mods,
                 NodeName n,
                 ExprPtr val = nullptr,
                 SourceSpan sp = {})
            : AstNode(KIND), modifiers(std::move(mods)), name(std::move(n)), value(std::move(val))
        {
            span = sp;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(modifiers) && (!value || value->is_valid());
        }
    };

    class EnumDefinition : public Statement
    {
    public:
        static constexpr AstKind KIND = AstKind::EnumDefinition;
        std::vector<Modifier> modifiers;
        NodeName name;
        TypeAnnPtr underlying_type;
        std::vector<EnumCase> cases;

        EnumDefinition(std::vector<Modifier> mods,
                       NodeName n,
                       TypeAnnPtr type,
                       std::vector<EnumCase> c)
            : Statement(KIND),
              modifiers(std::move(mods)),
              name(std::move(n)),
              underlying_type(std::move(type)),
              cases(std::move(c))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid() && are_all_valid(modifiers) && are_all_valid(cases) && (!underlying_type || underlying_type->is_valid());
        }
    };
}
