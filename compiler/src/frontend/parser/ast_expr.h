#pragma once
#include "ast_core.h"
#include "ast_type.h"

namespace valuascript::compiler
{
    class NumberLiteral : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::NumberLiteral;
        std::string value;

        explicit NumberLiteral(std::string_view val) : Expression(KIND), value(val)
        {
        }
    };

    class PercentageLiteral : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::PercentageLiteral;
        std::string value;

        explicit PercentageLiteral(std::string_view val) : Expression(KIND), value(val)
        {
        }
    };

    class StringLiteral : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::StringLiteral;
        std::string value;

        explicit StringLiteral(std::string_view val) : Expression(KIND), value(val)
        {
        }
    };

    class BooleanLiteral : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::BooleanLiteral;
        bool value;

        explicit BooleanLiteral(bool val) : Expression(KIND), value(val)
        {
        }
    };

    class IdentifierAccess : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::IdentifierAccess;
        NodeName name;

        explicit IdentifierAccess(NodeName n) : Expression(KIND), name(std::move(n))
        {
        }
    };

    class SelfExpression : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::SelfExpression;
        SelfExpression() : Expression(KIND) {}
    };

    class BinaryExpression : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::BinaryExpression;
        ExprPtr left;
        TokenType op;
        ExprPtr right;

        explicit BinaryExpression(ExprPtr l, TokenType o, ExprPtr r)
            : Expression(KIND), left(std::move(l)), op(o), right(std::move(r))
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
        static constexpr AstKind KIND = AstKind::UnaryExpression;
        TokenType op;
        ExprPtr right;

        explicit UnaryExpression(TokenType o, ExprPtr r)
            : Expression(KIND), op(o), right(std::move(r))
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
        static constexpr AstKind KIND = AstKind::GroupingExpression;
        ExprPtr expression;

        explicit GroupingExpression(ExprPtr expr)
            : Expression(KIND), expression(std::move(expr))
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
        static constexpr AstKind KIND = AstKind::ConditionalExpression;
        ExprPtr condition;
        ExprPtr then_branch;
        ExprPtr else_branch;

        explicit ConditionalExpression(ExprPtr cond, ExprPtr thn, ExprPtr els)
            : Expression(KIND), condition(std::move(cond)), then_branch(std::move(thn)), else_branch(std::move(els))
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
        static constexpr AstKind KIND = AstKind::FunctionCall;
        ExprPtr target;
        std::vector<CallArgument> arguments;

        explicit FunctionCall(ExprPtr tgt, std::vector<CallArgument> args)
            : Expression(KIND), target(std::move(tgt)), arguments(std::move(args))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            if (!target || !target->is_complete()) return false;
            for (const auto& arg : arguments)
            {
                if (!arg.value || !arg.value->is_complete()) return false;
            }
            return true;
        }
    };

    struct DictItem
    {
        std::vector<Modifier> modifiers;
        NodeName key;
        ExprPtr value;
        SourceSpan span = {};
    };

    class DictLiteral : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::DictLiteral;
        std::vector<DictItem> elements;

        explicit DictLiteral(std::vector<DictItem> elems)
            : Expression(KIND), elements(std::move(elems))
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
        static constexpr AstKind KIND = AstKind::TensorLiteral;
        std::vector<ExprPtr> elements;

        explicit TensorLiteral(std::vector<ExprPtr> elems)
            : Expression(KIND), elements(std::move(elems))
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
        static constexpr AstKind KIND = AstKind::TupleLiteral;
        std::vector<ExprPtr> elements;

        explicit TupleLiteral(std::vector<ExprPtr> elems)
            : Expression(KIND), elements(std::move(elems))
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
        static constexpr AstKind KIND = AstKind::BracketAccess;
        ExprPtr target;
        ExprPtr index;

        explicit BracketAccess(ExprPtr tgt, ExprPtr idx)
            : Expression(KIND), target(std::move(tgt)), index(std::move(idx))
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
        static constexpr AstKind KIND = AstKind::DotAccess;
        ExprPtr target;
        NodeName property_name;

        explicit DotAccess(ExprPtr tgt, NodeName prop)
            : Expression(KIND), target(std::move(tgt)), property_name(std::move(prop))
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
        std::vector<NodeName> identifiers;
        ExprPtr result;
        SourceSpan span = {};
    };

    class SwitchExpression : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::SwitchExpression;
        ExprPtr target;
        std::vector<SwitchCase> cases;
        std::vector<Modifier> default_modifiers;
        ExprPtr default_case;

        SwitchExpression(ExprPtr t,
                         std::vector<SwitchCase> c,
                         std::vector<Modifier> def_mods,
                         ExprPtr def)
            : Expression(KIND), target(std::move(t)), cases(std::move(c)), default_modifiers(std::move(def_mods)),
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
        while (auto* grouping = ast_cast<GroupingExpression>(expr))
        {
            expr = grouping->expression.get();
        }
        return expr;
    }
}
