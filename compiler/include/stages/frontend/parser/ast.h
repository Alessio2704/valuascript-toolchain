#pragma once

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include "../lexer/token.h"

namespace valuascript::compiler {

    struct SourceSpan {
        size_t line_start = 0;
        size_t column_start = 0;
        size_t line_end = 0;
        size_t column_end = 0;
        std::string file_path;
    };

    class AstNode {
    public:
        SourceSpan span;
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

    class PercentageLiteral : public Expression {
    public:
        std::string value;

        explicit PercentageLiteral(std::string value) : value(std::move(value)) {
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

        explicit BinaryExpression(std::unique_ptr<Expression> l, TokenType o, std::unique_ptr<Expression> r)
            : left(std::move(l)), op(o), right(std::move(r)) {
        }
    };

    class UnaryExpression : public Expression {
    public:
        TokenType op;
        std::unique_ptr<Expression> right;

        explicit UnaryExpression(TokenType o, std::unique_ptr<Expression> r)
            : op(o), right(std::move(r)) {
        }
    };

    class ConditionalExpression : public Expression {
    public:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<Expression> then_branch;
        std::unique_ptr<Expression> else_branch;

        explicit ConditionalExpression(std::unique_ptr<Expression> cond,
                                       std::unique_ptr<Expression> thn,
                                       std::unique_ptr<Expression> els)
            : condition(std::move(cond)), then_branch(std::move(thn)), else_branch(std::move(els)) {
        }
    };

    class FunctionCall : public Expression {
    public:
        std::unique_ptr<Expression> target;
        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > arguments;

        explicit FunctionCall(std::unique_ptr<Expression> tgt,
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

    class BracketAccess : public Expression {
    public:
        std::unique_ptr<Expression> target;
        std::unique_ptr<Expression> index;

        explicit BracketAccess(std::unique_ptr<Expression> tgt, std::unique_ptr<Expression> idx)
            : target(std::move(tgt)), index(std::move(idx)) {
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

    class TupleTypeAnnotation : public TypeAnnotation {
    public:
        std::vector<std::unique_ptr<TypeAnnotation> > element_types;

        explicit TupleTypeAnnotation(std::vector<std::unique_ptr<TypeAnnotation> > elements)
            : TypeAnnotation("tuple"), element_types(std::move(elements)) {
        }
    };

    class Assignment : public Statement {
    public:
        std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > targets;
        std::unique_ptr<Expression> value;
        bool is_mutable;

        Assignment(std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > targets,
                   std::unique_ptr<Expression> value, bool is_mutable)
            : targets(std::move(targets)), value(std::move(value)), is_mutable(is_mutable) {
        }
    };

    class Reassignment : public Statement {
    public:
        std::unique_ptr<Expression> target;
        std::unique_ptr<Expression> value;

        Reassignment(std::unique_ptr<Expression> target, std::unique_ptr<Expression> value)
            : target(std::move(target)), value(std::move(value)) {
        }
    };

    class ExpressionStatement : public Statement {
    public:
        std::unique_ptr<Expression> expr;

        explicit ExpressionStatement(std::unique_ptr<Expression> expr)
            : expr(std::move(expr)) {
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

        explicit Directive(std::string n, std::unique_ptr<Expression> val)
            : name(std::move(n)), value(std::move(val)) {
        }
    };

    class ImportStatement : public AstNode {
    public:
        std::string path;

        explicit ImportStatement(std::string p)
            : path(std::move(p)) {
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

        explicit FunctionDefinition(std::string n,
                                    std::vector<FunctionParameter> params,
                                    std::vector<std::unique_ptr<TypeAnnotation> > ret_types,
                                    std::vector<std::unique_ptr<Statement> > b,
                                    std::optional<std::string> docs = std::nullopt)
            : name(std::move(n)), parameters(std::move(params)),
              return_types(std::move(ret_types)), body(std::move(b)), docstring(std::move(docs)) {
        }
    };

    class StructDefinition : public AstNode {
    public:
        std::string name;
        std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > fields;

        explicit StructDefinition(std::string name,
                                  std::vector<std::pair<std::string, std::unique_ptr<TypeAnnotation> > > fields)
            : name(std::move(name)), fields(std::move(fields)) {
        }
    };

    class EnumDefinition : public Statement {
    public:
        std::string name;
        std::unique_ptr<TypeAnnotation> underlying_type;
        std::vector<std::pair<std::string, std::unique_ptr<Expression> > > cases;

        EnumDefinition(std::string name,
                       std::unique_ptr<TypeAnnotation> underlying_type,
                       std::vector<std::pair<std::string, std::unique_ptr<Expression> > > cases)
            : name(std::move(name)),
              underlying_type(std::move(underlying_type)),
              cases(std::move(cases)) {
        }
    };

    class DotAccess : public Expression {
    public:
        std::unique_ptr<Expression> target;
        std::string property_name;

        DotAccess(std::unique_ptr<Expression> target, std::string property_name)
            : target(std::move(target)), property_name(std::move(property_name)) {
        }
    };

    class SwitchExpression : public Expression {
    public:
        std::unique_ptr<Expression> target;
        std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > cases;
        std::unique_ptr<Expression> default_case;

        SwitchExpression(std::unique_ptr<Expression> target,
                         std::vector<std::pair<std::vector<std::string>, std::unique_ptr<Expression> > > cases,
                         std::unique_ptr<Expression> default_case)
            : target(std::move(target)),
              cases(std::move(cases)),
              default_case(std::move(default_case)) {
        }
    };

    class Program : public AstNode {
    public:
        std::vector<std::unique_ptr<ImportStatement> > import_statements;
        std::vector<std::unique_ptr<Directive> > directives;
        std::vector<std::unique_ptr<Statement> > execution_steps;
        std::vector<std::unique_ptr<FunctionDefinition> > function_definitions;
        std::vector<std::unique_ptr<StructDefinition> > struct_definitions;
        std::vector<std::unique_ptr<EnumDefinition> > enum_definitions;
    };
}
