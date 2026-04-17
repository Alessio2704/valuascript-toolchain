#pragma once

#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <utility>
#include "frontend/parser/ast.h"

namespace valuascript::compiler::test {
    using ExprVerifier = std::function<void(Expression *)>;
    using TypeVerifier = std::function<void(TypeAnnotation *)>;
    using StmtVerifier = std::function<void(Statement *)>;

    using FuncVerifier = std::function<void(FunctionDefinition *)>;
    using StructVerifier = std::function<void(StructDefinition *)>;
    using EnumVerifier = std::function<void(EnumDefinition *)>;
    using AliasVerifier = std::function<void(TypeAliasDefinition *)>;
    using ImportVerifier = std::function<void(ImportStatement *)>;
    using DirectiveVerifier = std::function<void(Directive *)>;

    struct ArgSpec {
        std::string label;
        ExprVerifier value_v = nullptr;
    };

    struct ModifierSpec {
        std::string name;
        std::vector<ArgSpec> args = {};
    };

    struct ParamSpec {
        std::string name;
        std::vector<ModifierSpec> mods = {};
        TypeVerifier type_v = nullptr;
        ExprVerifier default_v = nullptr;
    };

    struct FieldSpec {
        std::string name;
        std::vector<ModifierSpec> mods = {};
        TypeVerifier type_v = nullptr;
    };

    struct EnumCaseSpec {
        std::string name;
        std::vector<ModifierSpec> mods = {};
        ExprVerifier value_v = nullptr;
    };

    struct DictItemSpec {
        std::string key;
        std::vector<ModifierSpec> mods = {};
        ExprVerifier value_v = nullptr;
    };

    struct SwitchCaseSpec {
        std::vector<std::string> labels;
        ExprVerifier result_v = nullptr;
    };

    struct AssignmentTargetSpec {
        std::string name;
        TypeVerifier type_v = nullptr;
    };

    struct ProgramSpec {
        std::vector<ImportVerifier> imports = {};
        std::vector<DirectiveVerifier> directives = {};
        std::vector<StmtVerifier> execution_steps = {};
        std::vector<FuncVerifier> functions = {};
        std::vector<StructVerifier> structs = {};
        std::vector<EnumVerifier> enums = {};
        std::vector<AliasVerifier> aliases = {};
    };

    template<typename T>
    T *ExpectNode(AstNode *node) {
        if (!node) {
            ADD_FAILURE() << "Expected AST node of type [" << typeid(T).name() << "], but got[nullptr].";
            return nullptr;
        }
        T *casted = dynamic_cast<T *>(node);
        if (!casted) {
            ADD_FAILURE() << "Expected AST type[" << typeid(T).name() << "], but got [" << typeid(*node).name() << "].";
            return nullptr;
        }
        return casted;
    }

    inline void ExpectNullNode(AstNode *node) {
        EXPECT_EQ(node, nullptr) << "Expected node to be null, but it was populated.";
    }

    inline void ExpectIdentifier(AstNode *node, std::string_view name) {
        if (auto id = ExpectNode<IdentifierAccess>(node))
            EXPECT_EQ(id->name, name);
    }

    inline void ExpectNumber(AstNode *node, std::string_view val) {
        if (auto n = ExpectNode<NumberLiteral>(node))
            EXPECT_EQ(n->value, val);
    }

    inline void ExpectString(AstNode *node, std::string_view val) {
        if (auto s = ExpectNode<StringLiteral>(node))
            EXPECT_EQ(s->value, val);
    }

    inline void ExpectBoolean(AstNode *node, bool val) {
        if (auto b = ExpectNode<BooleanLiteral>(node))
            EXPECT_EQ(b->value, val);
    }

    inline void ExpectPercentage(AstNode *node, std::string_view val) {
        if (auto p = ExpectNode<PercentageLiteral>(node))
            EXPECT_EQ(p->value, val);
    }

    inline void ExpectSelf(AstNode *node) {
        ExpectNode<SelfExpression>(node);
    }

    inline void ExpectArguments(const std::vector<std::pair<std::string, std::unique_ptr<Expression> > > &actual,
                                const std::vector<ArgSpec> &specs) {
        ASSERT_EQ(actual.size(), specs.size()) << "Arg count mismatch.";
        for (size_t i = 0; i < specs.size(); i++) {
            EXPECT_EQ(actual[i].first, specs[i].label);
            if (specs[i].value_v) specs[i].value_v(actual[i].second.get());
        }
    }

    inline void ExpectModifiers(const std::vector<Modifier> &actual, const std::vector<ModifierSpec> &specs) {
        ASSERT_EQ(actual.size(), specs.size()) << "Modifier count mismatch.";
        for (size_t i = 0; i < specs.size(); i++) {
            EXPECT_EQ(actual[i].name, specs[i].name);
            ExpectArguments(actual[i].arguments, specs[i].args);
        }
    }

    inline void ExpectBinary(AstNode *node, shared::TokenType op, const ExprVerifier &l_v, const ExprVerifier &r_v) {
        if (auto b = ExpectNode<BinaryExpression>(node)) {
            EXPECT_EQ(b->op, op);
            if (l_v) l_v(b->left.get());
            if (r_v) r_v(b->right.get());
        }
    }

    inline void ExpectUnary(AstNode *node, shared::TokenType op, const ExprVerifier &r_v) {
        if (auto u = ExpectNode<UnaryExpression>(node)) {
            EXPECT_EQ(u->op, op);
            if (r_v) r_v(u->right.get());
        }
    }

    inline void ExpectGrouping(AstNode *node, const ExprVerifier &inner_v) {
        if (auto g = ExpectNode<GroupingExpression>(node)) {
            if (inner_v) inner_v(g->expression.get());
        }
    }

    inline void ExpectConditional(AstNode *node, const ExprVerifier &c_v, const ExprVerifier &t_v,
                                  const ExprVerifier &e_v) {
        if (auto cond = ExpectNode<ConditionalExpression>(node)) {
            if (c_v) c_v(cond->condition.get());
            if (t_v) t_v(cond->then_branch.get());
            if (e_v) e_v(cond->else_branch.get());
        }
    }

    inline void ExpectCall(AstNode *node, const ExprVerifier &target_v, const std::vector<ArgSpec> &args) {
        if (auto c = ExpectNode<FunctionCall>(node)) {
            if (target_v) target_v(c->target.get());
            ExpectArguments(c->arguments, args);
        }
    }

    inline void ExpectBracketAccess(AstNode *node, const ExprVerifier &target_v, const ExprVerifier &index_v) {
        if (auto b = ExpectNode<BracketAccess>(node)) {
            if (target_v) target_v(b->target.get());
            if (index_v) index_v(b->index.get());
        }
    }

    inline void ExpectDotAccess(AstNode *node, const ExprVerifier &target_v, std::string_view prop) {
        if (auto d = ExpectNode<DotAccess>(node)) {
            if (target_v) target_v(d->target.get());
            EXPECT_EQ(d->property_name, prop);
        }
    }

    inline void ExpectSwitch(AstNode *node, const ExprVerifier &target_v, const std::vector<SwitchCaseSpec> &cases,
                             const ExprVerifier &def_v) {
        if (auto sw = ExpectNode<SwitchExpression>(node)) {
            if (target_v) target_v(sw->target.get());
            ASSERT_EQ(sw->cases.size(), cases.size());
            for (size_t i = 0; i < cases.size(); i++) {
                EXPECT_EQ(sw->cases[i].first, cases[i].labels);
                if (cases[i].result_v) cases[i].result_v(sw->cases[i].second.get());
            }
            if (def_v) def_v(sw->default_case.get());
        }
    }

    inline void ExpectTensor(AstNode *node, const std::vector<ExprVerifier> &elements) {
        if (auto t = ExpectNode<TensorLiteral>(node)) {
            ASSERT_EQ(t->elements.size(), elements.size());
            for (size_t i = 0; i < elements.size(); i++) {
                if (elements[i]) elements[i](t->elements[i].get());
            }
        }
    }

    inline void ExpectTuple(AstNode *node, const std::vector<ExprVerifier> &elements) {
        if (auto t = ExpectNode<TupleLiteral>(node)) {
            ASSERT_EQ(t->elements.size(), elements.size());
            for (size_t i = 0; i < elements.size(); i++) {
                if (elements[i]) elements[i](t->elements[i].get());
            }
        }
    }

    inline void ExpectDict(AstNode *node, const std::vector<DictItemSpec> &specs) {
        if (auto d = ExpectNode<DictLiteral>(node)) {
            ASSERT_EQ(d->elements.size(), specs.size());
            for (size_t i = 0; i < specs.size(); i++) {
                EXPECT_EQ(d->elements[i].key, specs[i].key);
                ExpectModifiers(d->elements[i].modifiers, specs[i].mods);
                if (specs[i].value_v) specs[i].value_v(d->elements[i].value.get());
            }
        }
    }

    inline void ExpectType(TypeAnnotation *node, std::string_view name,
                           const std::vector<TypeVerifier> &generics = {}) {
        ASSERT_NE(node, nullptr);
        EXPECT_EQ(node->name, name);
        ASSERT_EQ(node->generic_args.size(), generics.size()) << "Generic arg count mismatch for " << name;
        for (size_t i = 0; i < generics.size(); i++) {
            if (generics[i]) generics[i](node->generic_args[i].get());
        }
    }

    inline void ExpectTupleType(TypeAnnotation *node, const std::vector<TypeVerifier> &elements) {
        if (auto t = ExpectNode<TupleTypeAnnotation>(node)) {
            ASSERT_EQ(t->element_types.size(), elements.size());
            for (size_t i = 0; i < elements.size(); i++) {
                if (elements[i]) elements[i](t->element_types[i].get());
            }
        }
    }

    inline void ExpectAssignment(Statement *stmt, const std::vector<ModifierSpec> &mods,
                                 const std::vector<AssignmentTargetSpec> &targets, const ExprVerifier &val_v) {
        if (auto a = ExpectNode<Assignment>(stmt)) {
            ExpectModifiers(a->modifiers, mods);
            ASSERT_EQ(a->targets.size(), targets.size());
            for (size_t i = 0; i < targets.size(); i++) {
                EXPECT_EQ(a->targets[i].first, targets[i].name);
                if (targets[i].type_v) targets[i].type_v(a->targets[i].second.get());
            }
            if (val_v) val_v(a->value.get());
        }
    }

    inline void ExpectReassignment(Statement *stmt, const ExprVerifier &target_v, const ExprVerifier &val_v) {
        if (auto r = ExpectNode<Reassignment>(stmt)) {
            if (target_v) target_v(r->target.get());
            if (val_v) val_v(r->value.get());
        }
    }

    inline void ExpectReturn(Statement *stmt, const std::vector<ExprVerifier> &values) {
        if (auto r = ExpectNode<ReturnStatement>(stmt)) {
            ASSERT_EQ(r->values.size(), values.size());
            for (size_t i = 0; i < values.size(); i++) {
                if (values[i]) values[i](r->values[i].get());
            }
        }
    }

    inline void ExpectExprStmt(Statement *stmt, const ExprVerifier &expr_v) {
        if (auto es = ExpectNode<ExpressionStatement>(stmt)) {
            if (expr_v) expr_v(es->expr.get());
        }
    }

    inline void ExpectFunctionDef(FunctionDefinition *f, std::string_view name,
                                  const std::vector<ModifierSpec> &mods,
                                  const std::vector<ParamSpec> &params,
                                  const std::vector<TypeVerifier> &returns,
                                  const std::vector<StmtVerifier> &body,
                                  const std::optional<std::string> &docstring) {
        ASSERT_NE(f, nullptr);
        EXPECT_EQ(f->name, name);
        ExpectModifiers(f->modifiers, mods);
        EXPECT_EQ(f->docstring, docstring);

        ASSERT_EQ(f->parameters.size(), params.size());
        for (size_t i = 0; i < params.size(); i++) {
            EXPECT_EQ(f->parameters[i].name, params[i].name);
            ExpectModifiers(f->parameters[i].modifiers, params[i].mods);
            if (params[i].type_v) params[i].type_v(f->parameters[i].type.get());
            if (params[i].default_v) params[i].default_v(f->parameters[i].default_value.get());
        }

        ASSERT_EQ(f->return_types.size(), returns.size());
        for (size_t i = 0; i < returns.size(); i++) {
            if (returns[i]) returns[i](f->return_types[i].get());
        }

        ASSERT_EQ(f->body.size(), body.size());
        for (size_t i = 0; i < body.size(); i++) {
            if (body[i]) body[i](f->body[i].get());
        }
    }

    inline void ExpectStructDef(StructDefinition *s, std::string_view name, const std::vector<ModifierSpec> &mods,
                                const std::vector<FieldSpec> &fields) {
        ASSERT_NE(s, nullptr);
        EXPECT_EQ(s->name, name);
        ExpectModifiers(s->modifiers, mods);
        ASSERT_EQ(s->fields.size(), fields.size());
        for (size_t i = 0; i < fields.size(); i++) {
            EXPECT_EQ(s->fields[i].name, fields[i].name);
            ExpectModifiers(s->fields[i].modifiers, fields[i].mods);
            if (fields[i].type_v) fields[i].type_v(s->fields[i].type.get());
        }
    }

    inline void ExpectEnumDef(EnumDefinition *e, std::string_view name, const std::vector<ModifierSpec> &mods,
                              const TypeVerifier &und_v, const std::vector<EnumCaseSpec> &cases) {
        ASSERT_NE(e, nullptr);
        EXPECT_EQ(e->name, name);
        ExpectModifiers(e->modifiers, mods);
        if (und_v) und_v(e->underlying_type.get());
        ASSERT_EQ(e->cases.size(), cases.size());
        for (size_t i = 0; i < cases.size(); i++) {
            EXPECT_EQ(e->cases[i].name, cases[i].name);
            ExpectModifiers(e->cases[i].modifiers, cases[i].mods);
            if (cases[i].value_v) cases[i].value_v(e->cases[i].value.get());
        }
    }

    inline void ExpectTypeAlias(TypeAliasDefinition *a, std::string_view name, const std::vector<ModifierSpec> &mods,
                                const TypeVerifier &target_v) {
        ASSERT_NE(a, nullptr);
        EXPECT_EQ(a->name, name);
        ExpectModifiers(a->modifiers, mods);
        if (target_v) target_v(a->target_type.get());
    }

    inline void ExpectImport(ImportStatement *imp, std::string_view path) {
        ASSERT_NE(imp, nullptr);
        EXPECT_EQ(imp->path, path);
    }

    inline void ExpectDirective(Directive *dir, std::string_view name, const ExprVerifier &val_v) {
        ASSERT_NE(dir, nullptr);
        EXPECT_EQ(dir->name, name);
        if (val_v) val_v(dir->value.get());
    }

    inline void ExpectProgram(Program *p, const ProgramSpec &spec) {
        ASSERT_NE(p, nullptr);

        ASSERT_EQ(p->import_statements.size(), spec.imports.size()) << "Import count mismatch";
        for (size_t i = 0; i < spec.imports.size(); i++) {
            if (spec.imports[i]) spec.imports[i](p->import_statements[i].get());
        }

        ASSERT_EQ(p->directives.size(), spec.directives.size()) << "Directive count mismatch";
        for (size_t i = 0; i < spec.directives.size(); i++) {
            if (spec.directives[i]) spec.directives[i](p->directives[i].get());
        }

        ASSERT_EQ(p->execution_steps.size(), spec.execution_steps.size()) << "Execution step count mismatch";
        for (size_t i = 0; i < spec.execution_steps.size(); i++) {
            if (spec.execution_steps[i]) spec.execution_steps[i](p->execution_steps[i].get());
        }

        ASSERT_EQ(p->function_definitions.size(), spec.functions.size()) << "Function count mismatch";
        for (size_t i = 0; i < spec.functions.size(); i++) {
            if (spec.functions[i]) spec.functions[i](p->function_definitions[i].get());
        }

        ASSERT_EQ(p->struct_definitions.size(), spec.structs.size()) << "Struct count mismatch";
        for (size_t i = 0; i < spec.structs.size(); i++) {
            if (spec.structs[i]) spec.structs[i](p->struct_definitions[i].get());
        }

        ASSERT_EQ(p->enum_definitions.size(), spec.enums.size()) << "Enum count mismatch";
        for (size_t i = 0; i < spec.enums.size(); i++) {
            if (spec.enums[i]) spec.enums[i](p->enum_definitions[i].get());
        }

        ASSERT_EQ(p->type_aliases.size(), spec.aliases.size()) << "Type alias count mismatch";
        for (size_t i = 0; i < spec.aliases.size(); i++) {
            if (spec.aliases[i]) spec.aliases[i](p->type_aliases[i].get());
        }
    }

    inline ExprVerifier IsNull() { return [](AstNode *n) { ExpectNullNode(n); }; }
    inline TypeVerifier IsNullType() { return [](TypeAnnotation *n) { ExpectNullNode(n); }; }

    inline ExprVerifier IsNumber(std::string val) {
        return [v = std::move(val)](Expression *n) { ExpectNumber(n, v); };
    }

    inline ExprVerifier IsString(std::string val) {
        return [v = std::move(val)](Expression *n) { ExpectString(n, v); };
    }

    inline ExprVerifier IsBoolean(bool val) { return [val](Expression *n) { ExpectBoolean(n, val); }; }

    inline ExprVerifier IsPercentage(std::string val) {
        return [v = std::move(val)](Expression *n) { ExpectPercentage(n, v); };
    }

    inline ExprVerifier IsIdentifier(std::string val) {
        return [v = std::move(val)](Expression *n) { ExpectIdentifier(n, v); };
    }

    inline ExprVerifier IsSelf() { return [](Expression *n) { ExpectSelf(n); }; }

    inline ExprVerifier IsBinary(shared::TokenType op, ExprVerifier l = nullptr, ExprVerifier r = nullptr) {
        return [op, left = std::move(l), right = std::move(r)](Expression *n) { ExpectBinary(n, op, left, right); };
    }

    inline ExprVerifier IsUnary(shared::TokenType op, ExprVerifier r = nullptr) {
        return [op, right = std::move(r)](Expression *n) { ExpectUnary(n, op, right); };
    }

    inline ExprVerifier IsGrouping(ExprVerifier inner = nullptr) {
        return [i = std::move(inner)](Expression *n) { ExpectGrouping(n, i); };
    }

    inline ExprVerifier IsConditional(ExprVerifier c = nullptr, ExprVerifier t = nullptr, ExprVerifier e = nullptr) {
        return [cond = std::move(c), thn = std::move(t), els = std::move(e)](Expression *n) {
            ExpectConditional(n, cond, thn, els);
        };
    }

    inline ExprVerifier IsCall(ExprVerifier t, std::vector<ArgSpec> args = {}) {
        return [target = std::move(t), a = std::move(args)](Expression *n) { ExpectCall(n, target, a); };
    }

    inline ExprVerifier IsBracket(ExprVerifier t, ExprVerifier idx) {
        return [target = std::move(t), index = std::move(idx)](Expression *n) {
            ExpectBracketAccess(n, target, index);
        };
    }

    inline ExprVerifier IsDot(ExprVerifier t, std::string prop) {
        return [target = std::move(t), p = std::move(prop)](Expression *n) { ExpectDotAccess(n, target, p); };
    }

    inline ExprVerifier IsSwitch(ExprVerifier t, std::vector<SwitchCaseSpec> cases, ExprVerifier def = nullptr) {
        return [target = std::move(t), c = std::move(cases), d = std::move(def)](Expression *n) {
            ExpectSwitch(n, target, c, d);
        };
    }

    inline ExprVerifier IsTensor(std::vector<ExprVerifier> elements = {}) {
        return [e = std::move(elements)](Expression *n) { ExpectTensor(n, e); };
    }

    inline ExprVerifier IsTuple(std::vector<ExprVerifier> elements = {}) {
        return [e = std::move(elements)](Expression *n) { ExpectTuple(n, e); };
    }

    inline ExprVerifier IsDict(std::vector<DictItemSpec> specs = {}) {
        return [s = std::move(specs)](Expression *n) { ExpectDict(n, s); };
    }

    inline TypeVerifier IsType(std::string name, std::vector<TypeVerifier> generics = {}) {
        return [n = std::move(name), g = std::move(generics)](TypeAnnotation *t) { ExpectType(t, n, g); };
    }

    inline TypeVerifier IsTupleType(std::vector<TypeVerifier> elements = {}) {
        return [e = std::move(elements)](TypeAnnotation *t) { ExpectTupleType(t, e); };
    }

    inline StmtVerifier IsAssignment(std::vector<ModifierSpec> mods, std::vector<AssignmentTargetSpec> targets,
                                     ExprVerifier val = nullptr) {
        return [m = std::move(mods), t = std::move(targets), v = std::move(val)](Statement *s) {
            ExpectAssignment(s, m, t, v);
        };
    }

    inline StmtVerifier IsReassignment(ExprVerifier target = nullptr, ExprVerifier val = nullptr) {
        return [t = std::move(target), v = std::move(val)](Statement *s) { ExpectReassignment(s, t, v); };
    }

    inline StmtVerifier IsReturn(std::vector<ExprVerifier> values = {}) {
        return [v = std::move(values)](Statement *s) { ExpectReturn(s, v); };
    }

    inline StmtVerifier IsExprStmt(ExprVerifier expr = nullptr) {
        return [e = std::move(expr)](Statement *s) { ExpectExprStmt(s, e); };
    }

    inline FuncVerifier IsFunctionDef(std::string name, std::vector<ModifierSpec> mods = {},
                                      std::vector<ParamSpec> params = {},
                                      std::vector<TypeVerifier> returns = {}, std::vector<StmtVerifier> body = {},
                                      std::optional<std::string> docstring = std::nullopt) {
        return [n = std::move(name), m = std::move(mods), p = std::move(params), r = std::move(returns), b =
                    std::move(body), d = std::move(docstring)]
        (FunctionDefinition *f) {
            ExpectFunctionDef(f, n, m, p, r, b, d);
        };
    }

    inline StructVerifier IsStructDef(std::string name, std::vector<ModifierSpec> mods = {},
                                      std::vector<FieldSpec> fields = {}) {
        return [n = std::move(name), m = std::move(mods), f = std::move(fields)](StructDefinition *s) {
            ExpectStructDef(s, n, m, f);
        };
    }

    inline EnumVerifier IsEnumDef(std::string name, std::vector<ModifierSpec> mods = {}, TypeVerifier und = nullptr,
                                  std::vector<EnumCaseSpec> cases = {}) {
        return [n = std::move(name), m = std::move(mods), u = std::move(und), c = std::move(cases)](EnumDefinition *e) {
            ExpectEnumDef(e, n, m, u, c);
        };
    }

    inline AliasVerifier IsTypeAlias(std::string name, std::vector<ModifierSpec> mods = {},
                                     TypeVerifier target = nullptr) {
        return [n = std::move(name), m = std::move(mods), t = std::move(target)](TypeAliasDefinition *a) {
            ExpectTypeAlias(a, n, m, t);
        };
    }

    inline ImportVerifier IsImport(std::string path) {
        return [p = std::move(path)](ImportStatement *i) { ExpectImport(i, p); };
    }

    inline DirectiveVerifier IsDirective(std::string name, ExprVerifier val = nullptr) {
        return [n = std::move(name), v = std::move(val)](Directive *d) { ExpectDirective(d, n, v); };
    }

    inline ProgramSpec MergeSpecs(ProgramSpec base, ProgramSpec extension) {
        base.imports.insert(base.imports.end(), extension.imports.begin(), extension.imports.end());
        base.directives.insert(base.directives.end(), extension.directives.begin(), extension.directives.end());
        base.execution_steps.insert(base.execution_steps.end(), extension.execution_steps.begin(),
                                    extension.execution_steps.end());
        base.functions.insert(base.functions.end(), extension.functions.begin(), extension.functions.end());
        base.structs.insert(base.structs.end(), extension.structs.begin(), extension.structs.end());
        base.enums.insert(base.enums.end(), extension.enums.begin(), extension.enums.end());
        base.aliases.insert(base.aliases.end(), extension.aliases.begin(), extension.aliases.end());
        return base;
    }
}
