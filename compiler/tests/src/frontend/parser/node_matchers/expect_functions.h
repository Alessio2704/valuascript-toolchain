#pragma once

#include "core.h"
#include "specs.h"

namespace valuascript::compiler::test
{
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

    inline void ExpectArguments(std::span<const CallArgument> actual,
                                std::span<const ArgSpec> specs)
    {
        ASSERT_EQ(actual.size(), specs.size()) << "Arg count mismatch.";
        for (size_t i = 0; i < specs.size(); i++)
        {
            EXPECT_EQ(actual[i].name, specs[i].label.get()) << "Argument label mismatch at index " << i << ".";
            if (specs[i].value_v) specs[i].value_v(actual[i].value.get());
        }
    }

    inline void ExpectModifiers(std::span<const Modifier> actual, std::span<const ModifierSpec> specs)
    {
        ASSERT_EQ(actual.size(), specs.size()) << "Modifier count mismatch.";
        for (size_t i = 0; i < specs.size(); i++)
        {
            EXPECT_EQ(actual[i].name, specs[i].name.get()) << "Modifier name mismatch at index " << i << ".";
            ExpectArguments(actual[i].arguments, specs[i].args);
        }
    }

    template <ExprMatcher L, ExprMatcher R>
    inline void ExpectBinary(AstNode* node, TokenType op, const L& l_v, const R& r_v)
    {
        if (auto b = ExpectNode<BinaryExpression>(node))
        {
            EXPECT_EQ(b->op, op) << "Binary expression operator mismatch.";
            if (l_v) l_v(b->left.get());
            if (r_v) r_v(b->right.get());
        }
    }

    template <ExprMatcher R>
    inline void ExpectUnary(AstNode* node, TokenType op, const R& r_v)
    {
        if (auto u = ExpectNode<UnaryExpression>(node))
        {
            EXPECT_EQ(u->op, op) << "Unary expression operator mismatch.";
            if (r_v) r_v(u->right.get());
        }
    }

    template <ExprMatcher I>
    inline void ExpectGrouping(AstNode* node, const I& inner_v)
    {
        if (auto g = ExpectNode<GroupingExpression>(node))
        {
            if (inner_v) inner_v(g->expression.get());
        }
    }

    template <ExprMatcher C, ExprMatcher T, ExprMatcher E>
    inline void ExpectConditional(AstNode* node, const C& c_v, const T& t_v, const E& e_v)
    {
        if (auto cond = ExpectNode<ConditionalExpression>(node))
        {
            if (c_v) c_v(cond->condition.get());
            if (t_v) t_v(cond->then_branch.get());
            if (e_v) e_v(cond->else_branch.get());
        }
    }

    template <ExprMatcher T>
    inline void ExpectCall(AstNode* node, const T& target_v, std::span<const ArgSpec> args)
    {
        if (auto c = ExpectNode<FunctionCall>(node))
        {
            if (target_v) target_v(c->target.get());
            ExpectArguments(c->arguments, args);
        }
    }

    template <ExprMatcher T, ExprMatcher I>
    inline void ExpectBracketAccess(AstNode* node, const T& target_v, const I& index_v)
    {
        if (auto b = ExpectNode<BracketAccess>(node))
        {
            if (target_v) target_v(b->target.get());
            if (index_v) index_v(b->index.get());
        }
    }

    template <ExprMatcher T>
    inline void ExpectDotAccess(AstNode* node, const T& target_v, std::string_view prop)
    {
        if (auto d = ExpectNode<DotAccess>(node))
        {
            if (target_v) target_v(d->target.get());
            EXPECT_EQ(d->property_name, prop) << "Dot access property name mismatch.";
        }
    }

    template <ExprMatcher T, ExprMatcher D>
    inline void ExpectSwitch(AstNode* node, const T& target_v, std::span<const SwitchCaseSpec> cases,
                             std::span<const ModifierSpec> default_mods,
                             const D& def_v)
    {
        if (auto sw = ExpectNode<SwitchExpression>(node))
        {
            if (target_v) target_v(sw->target.get());
            ASSERT_EQ(sw->cases.size(), cases.size()) << "Switch cases count mismatch.";
            for (size_t i = 0; i < cases.size(); i++)
            {
                ExpectModifiers(sw->cases[i].modifiers, cases[i].modifiers);
                ASSERT_EQ(sw->cases[i].identifiers.size(), cases[i].labels.size());
                for (size_t l = 0; l < cases[i].labels.size(); l++)
                {
                    EXPECT_EQ(sw->cases[i].identifiers[l], cases[i].labels[l].get()) << "Switch case label mismatch.";
                }
                if (cases[i].result_v) cases[i].result_v(sw->cases[i].result.get());
            }
            ExpectModifiers(sw->default_modifiers, default_mods);
            if (def_v) def_v(sw->default_case.get());
        }
    }

    inline void ExpectTensor(AstNode* node, std::span<const ExprVerifier> elements)
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

    inline void ExpectTuple(AstNode* node, std::span<const ExprVerifier> elements)
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

    inline void ExpectDict(AstNode* node, std::span<const DictItemSpec> items)
    {
        if (auto d = ExpectNode<DictLiteral>(node))
        {
            std::string keys;
            for (auto& el : d->elements) keys += "'" + el.key + "', ";
            ASSERT_EQ(d->elements.size(), items.size()) << "Dictionary items count mismatch. Got keys: " << keys;
            for (size_t i = 0; i < items.size(); i++)
            {
                EXPECT_EQ(d->elements[i].key, items[i].key.get()) << "Dictionary item key mismatch at index " << i << ".";
                ExpectModifiers(d->elements[i].modifiers, items[i].modifiers);
                if (items[i].value_v) items[i].value_v(d->elements[i].value.get());
            }
        }
    }

    inline void ExpectType(TypeAnnotation* node, std::string_view name,
                           std::span<const TypeVerifier> generics = {})
    {
        ASSERT_NE(node, nullptr) << "Expected TypeAnnotation node, but got nullptr.";
        EXPECT_EQ(node->name, name) << "TypeAnnotation name mismatch.";
        ASSERT_EQ(node->generic_args.size(), generics.size()) << "Generic arg count mismatch for type '" << name << "'.";
        for (size_t i = 0; i < generics.size(); i++)
        {
            if (generics[i]) generics[i](node->generic_args[i].get());
        }
    }

    inline void ExpectTupleType(TypeAnnotation* node, std::span<const TypeVerifier> elements)
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

    template <ExprMatcher V>
    inline void ExpectAssignment(Statement* stmt, std::span<const AssignmentTargetSpec> targets,
                                 const V& val_v)
    {
        if (auto a = ExpectNode<Assignment>(stmt))
        {
            ASSERT_EQ(a->targets.size(), targets.size()) << "Assignment targets count mismatch.";
            for (size_t i = 0; i < targets.size(); i++)
            {
                ExpectModifiers(a->targets[i].modifiers, targets[i].modifiers);
                EXPECT_EQ(a->targets[i].name, targets[i].name.get()) << "Assignment target name mismatch at index " << i << ".";
                if (targets[i].type_v) targets[i].type_v(a->targets[i].type.get());
            }
            if (val_v) val_v(a->value.get());
        }
    }

    template <ExprMatcher T, ExprMatcher V>
    inline void ExpectReassignment(Statement* stmt, const T& target_v, const V& val_v)
    {
        if (auto r = ExpectNode<Reassignment>(stmt))
        {
            if (target_v) target_v(r->target.get());
            if (val_v) val_v(r->value.get());
        }
    }

    inline void ExpectReturn(Statement* stmt,
                             std::span<const ModifierSpec> modifiers,
                             std::span<const ExprVerifier> values)
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

    template <ExprMatcher E>
    inline void ExpectExprStmt(Statement* stmt, const E& expr_v)
    {
        if (auto es = ExpectNode<ExpressionStatement>(stmt))
        {
            if (expr_v) expr_v(es->expr.get());
        }
    }

    inline void ExpectFunctionDef(FunctionDefinition* f, std::string_view name,
                                  std::span<const ModifierSpec> modifiers,
                                  std::span<const ParamSpec> params,
                                  std::span<const TypeVerifier> returns,
                                  std::span<const StmtVerifier> body,
                                  const std::optional<StringStorage>& docstring)
    {
        ASSERT_NE(f, nullptr) << "Expected FunctionDefinition node, but got nullptr.";
        EXPECT_EQ(f->name, name) << "FunctionDefinition name mismatch.";
        ExpectModifiers(f->modifiers, modifiers);
        if (docstring.has_value())
        {
            EXPECT_EQ(f->docstring, docstring->get()) << "FunctionDefinition docstring mismatch for function '" << name << "'.";
        }
        else
        {
            EXPECT_EQ(f->docstring, std::nullopt) << "FunctionDefinition docstring mismatch for function '" << name << "'.";
        }

        ASSERT_EQ(f->parameters.size(), params.size()) << "FunctionDefinition parameters count mismatch for function '"
            << name << "'.";
        for (size_t i = 0; i < params.size(); i++)
        {
            EXPECT_EQ(f->parameters[i].name, params[i].name.get()) << "Function parameter name mismatch at index " << i
                << " for function '" << name << "'.";
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
                                   std::span<const ModifierSpec> modifiers,
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

    inline void ExpectStructDef(StructDefinition* s, std::string_view name,
                                std::span<const ModifierSpec> modifiers,
                                std::span<const FieldSpec> fields)
    {
        ASSERT_NE(s, nullptr) << "Expected StructDefinition node, but got nullptr.";
        EXPECT_EQ(s->name, name) << "StructDefinition name mismatch.";
        ExpectModifiers(s->modifiers, modifiers);
        ASSERT_EQ(s->fields.size(), fields.size()) << "StructDefinition fields count mismatch for struct '" << name << "'.";
        for (size_t i = 0; i < fields.size(); i++)
        {
            EXPECT_EQ(s->fields[i].name, fields[i].name.get()) << "Struct field name mismatch at index " << i
                << " for struct '" << name << "'.";
            ExpectModifiers(s->fields[i].modifiers, fields[i].modifiers);
            if (fields[i].type_v) fields[i].type_v(s->fields[i].type.get());
        }
    }

    inline void ExpectEnumDef(EnumDefinition* e, std::string_view name,
                              std::span<const ModifierSpec> modifiers,
                              const TypeVerifier& und_v,
                              std::span<const EnumCaseSpec> cases)
    {
        ASSERT_NE(e, nullptr) << "Expected EnumDefinition node, but got nullptr.";
        EXPECT_EQ(e->name, name) << "EnumDefinition name mismatch.";
        ExpectModifiers(e->modifiers, modifiers);
        if (und_v) und_v(e->underlying_type.get());
        ASSERT_EQ(e->cases.size(), cases.size()) << "EnumDefinition cases count mismatch for enum '" << name << "'.";
        for (size_t i = 0; i < cases.size(); i++)
        {
            EXPECT_EQ(e->cases[i].name, cases[i].name.get()) << "Enum case name mismatch at index " << i
                << " for enum '" << name << "'.";
            ExpectModifiers(e->cases[i].modifiers, cases[i].modifiers);
            if (cases[i].value_v) cases[i].value_v(e->cases[i].value.get());
        }
    }

    inline void ExpectTypeAlias(TypeAliasDefinition* a, std::string_view name,
                                std::span<const ModifierSpec> modifiers,
                                const TypeVerifier& target_v)
    {
        ASSERT_NE(a, nullptr) << "Expected TypeAliasDefinition node, but got nullptr.";
        EXPECT_EQ(a->name, name) << "TypeAliasDefinition name mismatch.";
        ExpectModifiers(a->modifiers, modifiers);
        if (target_v) target_v(a->target_type.get());
    }

    inline void ExpectImport(ImportStatement* imp,
                             std::span<const ModifierSpec> modifiers,
                             std::string_view path)
    {
        ASSERT_NE(imp, nullptr) << "Expected ImportStatement node, but got nullptr.";
        ExpectModifiers(imp->modifiers, modifiers);
        EXPECT_EQ(imp->path, path) << "ImportStatement path mismatch.";
    }

    template <ExprMatcher V>
    inline void ExpectDirective(Directive* dir, std::string_view name, const V& val_v)
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
}
