#pragma once
#include "ast_core.h"
#include "ast_type.h"

namespace valuascript::compiler
{
    class NumberLiteral : public Expression
    {
    public:
        std::string value;

        explicit NumberLiteral(std::string val) : value(std::move(val))
        {
        }
    };

    class PercentageLiteral : public Expression
    {
    public:
        std::string value;

        explicit PercentageLiteral(std::string val) : value(std::move(val))
        {
        }
    };

    class StringLiteral : public Expression
    {
    public:
        std::string value;

        explicit StringLiteral(std::string val) : value(std::move(val))
        {
        }
    };

    class BooleanLiteral : public Expression
    {
    public:
        bool value;

        explicit BooleanLiteral(bool val) : value(val)
        {
        }
    };

    class IdentifierAccess : public Expression
    {
    public:
        std::string name;

        explicit IdentifierAccess(std::string n) : name(std::move(n))
        {
        }
    };

    class SelfExpression : public Expression
    {
    public:
        SelfExpression() = default;
    };

    class BinaryExpression : public Expression
    {
    public:
        ExprPtr left;
        TokenType op;
        ExprPtr right;

        explicit BinaryExpression(ExprPtr l, TokenType o, ExprPtr r)
            : left(std::move(l)), op(o), right(std::move(r))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            return left && left->is_complete() && right && right->is_complete();
        }
    };

    class UnaryExpression : public Expression
    {
    public:
        TokenType op;
        ExprPtr right;

        explicit UnaryExpression(TokenType o, ExprPtr r)
            : op(o), right(std::move(r))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            return right && right->is_complete();
        }
    };

    class GroupingExpression : public Expression
    {
    public:
        ExprPtr expression;

        explicit GroupingExpression(ExprPtr expr)
            : expression(std::move(expr))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            return expression && expression->is_complete();
        }
    };

    class ConditionalExpression : public Expression
    {
    public:
        ExprPtr condition;
        ExprPtr then_branch;
        ExprPtr else_branch;

        explicit ConditionalExpression(ExprPtr cond, ExprPtr thn, ExprPtr els)
            : condition(std::move(cond)), then_branch(std::move(thn)), else_branch(std::move(els))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            return condition && condition->is_complete() &&
                then_branch && then_branch->is_complete() &&
                else_branch && else_branch->is_complete();
        }
    };

    class FunctionCall : public Expression
    {
    public:
        ExprPtr target;
        std::vector<std::pair<std::string, ExprPtr>> arguments;

        explicit FunctionCall(ExprPtr tgt, std::vector<std::pair<std::string, ExprPtr>> args)
            : target(std::move(tgt)), arguments(std::move(args))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            if (!target || !target->is_complete()) return false;
            for (const auto& [name, val] : arguments)
            {
                if (!val || !val->is_complete()) return false;
            }
            return true;
        }
    };

    struct DictItem
    {
        std::vector<Modifier> modifiers;
        std::string key;
        ExprPtr value;
    };

    class DictLiteral : public Expression
    {
    public:
        std::vector<DictItem> elements;

        explicit DictLiteral(std::vector<DictItem> elems)
            : elements(std::move(elems))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            for (const auto& item : elements)
            {
                if (!item.value || !item.value->is_complete()) return false;
            }
            return true;
        }
    };

    class TensorLiteral : public Expression
    {
    public:
        std::vector<ExprPtr> elements;

        explicit TensorLiteral(std::vector<ExprPtr> elems)
            : elements(std::move(elems))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            for (const auto& elem : elements)
            {
                if (!elem || !elem->is_complete()) return false;
            }
            return true;
        }
    };

    class TupleLiteral : public Expression
    {
    public:
        std::vector<ExprPtr> elements;

        explicit TupleLiteral(std::vector<ExprPtr> elems)
            : elements(std::move(elems))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            for (const auto& elem : elements)
            {
                if (!elem || !elem->is_complete()) return false;
            }
            return true;
        }
    };

    class BracketAccess : public Expression
    {
    public:
        ExprPtr target;
        ExprPtr index;

        explicit BracketAccess(ExprPtr tgt, ExprPtr idx)
            : target(std::move(tgt)), index(std::move(idx))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            return target && target->is_complete() && index && index->is_complete();
        }
    };

    class DotAccess : public Expression
    {
    public:
        ExprPtr target;
        std::string property_name;

        DotAccess(ExprPtr t, std::string prop)
            : target(std::move(t)), property_name(std::move(prop))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            return target && target->is_complete();
        }
    };

    struct SwitchCase
    {
        std::vector<Modifier> modifiers;
        std::vector<std::string> identifiers;
        ExprPtr result;
    };

    class SwitchExpression : public Expression
    {
    public:
        ExprPtr target;
        std::vector<SwitchCase> cases;
        std::vector<Modifier> default_modifiers;
        ExprPtr default_case;

        SwitchExpression(ExprPtr t,
                         std::vector<SwitchCase> c,
                         std::vector<Modifier> def_mods,
                         ExprPtr def)
            : target(std::move(t)), cases(std::move(c)), default_modifiers(std::move(def_mods)),
              default_case(std::move(def))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            if (!target || !target->is_complete()) return false;
            for (const auto& c : cases)
            {
                if (!c.result || !c.result->is_complete()) return false;
            }
            if (default_case && !default_case->is_complete()) return false;
            return true;
        }
    };

    inline Expression* unwrap_grouping(Expression* expr)
    {
        while (auto* grouping = dynamic_cast<GroupingExpression*>(expr))
        {
            expr = grouping->expression.get();
        }
        return expr;
    }
}
