#pragma once

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <optional>
#include "token/token.h"

using namespace valuascript::shared;

namespace valuascript::compiler
{
    struct SourceSpan
    {
        size_t line_start = 0;
        size_t column_start = 0;
        size_t line_end = 0;
        size_t column_end = 0;
        std::string file_path;
    };

    class AstNode
    {
    public:
        SourceSpan span;

        virtual ~AstNode() = default;
    };

    class Expression : public AstNode
    {
    public:
        [[nodiscard]] virtual bool is_complete() const { return true; }
    };

    class Statement : public AstNode
    {
    };

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
        std::unique_ptr<Expression> left;
        TokenType op;
        std::unique_ptr<Expression> right;

        explicit BinaryExpression(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r)
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
        std::unique_ptr<Expression> right;

        explicit UnaryExpression(TokenType o, std::unique_ptr<Expression> r)
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
        std::unique_ptr<Expression> expression;

        explicit GroupingExpression(std::unique_ptr<Expression> expr)
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
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Expression> then_branch;
        std::unique_ptr<Expression> else_branch;

        explicit ConditionalExpression(std::unique_ptr<Expression> cond,
                                       std::unique_ptr<Expression> thn,
                                       std::unique_ptr<Expression> els)
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
        std::unique_ptr<Expression> target;
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> arguments;

        explicit FunctionCall(std::unique_ptr<Expression> tgt,
                              std::vector<std::pair<std::string, std::unique_ptr<Expression>>> args)
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

    struct Modifier
    {
        std::string name;
        std::vector<std::pair<std::string, std::unique_ptr<Expression>>> arguments;
        SourceSpan span;
    };

    struct DictItem
    {
        std::vector<Modifier> modifiers;
        std::string key;
        std::unique_ptr<Expression> value;
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
        std::vector<std::unique_ptr<Expression>> elements;

        explicit TensorLiteral(std::vector<std::unique_ptr<Expression>> elems)
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
        std::vector<std::unique_ptr<Expression>> elements;

        explicit TupleLiteral(std::vector<std::unique_ptr<Expression>> elems)
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
        std::unique_ptr<Expression> target;
        std::unique_ptr<Expression> index;

        explicit BracketAccess(std::unique_ptr<Expression> tgt, std::unique_ptr<Expression> idx)
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
        std::unique_ptr<Expression> target;
        std::string property_name;

        DotAccess(std::unique_ptr<Expression> t, std::string prop)
            : target(std::move(t)), property_name(std::move(prop))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            return target && target->is_complete();
        }
    };

    class SwitchExpression : public Expression
    {
    public:
        std::unique_ptr<Expression> target;
        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression>>> cases;
        std::unique_ptr<Expression> default_case;

        SwitchExpression(std::unique_ptr<Expression> t,
                         std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression>>> c,
                         std::unique_ptr<Expression> def)
            : target(std::move(t)),
              cases(std::move(c)),
              default_case(std::move(def))
        {
        }

        [[nodiscard]] bool is_complete() const override
        {
            if (!target || !target->is_complete()) return false;
            for (const auto& [ids, result] : cases)
            {
                if (!result || !result->is_complete()) return false;
            }
            if (default_case && !default_case->is_complete()) return false;
            return true;
        }
    };

    class TypeAnnotation : public AstNode
    {
    public:
        std::string name;
        std::vector<std::unique_ptr<TypeAnnotation>> generic_args;

        explicit TypeAnnotation(std::string n, std::vector<std::unique_ptr<TypeAnnotation>> args = {})
            : name(std::move(n)), generic_args(std::move(args))
        {
        }
    };

    class TupleTypeAnnotation : public TypeAnnotation
    {
    public:
        std::vector<std::unique_ptr<TypeAnnotation>> element_types;

        explicit TupleTypeAnnotation(std::vector<std::unique_ptr<TypeAnnotation>> elements)
            : TypeAnnotation("tuple"), element_types(std::move(elements))
        {
        }
    };

    class Assignment : public Statement
    {
    public:
        std::vector<Modifier> modifiers;
        std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation>>> targets;
        std::unique_ptr<Expression> value;

        Assignment(std::vector<Modifier> mods,
                   std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation>>> tar,
                   std::unique_ptr<Expression> val)
            : modifiers(std::move(mods)), targets(std::move(tar)), value(std::move(val))
        {
        }
    };

    class Reassignment : public Statement
    {
    public:
        std::unique_ptr<Expression> target;
        std::unique_ptr<Expression> value;

        Reassignment(std::unique_ptr<Expression> tar, std::unique_ptr<Expression> val)
            : target(std::move(tar)), value(std::move(val))
        {
        }
    };

    class ExpressionStatement : public Statement
    {
    public:
        std::unique_ptr<Expression> expr;

        explicit ExpressionStatement(std::unique_ptr<Expression> e)
            : expr(std::move(e))
        {
        }
    };

    class ReturnStatement : public Statement
    {
    public:
        std::vector<std::unique_ptr<Expression>> values;

        explicit ReturnStatement(std::vector<std::unique_ptr<Expression>> return_values)
            : values(std::move(return_values))
        {
        }
    };

    class Directive : public AstNode
    {
    public:
        std::string name;
        std::unique_ptr<Expression> value;

        explicit Directive(std::string n, std::unique_ptr<Expression> val)
            : name(std::move(n)), value(std::move(val))
        {
        }
    };

    class ImportStatement : public AstNode
    {
    public:
        std::string path;

        explicit ImportStatement(std::string p)
            : path(std::move(p))
        {
        }
    };

    struct FunctionParameter
    {
        std::vector<Modifier> modifiers;
        std::string name;
        std::unique_ptr<TypeAnnotation> type;
        std::unique_ptr<Expression> default_value = nullptr;
    };

    class FunctionDefinition : public AstNode
    {
    public:
        std::vector<Modifier> modifiers;
        std::string name;
        std::vector<FunctionParameter> parameters;
        std::vector<std::unique_ptr<TypeAnnotation>> return_types;
        std::vector<std::unique_ptr<Statement>> body;
        std::optional<std::string> docstring;

        explicit FunctionDefinition(std::vector<Modifier> mods,
                                    std::string n,
                                    std::vector<FunctionParameter> params,
                                    std::vector<std::unique_ptr<TypeAnnotation>> ret_types,
                                    std::vector<std::unique_ptr<Statement>> b,
                                    std::optional<std::string> docs = std::nullopt)
            : modifiers(std::move(mods)), name(std::move(n)), parameters(std::move(params)),
              return_types(std::move(ret_types)), body(std::move(b)), docstring(std::move(docs))
        {
        }
    };

    struct StructField
    {
        std::vector<Modifier> modifiers;
        std::string name;
        std::unique_ptr<TypeAnnotation> type;
        SourceSpan span;
    };

    class StructDefinition : public AstNode
    {
    public:
        std::vector<Modifier> modifiers;
        std::string name;
        std::vector<StructField> fields;

        explicit StructDefinition(std::vector<Modifier> mods,
                                  std::string n,
                                  std::vector<StructField> f)
            : modifiers(std::move(mods)), name(std::move(n)), fields(std::move(f))
        {
        }
    };

    struct EnumCase
    {
        std::vector<Modifier> modifiers;
        std::string name;
        std::unique_ptr<Expression> value;
    };

    class EnumDefinition : public Statement
    {
    public:
        std::vector<Modifier> modifiers;
        std::string name;
        std::unique_ptr<TypeAnnotation> underlying_type;
        std::vector<EnumCase> cases;

        EnumDefinition(std::vector<Modifier> mods,
                       std::string n,
                       std::unique_ptr<TypeAnnotation> type,
                       std::vector<EnumCase> c)
            : modifiers(std::move(mods)),
              name(std::move(n)),
              underlying_type(std::move(type)),
              cases(std::move(c))
        {
        }
    };

    class TypeAliasDefinition : public AstNode
    {
    public:
        std::vector<Modifier> modifiers;
        std::string name;
        std::unique_ptr<TypeAnnotation> target_type;

        explicit TypeAliasDefinition(std::vector<Modifier> mods,
                                     std::string n,
                                     std::unique_ptr<TypeAnnotation> t_type)
            : modifiers(std::move(mods)), name(std::move(n)), target_type(std::move(t_type))
        {
        }
    };

    class Program : public AstNode
    {
    public:
        std::vector<std::unique_ptr<ImportStatement>> import_statements;
        std::vector<std::unique_ptr<Directive>> directives;
        std::vector<std::unique_ptr<Statement>> execution_steps;
        std::vector<std::unique_ptr<FunctionDefinition>> function_definitions;
        std::vector<std::unique_ptr<StructDefinition>> struct_definitions;
        std::vector<std::unique_ptr<EnumDefinition>> enum_definitions;
        std::vector<std::unique_ptr<TypeAliasDefinition>> type_aliases;
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
