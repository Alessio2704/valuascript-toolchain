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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return !value.empty() && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return !value.empty() && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return name.is_valid() && span.is_valid();
        }
    };

    class SelfExpression : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::SelfExpression;
        SelfExpression() : Expression(KIND) {}

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return span.is_valid();
        }
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return left && left->is_valid() && right && right->is_valid() && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return right && right->is_valid() && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return expression && expression->is_valid() && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return condition && condition->is_valid() &&
                   then_branch && then_branch->is_valid() &&
                   else_branch && else_branch->is_valid() &&
                   span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return target && target->is_valid() && are_all_valid(arguments) && span.is_valid();
        }
    };

    class DictItem : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::DictItem;
        std::vector<Modifier> modifiers;
        NodeName key;
        ExprPtr value = nullptr;

        DictItem() : AstNode(KIND) {}
        DictItem(std::vector<Modifier> mods,
                 NodeName k,
                 ExprPtr v = nullptr,
                 SourceSpan sp = {})
            : AstNode(KIND), modifiers(std::move(mods)), key(std::move(k)), value(std::move(v))
        {
            span = sp;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return key.is_valid() && span.is_valid() && are_all_valid(modifiers) && (!value || value->is_valid());
        }
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return are_all_valid(elements) && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return are_all_valid(elements) && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return are_all_valid(elements) && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return target && target->is_valid() && index && index->is_valid() && span.is_valid();
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

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return target && target->is_valid() && property_name.is_valid() && span.is_valid();
        }
    };

    class SwitchCase : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::SwitchCase;
        std::vector<Modifier> modifiers;
        std::vector<NodeName> identifiers;
        ExprPtr result = nullptr;

        SwitchCase() : AstNode(KIND) {}
        SwitchCase(std::vector<Modifier> mods,
                   std::vector<NodeName> ids,
                   ExprPtr res = nullptr,
                   SourceSpan sp = {})
            : AstNode(KIND), modifiers(std::move(mods)), identifiers(std::move(ids)), result(std::move(res))
        {
            span = sp;
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return span.is_valid() && are_all_valid(modifiers) && are_all_valid(identifiers) && (!result || result->is_valid());
        }
    };

    class SwitchExpression : public Expression
    {
    public:
        static constexpr AstKind KIND = AstKind::SwitchExpression;
        ExprPtr target;
        std::vector<SwitchCase> cases;
        std::vector<Modifier> default_modifiers;
        OptionalAstField<ExprPtr> default_case = std::nullopt;

        SwitchExpression(ExprPtr t,
                         std::vector<SwitchCase> c,
                         std::vector<Modifier> def_mods,
                         OptionalAstField<ExprPtr> def = std::nullopt)
            : Expression(KIND), target(std::move(t)), cases(std::move(c)), default_modifiers(std::move(def_mods)),
              default_case(std::move(def))
        {
        }

        [[nodiscard]] bool is_valid() const noexcept override
        {
            return target && target->is_valid() &&
                   are_all_valid(cases) &&
                   are_all_valid(default_modifiers) &&
                   (!default_case || default_case->is_valid()) &&
                   span.is_valid();
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
