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
    };
}
