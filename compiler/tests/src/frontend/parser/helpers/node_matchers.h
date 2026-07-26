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

#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
#include <cstdlib>
#endif

namespace valuascript::compiler::test
{
    inline std::string get_demangled_name(const char* mangled_name)
    {
#if defined(__GNUC__) || defined(__clang__)
        int status = 0;
        char* demangled = abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status);
        std::string result = (status == 0 && demangled) ? demangled : mangled_name;
        std::free(demangled);

        std::string prefix = "valuascript::compiler::";
        if (result.find(prefix) == 0)
        {
            result = result.substr(prefix.length());
        }

        return result;
#else
        return mangled_name;
#endif
    }

    struct ModifierSpec;

    using ExprVerifier = std::function<void(Expression*)>;
    using TypeVerifier = std::function<void(TypeAnnotation*)>;
    using ImportVerifier = std::function<void(ImportStatement*)>;
    using DirectiveVerifier = std::function<void(Directive*)>;
    using FuncVerifier = std::function<void(FunctionDefinition*)>;
    using ExtVerifier = std::function<void(ExtensionDefinition*)>;
    using StructVerifier = std::function<void(StructDefinition*)>;
    using EnumVerifier = std::function<void(EnumDefinition*)>;
    using AliasVerifier = std::function<void(TypeAliasDefinition*)>;

    using ModifierVerifier = std::vector<ModifierSpec>;
    using AssignmentVerifier = std::function<void(Assignment*)>;
    using ReassignmentVerifier = std::function<void(Reassignment*)>;
    using ReturnVerifier = std::function<void(ReturnStatement*)>;
    using ExprStmtVerifier = std::function<void(ExpressionStatement*)>;

    struct ArgSpec
    {
        std::string label;
        ExprVerifier value_v = nullptr;
    };

    struct ModifierSpec
    {
        std::string name;
        std::vector<ArgSpec> args = {};
    };

    struct ParamSpec
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        TypeVerifier type_v = nullptr;
        ExprVerifier default_v = nullptr;
    };

    struct FieldSpec
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        TypeVerifier type_v = nullptr;
    };

    struct EnumCaseSpec
    {
        std::string name;
        std::vector<ModifierSpec> modifiers = {};
        ExprVerifier value_v = nullptr;
    };

    struct DictItemSpec
    {
        std::string key;
        std::vector<ModifierSpec> modifiers = {};
        ExprVerifier value_v = nullptr;
    };

    struct SwitchCaseSpec
    {
        std::vector<ModifierSpec> modifiers = {};
        std::vector<std::string> labels;
        ExprVerifier result_v = nullptr;

        SwitchCaseSpec(std::vector<std::string> l, ExprVerifier r = nullptr)
            : labels(std::move(l)), result_v(std::move(r))
        {
        }

        SwitchCaseSpec(std::vector<ModifierSpec> m, std::vector<std::string> l, ExprVerifier r = nullptr)
            : modifiers(std::move(m)), labels(std::move(l)), result_v(std::move(r))
        {
        }
    };

    struct AssignmentTargetSpec
    {
        std::vector<ModifierSpec> modifiers = {};
        std::string name;
        TypeVerifier type_v = nullptr;

        AssignmentTargetSpec(std::string n, TypeVerifier t = nullptr)
            : name(std::move(n)), type_v(std::move(t))
        {
        }

        AssignmentTargetSpec(std::vector<ModifierSpec> m, std::string n, TypeVerifier t = nullptr)
            : modifiers(std::move(m)), name(std::move(n)), type_v(std::move(t))
        {
        }
    };

    template <typename T>
    T* ExpectNode(AstNode* node)
    {
        if (!node)
        {
            ADD_FAILURE() << "Expected AST node of type [" << get_demangled_name(typeid(T).name())
                << "], but got [nullptr].";
            return nullptr;
        }
        T* casted = dynamic_cast<T*>(node);
        if (!casted)
        {
            ADD_FAILURE() << "Expected AST type [" << get_demangled_name(typeid(T).name())
                << "], but got [" << get_demangled_name(typeid(*node).name()) << "].";
            return nullptr;
        }
        return casted;
    }

    struct StmtVerifier
    {
        std::function<void(Statement*)> checker;

        StmtVerifier() = default;

        StmtVerifier(std::nullptr_t) : checker(nullptr)
        {
        }

        StmtVerifier(std::function<void(Statement*)> f) : checker(std::move(f))
        {
        }

        template <typename T>
            requires std::derived_from<T, Statement>
        StmtVerifier(std::function<void(T*)> specific)
        {
            checker = [specific](Statement* s)
            {
                if (auto* casted = ExpectNode<T>(s))
                {
                    specific(casted);
                }
            };
        }

        void operator()(Statement* s) const
        {
            if (checker) checker(s);
        }

        explicit operator bool() const
        {
            return static_cast<bool>(checker);
        }
    };

    struct ProgramSpec
    {
        std::vector<ImportVerifier> imports = {};
        std::vector<DirectiveVerifier> directives = {};
        std::vector<StmtVerifier> execution_steps = {};
        std::vector<FuncVerifier> functions = {};
        std::vector<StructVerifier> structs = {};
        std::vector<EnumVerifier> enums = {};
        std::vector<AliasVerifier> type_aliases = {};
        std::vector<ExtVerifier> extensions = {};
    };

    inline void ExpectNullNode(AstNode* node)
    {
        EXPECT_EQ(node, nullptr) << "Expected node to be null, but it was populated.";
    }

    struct NullVerifier
    {
        void operator()(AstNode* node) const { ExpectNullNode(node); }
        void operator()(TypeAnnotation* node) const { ExpectNullNode(node); }
    };

    inline void ExpectIdentifier(AstNode* node, std::string_view name)
    {
        if (auto id = ExpectNode<IdentifierAccess>(node))
        {
            EXPECT_EQ(id->name, name) << "Identifier name mismatch.";
        }
    }

    inline void ExpectNumber(AstNode* node, std::string_view val)
    {
        if (auto n = ExpectNode<NumberLiteral>(node))
        {
            EXPECT_EQ(n->value, val) << "Number literal value mismatch.";
        }
    }

    inline void ExpectString(AstNode* node, std::string_view val)
    {
        if (auto s = ExpectNode<StringLiteral>(node))
        {
            EXPECT_EQ(s->value, val) << "String literal value mismatch.";
        }
    }

    inline void ExpectBoolean(AstNode* node, bool val)
    {
        if (auto b = ExpectNode<BooleanLiteral>(node))
        {
            EXPECT_EQ(b->value, val) << "Boolean literal value mismatch.";
        }
    }

    inline void ExpectPercentage(AstNode* node, std::string_view val)
    {
        if (auto p = ExpectNode<PercentageLiteral>(node))
        {
            EXPECT_EQ(p->value, val) << "Percentage literal value mismatch.";
        }
    }

    inline void ExpectSelf(AstNode* node)
    {
        ExpectNode<SelfExpression>(node);
    }

    inline void ExpectArguments(const std::vector<std::pair<std::string, std::unique_ptr<Expression>>>& actual,
                                const std::vector<ArgSpec>& specs)
    {
        ASSERT_EQ(actual.size(), specs.size()) << "Arg count mismatch.";
        for (size_t i = 0; i < specs.size(); i++)
        {
            EXPECT_EQ(actual[i].first, specs[i].label) << "Argument label mismatch at index " << i << ".";
            if (specs[i].value_v) specs[i].value_v(actual[i].second.get());
        }
    }

    inline void ExpectModifiers(const std::vector<Modifier>& actual, const std::vector<ModifierSpec>& specs)
    {
        ASSERT_EQ(actual.size(), specs.size()) << "Modifier count mismatch.";
        for (size_t i = 0; i < specs.size(); i++)
        {
            EXPECT_EQ(actual[i].name, specs[i].name) << "Modifier name mismatch at index " << i << ".";
            ExpectArguments(actual[i].arguments, specs[i].args);
        }
    }

    inline void ExpectBinary(AstNode* node, TokenType op, const ExprVerifier& l_v, const ExprVerifier& r_v)
    {
        if (auto b = ExpectNode<BinaryExpression>(node))
        {
            EXPECT_EQ(b->op, op) << "Binary expression operator mismatch.";
            if (l_v) l_v(b->left.get());
            if (r_v) r_v(b->right.get());
        }
    }

    inline void ExpectUnary(AstNode* node, TokenType op, const ExprVerifier& r_v)
    {
        if (auto u = ExpectNode<UnaryExpression>(node))
        {
            EXPECT_EQ(u->op, op) << "Unary expression operator mismatch.";
            if (r_v) r_v(u->right.get());
        }
    }

    inline void ExpectGrouping(AstNode* node, const ExprVerifier& inner_v)
    {
        if (auto g = ExpectNode<GroupingExpression>(node))
        {
            if (inner_v) inner_v(g->expression.get());
        }
    }

    inline void ExpectConditional(AstNode* node, const ExprVerifier& c_v, const ExprVerifier& t_v,
                                  const ExprVerifier& e_v)
    {
        if (auto cond = ExpectNode<ConditionalExpression>(node))
        {
            if (c_v) c_v(cond->condition.get());
            if (t_v) t_v(cond->then_branch.get());
            if (e_v) e_v(cond->else_branch.get());
        }
    }

    inline void ExpectCall(AstNode* node, const ExprVerifier& target_v, const std::vector<ArgSpec>& args)
    {
        if (auto c = ExpectNode<FunctionCall>(node))
        {
            if (target_v) target_v(c->target.get());
            ExpectArguments(c->arguments, args);
        }
    }

    inline void ExpectBracketAccess(AstNode* node, const ExprVerifier& target_v, const ExprVerifier& index_v)
    {
        if (auto b = ExpectNode<BracketAccess>(node))
        {
            if (target_v) target_v(b->target.get());
            if (index_v) index_v(b->index.get());
        }
    }

    inline void ExpectDotAccess(AstNode* node, const ExprVerifier& target_v, std::string_view prop)
    {
        if (auto d = ExpectNode<DotAccess>(node))
        {
            if (target_v) target_v(d->target.get());
            {
                EXPECT_EQ(d->property_name, prop) << "Dot access property name mismatch.";
            }
        }
    }

    inline void ExpectSwitch(AstNode* node, const ExprVerifier& target_v, const std::vector<SwitchCaseSpec>& cases,
                             const std::vector<ModifierSpec>& default_mods,
                             const ExprVerifier& def_v)
    {
        if (auto sw = ExpectNode<SwitchExpression>(node))
        {
            if (target_v) target_v(sw->target.get());
            {
                ASSERT_EQ(sw->cases.size(), cases.size()) << "Switch cases count mismatch.";
            }
            for (size_t i = 0; i < cases.size(); i++)
            {
                ExpectModifiers(sw->cases[i].modifiers, cases[i].modifiers);
                EXPECT_EQ(sw->cases[i].identifiers, cases[i].labels) << "Switch case labels mismatch at index " << i <<
 ".";
                if (cases[i].result_v) cases[i].result_v(sw->cases[i].result.get());
            }
            ExpectModifiers(sw->default_modifiers, default_mods);
            if (def_v) def_v(sw->default_case.get());
        }
    }

    inline void ExpectTensor(AstNode* node, const std::vector<ExprVerifier>& elements)
    {
        if (auto t = ExpectNode<TensorLiteral>(node))
        {
            ASSERT_EQ(t->elements.size(), elements.size()) << "Tensor elements count mismatch.";
            for (size_t i = 0; i < elements.size(); i++)
            {
                if (elements[i]) elements[i](t->elements[i].get());
            }
        }
    }

    inline void ExpectTuple(AstNode* node, const std::vector<ExprVerifier>& elements)
    {
        if (auto t = ExpectNode<TupleLiteral>(node))
        {
            ASSERT_EQ(t->elements.size(), elements.size()) << "Tuple elements count mismatch.";
            for (size_t i = 0; i < elements.size(); i++)
            {
                if (elements[i]) elements[i](t->elements[i].get());
            }
        }
    }

    inline void ExpectDict(AstNode* node, const std::vector<DictItemSpec>& items)
    {
        if (auto d = ExpectNode<DictLiteral>(node))
        {
            ASSERT_EQ(d->elements.size(), items.size()) << "Dictionary items count mismatch.";
            for (size_t i = 0; i < items.size(); i++)
            {
                EXPECT_EQ(d->elements[i].key, items[i].key) << "Dictionary item key mismatch at index " << i << ".";
                ExpectModifiers(d->elements[i].modifiers, items[i].modifiers);
                if (items[i].value_v) items[i].value_v(d->elements[i].value.get());
            }
        }
    }

    inline void ExpectType(TypeAnnotation* node, std::string_view name,
                           const std::vector<TypeVerifier>& generics = {})
    {
        ASSERT_NE(node, nullptr) << "Expected TypeAnnotation node, but got nullptr.";
        EXPECT_EQ(node->name, name) << "TypeAnnotation name mismatch.";
        ASSERT_EQ(node->generic_args.size(), generics.size()) << "Generic arg count mismatch for type '" << name <<
 "'.";
        for (size_t i = 0; i < generics.size(); i++)
        {
            if (generics[i]) generics[i](node->generic_args[i].get());
        }
    }

    inline void ExpectTupleType(TypeAnnotation* node, const std::vector<TypeVerifier>& elements)
    {
        if (auto t = ExpectNode<TupleTypeAnnotation>(node))
        {
            ASSERT_EQ(t->element_types.size(), elements.size()) << "TupleType element count mismatch.";
            for (size_t i = 0; i < elements.size(); i++)
            {
                if (elements[i]) elements[i](t->element_types[i].get());
            }
        }
    }

    inline void ExpectAssignment(Statement* stmt, const std::vector<AssignmentTargetSpec>& targets,
                                 const ExprVerifier& val_v)
    {
        if (auto a = ExpectNode<Assignment>(stmt))
        {
            ASSERT_EQ(a->targets.size(), targets.size()) << "Assignment targets count mismatch.";
            for (size_t i = 0; i < targets.size(); i++)
            {
                ExpectModifiers(a->targets[i].modifiers, targets[i].modifiers);
                EXPECT_EQ(a->targets[i].name, targets[i].name) << "Assignment target name mismatch at index " << i <<
 ".";
                if (targets[i].type_v) targets[i].type_v(a->targets[i].type.get());
            }
            if (val_v) val_v(a->value.get());
        }
    }

    inline void ExpectReassignment(Statement* stmt, const ExprVerifier& target_v, const ExprVerifier& val_v)
    {
        if (auto r = ExpectNode<Reassignment>(stmt))
        {
            if (target_v) target_v(r->target.get());
            if (val_v) val_v(r->value.get());
        }
    }

    inline void ExpectReturn(Statement* stmt,
                             const std::vector<ModifierSpec>& modifiers,
                             const std::vector<ExprVerifier>& values)
    {
        if (auto r = ExpectNode<ReturnStatement>(stmt))
        {
            ExpectModifiers(r->modifiers, modifiers);
            ASSERT_EQ(r->values.size(), values.size()) << "Return values count mismatch.";
            for (size_t i = 0; i < values.size(); i++)
            {
                if (values[i]) values[i](r->values[i].get());
            }
        }
    }


    inline void ExpectExprStmt(Statement* stmt, const ExprVerifier& expr_v)
    {
        if (auto es = ExpectNode<ExpressionStatement>(stmt))
        {
            if (expr_v) expr_v(es->expr.get());
        }
    }

    inline void ExpectFunctionDef(FunctionDefinition* f, std::string_view name,
                                  const std::vector<ModifierSpec>& modifiers,
                                  const std::vector<ParamSpec>& params,
                                  const std::vector<TypeVerifier>& returns,
                                  const std::vector<StmtVerifier>& body,
                                  const std::optional<std::string>& docstring)
    {
        ASSERT_NE(f, nullptr) << "Expected FunctionDefinition node, but got nullptr.";
        EXPECT_EQ(f->name, name) << "FunctionDefinition name mismatch.";
        ExpectModifiers(f->modifiers, modifiers);
        EXPECT_EQ(f->docstring, docstring) << "FunctionDefinition docstring mismatch for function '" << name << "'.";

        ASSERT_EQ(f->parameters.size(), params.size()) << "FunctionDefinition parameters count mismatch for function '"
 << name << "'.";
        for (size_t i = 0; i < params.size(); i++)
        {
            EXPECT_EQ(f->parameters[i].name, params[i].name) << "Function parameter name mismatch at index " << i <<
 " for function '" << name << "'.";
            ExpectModifiers(f->parameters[i].modifiers, params[i].modifiers);
            if (params[i].type_v) params[i].type_v(f->parameters[i].type.get());
            if (params[i].default_v) params[i].default_v(f->parameters[i].default_value.get());
        }

        ASSERT_EQ(f->return_types.size(), returns.size()) <<
 "FunctionDefinition return types count mismatch for function '" << name << "'.";
        for (size_t i = 0; i < returns.size(); i++)
        {
            if (returns[i]) returns[i](f->return_types[i].get());
        }

        ASSERT_EQ(f->body.size(), body.size()) << "FunctionDefinition body statements count mismatch for function '" <<
 name << "'.";
        for (size_t i = 0; i < body.size(); i++)
        {
            if (body[i]) body[i](f->body[i].get());
        }
    }

    inline void ExpectExtensionDef(ExtensionDefinition* e,
                                  const std::vector<ModifierSpec>& modifiers,
                                  const TypeVerifier& target,
                                  const ProgramSpec& spec)
    {
        ASSERT_NE(e, nullptr) << "Expected ExtensionDefinition node, but got nullptr.";
        ExpectModifiers(e->modifiers, modifiers);
        if (target) target(e->target_type.get());

        ASSERT_EQ(e->execution_steps.size(), spec.execution_steps.size()) << "Execution steps count mismatch in Extension.";
        for (size_t i = 0; i < spec.execution_steps.size(); i++)
            if (spec.execution_steps[i]) spec.execution_steps[i](e->execution_steps[i].get());

        ASSERT_EQ(e->function_definitions.size(), spec.functions.size()) << "Functions count mismatch in Extension.";
        for (size_t i = 0; i < spec.functions.size(); i++)
            if (spec.functions[i]) spec.functions[i](e->function_definitions[i].get());

        ASSERT_EQ(e->struct_definitions.size(), spec.structs.size()) << "Structs count mismatch in Extension.";
        for (size_t i = 0; i < spec.structs.size(); i++)
            if (spec.structs[i]) spec.structs[i](e->struct_definitions[i].get());

        ASSERT_EQ(e->enum_definitions.size(), spec.enums.size()) << "Enums count mismatch in Extension.";
        for (size_t i = 0; i < spec.enums.size(); i++)
            if (spec.enums[i]) spec.enums[i](e->enum_definitions[i].get());

        ASSERT_EQ(e->type_aliases.size(), spec.type_aliases.size()) << "Type aliases count mismatch in Extension.";
        for (size_t i = 0; i < spec.type_aliases.size(); i++)
            if (spec.type_aliases[i]) spec.type_aliases[i](e->type_aliases[i].get());
    }

    inline void ExpectStructDef(StructDefinition* s, std::string_view name, const std::vector<ModifierSpec>& modifiers,
                                const std::vector<FieldSpec>& fields)
    {
        ASSERT_NE(s, nullptr) << "Expected StructDefinition node, but got nullptr.";
        EXPECT_EQ(s->name, name) << "StructDefinition name mismatch.";
        ExpectModifiers(s->modifiers, modifiers);
        ASSERT_EQ(s->fields.size(), fields.size()) << "StructDefinition fields count mismatch for struct '" << name <<
 "'.";
        for (size_t i = 0; i < fields.size(); i++)
        {
            EXPECT_EQ(s->fields[i].name, fields[i].name) << "Struct field name mismatch at index " << i <<
 " for struct '" << name << "'.";
            ExpectModifiers(s->fields[i].modifiers, fields[i].modifiers);
            if (fields[i].type_v) fields[i].type_v(s->fields[i].type.get());
        }
    }

    inline void ExpectEnumDef(EnumDefinition* e, std::string_view name, const std::vector<ModifierSpec>& modifiers,
                              const TypeVerifier& und_v, const std::vector<EnumCaseSpec>& cases)
    {
        ASSERT_NE(e, nullptr) << "Expected EnumDefinition node, but got nullptr.";
        EXPECT_EQ(e->name, name) << "EnumDefinition name mismatch.";
        ExpectModifiers(e->modifiers, modifiers);
        if (und_v) und_v(e->underlying_type.get());
        ASSERT_EQ(e->cases.size(), cases.size()) << "EnumDefinition cases count mismatch for enum '" << name << "'.";
        for (size_t i = 0; i < cases.size(); i++)
        {
            EXPECT_EQ(e->cases[i].name, cases[i].name) << "Enum case name mismatch at index " << i << " for enum '" <<
 name << "'.";
            ExpectModifiers(e->cases[i].modifiers, cases[i].modifiers);
            if (cases[i].value_v) cases[i].value_v(e->cases[i].value.get());
        }
    }

    inline void ExpectTypeAlias(TypeAliasDefinition* a, std::string_view name,
                                const std::vector<ModifierSpec>& modifiers,
                                const TypeVerifier& target_v)
    {
        ASSERT_NE(a, nullptr) << "Expected TypeAliasDefinition node, but got nullptr.";
        EXPECT_EQ(a->name, name) << "TypeAliasDefinition name mismatch.";
        ExpectModifiers(a->modifiers, modifiers);
        if (target_v) target_v(a->target_type.get());
    }

    inline void ExpectImport(ImportStatement* imp,
                             const std::vector<ModifierSpec>& modifiers,
                             std::string_view path)
    {
        ASSERT_NE(imp, nullptr) << "Expected ImportStatement node, but got nullptr.";
        ExpectModifiers(imp->modifiers, modifiers);
        EXPECT_EQ(imp->path, path) << "ImportStatement path mismatch.";
    }

    inline void ExpectDirective(Directive* dir, std::string_view name, const ExprVerifier& val_v)
    {
        ASSERT_NE(dir, nullptr) << "Expected Directive node, but got nullptr.";
        EXPECT_EQ(dir->name, name) << "Directive name mismatch.";
        if (val_v) val_v(dir->value.get());
    }

    inline void ExpectProgram(const Program* p, const ProgramSpec& spec)
    {
        ASSERT_NE(p, nullptr) << "Expected Program node, but got nullptr.";

        ASSERT_EQ(p->import_statements.size(), spec.imports.size()) << "Program import count mismatch.";
        for (size_t i = 0; i < spec.imports.size(); i++)
        {
            if (spec.imports[i]) spec.imports[i](p->import_statements[i].get());
        }

        ASSERT_EQ(p->directives.size(), spec.directives.size()) << "Program directive count mismatch.";
        for (size_t i = 0; i < spec.directives.size(); i++)
        {
            if (spec.directives[i]) spec.directives[i](p->directives[i].get());
        }

        ASSERT_EQ(p->execution_steps.size(), spec.execution_steps.size()) << "Program execution step count mismatch.";
        for (size_t i = 0; i < spec.execution_steps.size(); i++)
        {
            if (spec.execution_steps[i]) spec.execution_steps[i](p->execution_steps[i].get());
        }

        ASSERT_EQ(p->function_definitions.size(), spec.functions.size()) << "Program function count mismatch.";
        for (size_t i = 0; i < spec.functions.size(); i++)
        {
            if (spec.functions[i]) spec.functions[i](p->function_definitions[i].get());
        }

        ASSERT_EQ(p->struct_definitions.size(), spec.structs.size()) << "Program struct count mismatch.";
        for (size_t i = 0; i < spec.structs.size(); i++)
        {
            if (spec.structs[i]) spec.structs[i](p->struct_definitions[i].get());
        }

        ASSERT_EQ(p->enum_definitions.size(), spec.enums.size()) << "Program enum count mismatch.";
        for (size_t i = 0; i < spec.enums.size(); i++)
        {
            if (spec.enums[i]) spec.enums[i](p->enum_definitions[i].get());
        }

        ASSERT_EQ(p->type_aliases.size(), spec.type_aliases.size()) << "Program type alias count mismatch.";
        for (size_t i = 0; i < spec.type_aliases.size(); i++)
        {
            if (spec.type_aliases[i]) spec.type_aliases[i](p->type_aliases[i].get());
        }
    }

    inline NullVerifier IsNull() { return NullVerifier{}; }
    inline NullVerifier IsNullType() { return NullVerifier{}; }

    inline ExprVerifier IsNumber(std::string value)
    {
        return [v = std::move(value)](Expression* node) { ExpectNumber(node, v); };
    }

    inline ExprVerifier IsString(std::string val)
    {
        return [v = std::move(val)](Expression* node) { ExpectString(node, v); };
    }

    inline ExprVerifier IsBoolean(bool val) { return [val](Expression* node) { ExpectBoolean(node, val); }; }

    inline ExprVerifier IsPercentage(std::string val)
    {
        return [v = std::move(val)](Expression* node) { ExpectPercentage(node, v); };
    }

    inline ExprVerifier IsIdentifier(std::string val)
    {
        return [v = std::move(val)](Expression* node) { ExpectIdentifier(node, v); };
    }

    inline ExprVerifier IsSelf() { return [](Expression* node) { ExpectSelf(node); }; }

    inline ExprVerifier IsBinary(TokenType op, ExprVerifier l = nullptr, ExprVerifier r = nullptr)
    {
        return [op, left = std::move(l), right = std::move(r)](Expression* node)
        {
            ExpectBinary(node, op, left, right);
        };
    }

    inline ExprVerifier IsUnary(TokenType op, ExprVerifier r = nullptr)
    {
        return [op, right = std::move(r)](Expression* node) { ExpectUnary(node, op, right); };
    }

    inline ExprVerifier IsGrouping(ExprVerifier inner = nullptr)
    {
        return [i = std::move(inner)](Expression* node) { ExpectGrouping(node, i); };
    }

    inline ExprVerifier IsConditional(ExprVerifier condition = nullptr, ExprVerifier then_expr = nullptr,
                                      ExprVerifier else_expr = nullptr)
    {
        return [cond = std::move(condition), thn = std::move(then_expr), els = std::move(else_expr)](Expression* node)
        {
            ExpectConditional(node, cond, thn, els);
        };
    }

    inline ExprVerifier IsCall(ExprVerifier target, std::vector<ArgSpec> args = {})
    {
        return [t = std::move(target), a = std::move(args)](Expression* node) { ExpectCall(node, t, a); };
    }

    inline ExprVerifier IsBracket(ExprVerifier target, ExprVerifier idx)
    {
        return [t = std::move(target), index = std::move(idx)](Expression* node)
        {
            ExpectBracketAccess(node, t, index);
        };
    }

    inline ExprVerifier IsDot(ExprVerifier target, std::string property)
    {
        return [t = std::move(target), p = std::move(property)](Expression* node)
        {
            ExpectDotAccess(node, t, p);
        };
    }

    inline ExprVerifier IsSwitch(ExprVerifier t, std::vector<SwitchCaseSpec> cases,
                                 std::vector<ModifierSpec> default_mods,
                                 ExprVerifier default_expr = nullptr)
    {
        return [target = std::move(t), c = std::move(cases), mods = std::move(default_mods), d = std::move(default_expr)
            ](Expression* node)
        {
            ExpectSwitch(node, target, c, mods, d);
        };
    }

    inline ExprVerifier IsSwitch(ExprVerifier t, std::vector<SwitchCaseSpec> cases,
                                 ExprVerifier default_expr = nullptr)
    {
        return IsSwitch(std::move(t), std::move(cases), {}, std::move(default_expr));
    }

    inline ExprVerifier IsTensor(std::vector<ExprVerifier> elements = {})
    {
        return [e = std::move(elements)](Expression* node) { ExpectTensor(node, e); };
    }

    inline ExprVerifier IsTuple(std::vector<ExprVerifier> elements = {})
    {
        return [e = std::move(elements)](Expression* node) { ExpectTuple(node, e); };
    }

    inline ExprVerifier IsDict(std::vector<DictItemSpec> items = {})
    {
        return [i = std::move(items)](Expression* node) { ExpectDict(node, i); };
    }

    inline TypeVerifier IsType(std::string name, std::vector<TypeVerifier> generics = {})
    {
        return [n = std::move(name), g = std::move(generics)](TypeAnnotation* t) { ExpectType(t, n, g); };
    }

    inline TypeVerifier IsTupleType(std::vector<TypeVerifier> elements = {})
    {
        return [e = std::move(elements)](TypeAnnotation* t) { ExpectTupleType(t, e); };
    }

    inline AssignmentVerifier IsAssignment(std::vector<AssignmentTargetSpec> targets,
                                           ExprVerifier value = nullptr)
    {
        return [t = std::move(targets), v = std::move(value)](Statement* s)
        {
            ExpectAssignment(s, t, v);
        };
    }

    inline ReassignmentVerifier IsReassignment(ExprVerifier target = nullptr, ExprVerifier value = nullptr)
    {
        return [t = std::move(target), v = std::move(value)](Statement* s) { ExpectReassignment(s, t, v); };
    }

    inline ReturnVerifier IsReturn(std::vector<ModifierSpec> modifiers = {},
                                   std::vector<ExprVerifier> values = {})
    {
        return [m = std::move(modifiers), v = std::move(values)](Statement* s) { ExpectReturn(s, m, v); };
    }

    inline ReturnVerifier IsReturn(std::vector<ExprVerifier> values = {})
    {
        return IsReturn({}, std::move(values));
    }

    inline ExprStmtVerifier IsExprStmt(ExprVerifier expr = nullptr)
    {
        return [e = std::move(expr)](Statement* s) { ExpectExprStmt(s, e); };
    }

    inline FuncVerifier IsFunctionDef(std::string name,
                                      std::vector<ModifierSpec> modifiers = {},
                                      std::vector<ParamSpec> params = {},
                                      std::vector<TypeVerifier> returns = {},
                                      std::vector<StmtVerifier> body = {},
                                      std::optional<std::string> docstring = std::nullopt)
    {
        return [n = std::move(name), m = std::move(modifiers), p = std::move(params), r = std::move(returns), b =
                std::move(body), d = std::move(docstring)]
        (FunctionDefinition* f)
        {
            ExpectFunctionDef(f, n, m, p, r, b, d);
        };
    }

    inline StructVerifier IsStructDef(std::string name, std::vector<ModifierSpec> modifiers = {},
                                      std::vector<FieldSpec> fields = {})
    {
        return [n = std::move(name), m = std::move(modifiers), f = std::move(fields)](StructDefinition* s)
        {
            ExpectStructDef(s, n, m, f);
        };
    }

    inline EnumVerifier IsEnumDef(std::string name, std::vector<ModifierSpec> modifiers = {},
                                  TypeVerifier type = nullptr,
                                  std::vector<EnumCaseSpec> cases = {})
    {
        return [n = std::move(name), m = std::move(modifiers), u = std::move(type), c = std::move(cases)
            ](EnumDefinition* e)
        {
            ExpectEnumDef(e, n, m, u, c);
        };
    }

    inline AliasVerifier IsTypeAlias(std::string name, std::vector<ModifierSpec> modifiers = {},
                                     TypeVerifier target = nullptr)
    {
        return [n = std::move(name), m = std::move(modifiers), t = std::move(target)](TypeAliasDefinition* a)
        {
            ExpectTypeAlias(a, n, m, t);
        };
    }

    inline ExtVerifier IsExtensionDef(std::vector<ModifierSpec> modifiers = {},
                                      TypeVerifier target = nullptr,
                                      ProgramSpec spec = {})
    {
        return [m = std::move(modifiers), t = std::move(target), s = std::move(spec)](ExtensionDefinition* e)
        {
            ExpectExtensionDef(e, m, t, s);
        };
    }

    inline ImportVerifier IsImport(std::string path, std::vector<ModifierSpec> modifiers = {})
    {
        return [p = std::move(path), m = std::move(modifiers)](ImportStatement* i) { ExpectImport(i, m, p); };
    }

    inline DirectiveVerifier IsDirective(std::string name, ExprVerifier value = nullptr)
    {
        return [n = std::move(name), v = std::move(value)](Directive* d) { ExpectDirective(d, n, v); };
    }

    inline ProgramSpec MergeSpecs(ProgramSpec base, ProgramSpec extension)
    {
        base.imports.insert(base.imports.end(), extension.imports.begin(), extension.imports.end());
        base.directives.insert(base.directives.end(), extension.directives.begin(), extension.directives.end());
        base.execution_steps.insert(base.execution_steps.end(), extension.execution_steps.begin(),
                                    extension.execution_steps.end());
        base.functions.insert(base.functions.end(), extension.functions.begin(), extension.functions.end());
        base.structs.insert(base.structs.end(), extension.structs.begin(), extension.structs.end());
        base.enums.insert(base.enums.end(), extension.enums.begin(), extension.enums.end());
        base.type_aliases.insert(base.type_aliases.end(), extension.type_aliases.begin(), extension.type_aliases.end());
        base.extensions.insert(base.extensions.end(), extension.extensions.begin(), extension.extensions.end());
        return base;
    }
}
