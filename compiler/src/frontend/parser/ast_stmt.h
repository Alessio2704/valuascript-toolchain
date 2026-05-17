#pragma once
#include "ast_core.h"
#include "ast_expr.h"
#include "ast_type.h"

namespace valuascript::compiler
{
    class Assignment : public Statement
    {
    public:
        std::vector<Modifier> modifiers;
        std::vector<std::pair<std::string, TypeAnnPtr>> targets;
        ExprPtr value;

        Assignment(std::vector<Modifier> mods,
                   std::vector<std::pair<std::string, TypeAnnPtr>> tar,
                   ExprPtr val)
            : modifiers(std::move(mods)), targets(std::move(tar)), value(std::move(val))
        {
        }
    };

    class Reassignment : public Statement
    {
    public:
        ExprPtr target;
        ExprPtr value;

        Reassignment(ExprPtr tar, ExprPtr val)
            : target(std::move(tar)), value(std::move(val))
        {
        }
    };

    class ExpressionStatement : public Statement
    {
    public:
        ExprPtr expr;

        explicit ExpressionStatement(ExprPtr e)
            : expr(std::move(e))
        {
        }
    };

    class ReturnStatement : public Statement
    {
    public:
        std::vector<ExprPtr> values;

        explicit ReturnStatement(std::vector<ExprPtr> return_values)
            : values(std::move(return_values))
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
        std::vector<Modifier> modifiers;
        std::string name;
        TypeAnnPtr underlying_type;
        std::vector<EnumCase> cases;

        EnumDefinition(std::vector<Modifier> mods,
                       std::string n,
                       TypeAnnPtr type,
                       std::vector<EnumCase> c)
            : modifiers(std::move(mods)),
              name(std::move(n)),
              underlying_type(std::move(type)),
              cases(std::move(c))
        {
        }
    };
}
