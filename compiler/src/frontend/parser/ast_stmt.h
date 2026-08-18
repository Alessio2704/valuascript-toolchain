#pragma once
#include "ast_core.h"
#include "ast_expr.h"
#include "ast_type.h"

namespace valuascript::compiler
{
    struct AssignmentTarget
    {
        std::vector<Modifier> modifiers;
        std::string name;
        TypeAnnPtr type;
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

    struct EnumCase
    {
        std::vector<Modifier> modifiers;
        std::string name;
        ExprPtr value;
    };

    class EnumDefinition : public Statement
    {
    public:
        static constexpr AstKind KIND = AstKind::EnumDefinition;
        std::vector<Modifier> modifiers;
        std::string name;
        TypeAnnPtr underlying_type;
        std::vector<EnumCase> cases;

        EnumDefinition(std::vector<Modifier> mods,
                       std::string_view n,
                       TypeAnnPtr type,
                       std::vector<EnumCase> c)
            : Statement(KIND),
              modifiers(std::move(mods)),
              name(n),
              underlying_type(std::move(type)),
              cases(std::move(c))
        {
        }
    };
}
