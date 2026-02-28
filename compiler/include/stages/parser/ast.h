#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "../lexer/token.h"

namespace valuascript::compiler {
    class AstNode {
    public:
        virtual ~AstNode() = default;
    };

    class Expression : public AstNode {
    };

    class Statement : public AstNode {
    };

    class NumberLiteral : public Expression {
    public:
        std::string value;

        explicit NumberLiteral(std::string val) : value(std::move(val)) {
        }
    };

    class StringLiteral : public Expression {
    public:
        std::string value;

        explicit StringLiteral(std::string val) : value(std::move(val)) {
        }
    };

    class BooleanLiteral : public Expression {
    public:
        bool value;

        explicit BooleanLiteral(bool val) : value(val) {
        }
    };

    class IdentifierAccess : public Expression {
    public:
        std::string name;

        explicit IdentifierAccess(std::string n) : name(std::move(n)) {
        }
    };

    class BinaryExpression : public Expression {
    public:
        std::unique_ptr<Expression> left;
        TokenType op;
        std::unique_ptr<Expression> right;

        BinaryExpression(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r)
            : left(std::move(l)), op(o), right(std::move(r)) {
        }
    };

    class UnaryExpression : public Expression {
    public:
        TokenType op;
        std::unique_ptr<Expression> right;

        UnaryExpression(TokenType o, std::unique_ptr<Expression> r)
            : op(o), right(std::move(r)) {
        }
    };

    class ConditionalExpression : public Expression {
    public:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Expression> then_branch;
        std::unique_ptr<Expression> else_branch;

        ConditionalExpression(std::unique_ptr<Expression> cond,
                              std::unique_ptr<Expression> thn,
                              std::unique_ptr<Expression> els)
            : condition(std::move(cond)), then_branch(std::move(thn)), else_branch(std::move(els)) {
        }
    };

    class FunctionCall : public Expression {
    public:
        std::unique_ptr<Expression> target;
        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

        FunctionCall(std::unique_ptr<Expression> tgt,
                     std::vector<std::pair<std::string, std::unique_ptr<Expression> > > args)
            : target(std::move(tgt)), arguments(std::move(args)) {
        }
    };

    class DictLiteral : public Expression {
    public:
        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > pairs;

        explicit DictLiteral(std::vector<std::pair<std::string, std::unique_ptr<Expression> > > pairs)
            : pairs(std::move(pairs)) {
        }
    };

    class TensorLiteral : public Expression {
    public:
        std::vector<std::unique_ptr<Expression> > elements;

        explicit TensorLiteral(std::vector<std::unique_ptr<Expression> > elems)
            : elements(std::move(elems)) {
        }
    };

    class TupleLiteral : public Expression {
    public:
        std::vector<std::unique_ptr<Expression> > elements;

        explicit TupleLiteral(std::vector<std::unique_ptr<Expression> > elements)
            : elements(std::move(elements)) {
        }
    };

    class TensorAccess : public Expression {
    public:
        std::unique_ptr<Expression> target;
        std::unique_ptr<Expression> index;

        TensorAccess(std::unique_ptr<Expression> tgt, std::unique_ptr<Expression> idx)
            : target(std::move(tgt)), index(std::move(idx)) {
        }
    };

    class Assignment : public Statement {
    public:
        std::vector<std::string> targets;
        std::unique_ptr<Expression> value;

        Assignment(std::vector<std::string> tgts, std::unique_ptr<Expression> val)
            : targets(std::move(tgts)), value(std::move(val)) {
        }
    };

    class ReturnStatement : public Statement {
    public:
        std::vector<std::unique_ptr<Expression> > values;

        explicit ReturnStatement(std::vector<std::unique_ptr<Expression> > return_values)
            : values(std::move(return_values)) {
        }
    };

    class Directive : public AstNode {
    public:
        std::string name;
        std::unique_ptr<Expression> value;

        Directive(std::string n, std::unique_ptr<Expression> val)
            : name(std::move(n)), value(std::move(val)) {
        }
    };

    class TypeAnnotation : public AstNode {
    public:
        std::string name;
        std::vector<std::unique_ptr<TypeAnnotation> > generic_args;

        explicit TypeAnnotation(std::string n, std::vector<std::unique_ptr<TypeAnnotation> > args = {})
            : name(std::move(n)), generic_args(std::move(args)) {
        }
    };

    struct FunctionParameter {
        std::string name;
        std::unique_ptr<TypeAnnotation> type;
    };

    class FunctionDefinition : public AstNode {
    public:
        std::string name;
        std::vector<FunctionParameter> parameters;
        std::vector<std::unique_ptr<TypeAnnotation> > return_types;
        std::vector<std::unique_ptr<Statement> > body;
        std::optional<std::string> docstring;

        FunctionDefinition(std::string n,
                           std::vector<FunctionParameter> params,
                           std::vector<std::unique_ptr<TypeAnnotation> > ret_types,
                           std::vector<std::unique_ptr<Statement> > b,
                           std::optional<std::string> docs = std::nullopt)
            : name(std::move(n)), parameters(std::move(params)),
              return_types(std::move(ret_types)), body(std::move(b)), docstring(std::move(docs)) {
        }
    };

    class Program : public AstNode {
    public:
        std::vector<std::unique_ptr<Directive> > directives;
        std::vector<std::unique_ptr<Assignment> > execution_steps;
        std::vector<std::unique_ptr<FunctionDefinition> > function_definitions;
    };
}
