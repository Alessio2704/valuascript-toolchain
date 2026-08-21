#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <cstddef>

#include "token/source_span.h"
#include "token/token_type.h"
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"

namespace valuascript::compiler::test
{
    class DummyAstGeneratorState
    {
    public:
        static size_t next_id()
        {
            static size_t id = 100;
            return ++id;
        }

        static SourceSpan make_span(int depth = 0)
        {
            size_t id = next_id();
            SourceSpan sp{};
            sp.line_start = (id % 1000) + 1;
            sp.column_start = (static_cast<size_t>(depth) * 4) + 1;
            sp.line_end = sp.line_start + (depth > 0 ? 0 : 1);
            sp.column_end = sp.column_start + 12;
            sp.start_offset = id * 20;
            sp.length = 12;
            sp.file_path = std::make_shared<const std::string>("dummy_source.vs");
            return sp;
        }

        static NodeName make_name(std::string_view prefix = "dummy_id", int depth = 0)
        {
            size_t id = next_id();
            std::string str = std::string(prefix) + "_" + std::to_string(id);
            return NodeName{std::move(str), make_span(depth)};
        }
    };

    inline SourceSpan dummy_span(int depth = 0)
    {
        return DummyAstGeneratorState::make_span(depth);
    }

    inline NodeName dummy_name(std::string_view prefix = "node", int depth = 0)
    {
        return DummyAstGeneratorState::make_name(prefix, depth);
    }

    inline ExprPtr dummy_expr(int depth = 0);
    inline StmtPtr dummy_stmt(int depth = 0);
    inline TypeAnnPtr dummy_type(int depth = 0);

    inline CallArgument dummy_call_arg(int depth = 0);
    inline Modifier dummy_modifier(int depth = 0);
    inline FunctionParameter dummy_param(int depth = 0);
    inline StructField dummy_field(int depth = 0);
    inline EnumCase dummy_enum_case(int depth = 0);
    inline DictItem dummy_dict_item(int depth = 0);
    inline SwitchCase dummy_switch_case(int depth = 0);
    inline AssignmentTarget dummy_assignment_target(int depth = 0);

    inline std::vector<CallArgument> dummy_call_args(int count = 2, int depth = 0)
    {
        std::vector<CallArgument> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_call_arg(depth + 1));
        return vec;
    }

    inline std::vector<Modifier> dummy_modifiers(int count = 2, int depth = 0)
    {
        std::vector<Modifier> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_modifier(depth + 1));
        return vec;
    }

    inline std::vector<FunctionParameter> dummy_params(int count = 2, int depth = 0)
    {
        std::vector<FunctionParameter> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_param(depth + 1));
        return vec;
    }

    inline std::vector<StructField> dummy_fields(int count = 2, int depth = 0)
    {
        std::vector<StructField> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_field(depth + 1));
        return vec;
    }

    inline std::vector<EnumCase> dummy_enum_cases(int count = 2, int depth = 0)
    {
        std::vector<EnumCase> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_enum_case(depth + 1));
        return vec;
    }

    inline std::vector<DictItem> dummy_dict_items(int count = 2, int depth = 0)
    {
        std::vector<DictItem> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_dict_item(depth + 1));
        return vec;
    }

    inline std::vector<SwitchCase> dummy_switch_cases(int count = 2, int depth = 0)
    {
        std::vector<SwitchCase> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_switch_case(depth + 1));
        return vec;
    }

    inline std::vector<AssignmentTarget> dummy_assignment_targets(int count = 3, int depth = 0)
    {
        std::vector<AssignmentTarget> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_assignment_target(depth + 1));
        return vec;
    }

    inline std::vector<TypeAnnPtr> dummy_type_vec(int count = 2, int depth = 0)
    {
        std::vector<TypeAnnPtr> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_type(depth + 1));
        return vec;
    }

    inline std::vector<StmtPtr> dummy_stmt_vec(int count = 2, int depth = 0)
    {
        std::vector<StmtPtr> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_stmt(depth + 1));
        return vec;
    }

    inline std::vector<ExprPtr> dummy_expr_vec(int count = 2, int depth = 0)
    {
        std::vector<ExprPtr> vec;
        vec.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) vec.push_back(dummy_expr(depth + 1));
        return vec;
    }

    inline ExprPtr dummy_expr(int depth)
    {
        if (depth >= 2)
        {
            auto lit = std::make_unique<NumberLiteral>(std::to_string(DummyAstGeneratorState::next_id()));
            lit->span = dummy_span(depth);
            return lit;
        }
        if (depth == 1)
        {
            auto id_acc = std::make_unique<IdentifierAccess>(dummy_name("id_leaf", depth));
            id_acc->span = dummy_span(depth);
            return id_acc;
        }
        auto bin = std::make_unique<BinaryExpression>(dummy_expr(depth + 1), TokenType::Plus, dummy_expr(depth + 2));
        bin->span = dummy_span(depth);
        return bin;
    }

    inline StmtPtr dummy_stmt(int depth)
    {
        if (depth >= 2)
        {
            auto ret = std::make_unique<ReturnStatement>(dummy_modifiers(1, depth + 1), dummy_expr_vec(2, depth + 1));
            ret->span = dummy_span(depth);
            return ret;
        }
        auto expr_stmt = std::make_unique<ExpressionStatement>(dummy_expr(depth + 1));
        expr_stmt->span = dummy_span(depth);
        return expr_stmt;
    }

    inline TypeAnnPtr dummy_type(int depth)
    {
        if (depth >= 3)
        {
            auto t = std::make_unique<TypeAnnotation>(dummy_name("LeafType", depth));
            t->span = dummy_span(depth);
            return t;
        }
        if (depth == 2)
        {
            std::vector<TypeAnnPtr> elems;
            elems.push_back(std::make_unique<TypeAnnotation>(dummy_name("TupleElemA", depth + 1)));
            elems.back()->span = dummy_span(depth + 1);
            elems.push_back(std::make_unique<TypeAnnotation>(dummy_name("TupleElemB", depth + 1)));
            elems.back()->span = dummy_span(depth + 1);
            auto tuple_t = std::make_unique<TupleTypeAnnotation>(std::move(elems));
            tuple_t->span = dummy_span(depth);
            return tuple_t;
        }
        if (depth == 1)
        {
            std::vector<TypeAnnPtr> args;
            args.push_back(dummy_type(depth + 1));
            args.push_back(dummy_type(depth + 2));
            auto gen_t = std::make_unique<TypeAnnotation>(dummy_name("GenericType", depth), std::move(args));
            gen_t->span = dummy_span(depth);
            return gen_t;
        }
        std::vector<TypeAnnPtr> outer_elems;
        outer_elems.push_back(dummy_type(1));
        outer_elems.push_back(dummy_type(2));
        auto nested_tuple = std::make_unique<TupleTypeAnnotation>(std::move(outer_elems));
        nested_tuple->span = dummy_span(depth);
        return nested_tuple;
    }

    inline CallArgument dummy_call_arg(int depth)
    {
        return CallArgument(dummy_name("arg", depth), dummy_expr(depth + 1), dummy_span(depth));
    }

    inline Modifier dummy_modifier(int depth)
    {
        return Modifier(dummy_name("mod", depth), depth < 2 ? dummy_call_args(2, depth + 1) : std::vector<CallArgument>{}, dummy_span(depth));
    }

    inline FunctionParameter dummy_param(int depth)
    {
        return FunctionParameter(dummy_modifiers(2, depth + 1), dummy_name("param", depth), dummy_type(depth + 1), dummy_expr(depth + 1), dummy_span(depth));
    }

    inline StructField dummy_field(int depth)
    {
        return StructField(dummy_modifiers(2, depth + 1), dummy_name("field", depth), dummy_type(depth + 1), dummy_span(depth));
    }

    inline EnumCase dummy_enum_case(int depth)
    {
        return EnumCase(dummy_modifiers(2, depth + 1), dummy_name("Case", depth), dummy_expr(depth + 1), dummy_span(depth));
    }

    inline DictItem dummy_dict_item(int depth)
    {
        return DictItem(dummy_modifiers(2, depth + 1), dummy_name("key", depth), dummy_expr(depth + 1), dummy_span(depth));
    }

    inline SwitchCase dummy_switch_case(int depth)
    {
        std::vector<NodeName> ids{dummy_name("CaseIdA", depth), dummy_name("CaseIdB", depth)};
        return SwitchCase(dummy_modifiers(2, depth + 1), std::move(ids), dummy_expr(depth + 1), dummy_span(depth));
    }

    inline AssignmentTarget dummy_assignment_target(int depth)
    {
        return AssignmentTarget(dummy_modifiers(2, depth + 1), dummy_name("target", depth), dummy_type(depth + 1), dummy_span(depth));
    }

    template <typename T>
    struct DummyNodeGenerator;

    template <typename T>
    inline auto create_dummy(int depth = 0)
    {
        return DummyNodeGenerator<T>::create(depth);
    }

    template <>
    struct DummyNodeGenerator<Comment>
    {
        static Comment create(int depth = 0)
        {
            return Comment("comment_" + std::to_string(DummyAstGeneratorState::next_id()), dummy_span(depth));
        }
    };

    template <>
    struct DummyNodeGenerator<CallArgument>
    {
        static CallArgument create(int depth = 0)
        {
            return dummy_call_arg(depth);
        }
    };

    template <>
    struct DummyNodeGenerator<Modifier>
    {
        static Modifier create(int depth = 0)
        {
            return dummy_modifier(depth);
        }
    };

    template <>
    struct DummyNodeGenerator<FunctionParameter>
    {
        static FunctionParameter create(int depth = 0)
        {
            return dummy_param(depth);
        }
    };

    template <>
    struct DummyNodeGenerator<StructField>
    {
        static StructField create(int depth = 0)
        {
            return dummy_field(depth);
        }
    };

    template <>
    struct DummyNodeGenerator<EnumCase>
    {
        static EnumCase create(int depth = 0)
        {
            return dummy_enum_case(depth);
        }
    };

    template <>
    struct DummyNodeGenerator<DictItem>
    {
        static DictItem create(int depth = 0)
        {
            return dummy_dict_item(depth);
        }
    };

    template <>
    struct DummyNodeGenerator<SwitchCase>
    {
        static SwitchCase create(int depth = 0)
        {
            return dummy_switch_case(depth);
        }
    };

    template <>
    struct DummyNodeGenerator<AssignmentTarget>
    {
        static AssignmentTarget create(int depth = 0)
        {
            return dummy_assignment_target(depth);
        }
    };

    template <>
    struct DummyNodeGenerator<NumberLiteral>
    {
        static std::unique_ptr<NumberLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<NumberLiteral>(std::to_string(DummyAstGeneratorState::next_id()) + ".75");
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<PercentageLiteral>
    {
        static std::unique_ptr<PercentageLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<PercentageLiteral>(std::to_string(DummyAstGeneratorState::next_id() % 100) + ".5%");
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<StringLiteral>
    {
        static std::unique_ptr<StringLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<StringLiteral>("dummy_string_" + std::to_string(DummyAstGeneratorState::next_id()));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<BooleanLiteral>
    {
        static std::unique_ptr<BooleanLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<BooleanLiteral>(DummyAstGeneratorState::next_id() % 2 == 0);
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<IdentifierAccess>
    {
        static std::unique_ptr<IdentifierAccess> create(int depth = 0)
        {
            auto node = std::make_unique<IdentifierAccess>(dummy_name("identifier", depth));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<SelfExpression>
    {
        static std::unique_ptr<SelfExpression> create(int depth = 0)
        {
            auto node = std::make_unique<SelfExpression>();
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<BinaryExpression>
    {
        static std::unique_ptr<BinaryExpression> create(int depth = 0)
        {
            auto node = std::make_unique<BinaryExpression>(dummy_expr(depth + 1), TokenType::Plus, dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<UnaryExpression>
    {
        static std::unique_ptr<UnaryExpression> create(int depth = 0)
        {
            auto node = std::make_unique<UnaryExpression>(TokenType::Minus, dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<GroupingExpression>
    {
        static std::unique_ptr<GroupingExpression> create(int depth = 0)
        {
            auto node = std::make_unique<GroupingExpression>(dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<ConditionalExpression>
    {
        static std::unique_ptr<ConditionalExpression> create(int depth = 0)
        {
            auto node = std::make_unique<ConditionalExpression>(dummy_expr(depth + 1), dummy_expr(depth + 1), dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<FunctionCall>
    {
        static std::unique_ptr<FunctionCall> create(int depth = 0)
        {
            auto node = std::make_unique<FunctionCall>(dummy_expr(depth + 1), dummy_call_args(3, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<DictLiteral>
    {
        static std::unique_ptr<DictLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<DictLiteral>(dummy_dict_items(3, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<TensorLiteral>
    {
        static std::unique_ptr<TensorLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<TensorLiteral>(dummy_expr_vec(3, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<TupleLiteral>
    {
        static std::unique_ptr<TupleLiteral> create(int depth = 0)
        {
            auto node = std::make_unique<TupleLiteral>(dummy_expr_vec(3, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<BracketAccess>
    {
        static std::unique_ptr<BracketAccess> create(int depth = 0)
        {
            auto node = std::make_unique<BracketAccess>(dummy_expr(depth + 1), dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<DotAccess>
    {
        static std::unique_ptr<DotAccess> create(int depth = 0)
        {
            auto node = std::make_unique<DotAccess>(dummy_expr(depth + 1), dummy_name("prop", depth));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<SwitchExpression>
    {
        static std::unique_ptr<SwitchExpression> create(int depth = 0)
        {
            auto node = std::make_unique<SwitchExpression>(
                dummy_expr(depth + 1),
                dummy_switch_cases(3, depth + 1),
                dummy_modifiers(2, depth + 1),
                dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<Assignment>
    {
        static std::unique_ptr<Assignment> create(int depth = 0)
        {
            auto node = std::make_unique<Assignment>(dummy_assignment_targets(3, depth + 1), dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<Reassignment>
    {
        static std::unique_ptr<Reassignment> create(int depth = 0)
        {
            auto node = std::make_unique<Reassignment>(dummy_expr(depth + 1), dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<ExpressionStatement>
    {
        static std::unique_ptr<ExpressionStatement> create(int depth = 0)
        {
            auto node = std::make_unique<ExpressionStatement>(dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<ReturnStatement>
    {
        static std::unique_ptr<ReturnStatement> create(int depth = 0)
        {
            auto node = std::make_unique<ReturnStatement>(dummy_modifiers(2, depth + 1), dummy_expr_vec(3, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<EnumDefinition>
    {
        static std::unique_ptr<EnumDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<EnumDefinition>(
                dummy_modifiers(2, depth + 1),
                dummy_name("EnumDef", depth),
                dummy_type(depth + 1),
                dummy_enum_cases(3, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<Directive>
    {
        static std::unique_ptr<Directive> create(int depth = 0)
        {
            auto node = std::make_unique<Directive>(dummy_name("directive_name", depth), dummy_expr(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<ImportStatement>
    {
        static std::unique_ptr<ImportStatement> create(int depth = 0)
        {
            auto node = std::make_unique<ImportStatement>(dummy_modifiers(2, depth + 1), dummy_name("import.module.path", depth));
            node->resolved_canonical_path = "/resolved/path/module_" + std::to_string(DummyAstGeneratorState::next_id()) + ".vs";
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<FunctionDefinition>
    {
        static std::unique_ptr<FunctionDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<FunctionDefinition>(
                dummy_modifiers(2, depth + 1),
                dummy_name("func_name", depth),
                dummy_params(3, depth + 1),
                dummy_type_vec(2, depth + 1),
                dummy_stmt_vec(3, depth + 1),
                std::make_optional<std::string>("Docstring for dummy function"));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<StructDefinition>
    {
        static std::unique_ptr<StructDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<StructDefinition>(
                dummy_modifiers(2, depth + 1),
                dummy_name("StructName", depth),
                dummy_fields(3, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<TypeAliasDefinition>
    {
        static std::unique_ptr<TypeAliasDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<TypeAliasDefinition>(
                dummy_modifiers(2, depth + 1),
                dummy_name("AliasName", depth),
                dummy_type(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<ExtensionDefinition>
    {
        static std::unique_ptr<ExtensionDefinition> create(int depth = 0)
        {
            auto node = std::make_unique<ExtensionDefinition>(
                dummy_modifiers(2, depth + 1),
                dummy_type(depth + 1));
            node->execution_steps = dummy_stmt_vec(2, depth + 1);
            node->function_definitions.push_back(create_dummy<FunctionDefinition>(depth + 1));
            node->struct_definitions.push_back(create_dummy<StructDefinition>(depth + 1));
            node->enum_definitions.push_back(create_dummy<EnumDefinition>(depth + 1));
            node->type_aliases.push_back(create_dummy<TypeAliasDefinition>(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<Program>
    {
        static std::unique_ptr<Program> create(int depth = 0)
        {
            auto node = std::make_unique<Program>();
            node->comments.push_back(create_dummy<Comment>(depth + 1));
            node->comments.push_back(create_dummy<Comment>(depth + 1));
            node->import_statements.push_back(create_dummy<ImportStatement>(depth + 1));
            node->directives.push_back(create_dummy<Directive>(depth + 1));
            node->execution_steps = dummy_stmt_vec(2, depth + 1);
            node->function_definitions.push_back(create_dummy<FunctionDefinition>(depth + 1));
            node->struct_definitions.push_back(create_dummy<StructDefinition>(depth + 1));
            node->enum_definitions.push_back(create_dummy<EnumDefinition>(depth + 1));
            node->type_aliases.push_back(create_dummy<TypeAliasDefinition>(depth + 1));
            node->extension_definitions.push_back(create_dummy<ExtensionDefinition>(depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<TypeAnnotation>
    {
        static std::unique_ptr<TypeAnnotation> create(int depth = 0)
        {
            auto node = std::make_unique<TypeAnnotation>(dummy_name("TypeAnn", depth), dummy_type_vec(2, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };

    template <>
    struct DummyNodeGenerator<TupleTypeAnnotation>
    {
        static std::unique_ptr<TupleTypeAnnotation> create(int depth = 0)
        {
            auto node = std::make_unique<TupleTypeAnnotation>(dummy_type_vec(3, depth + 1));
            node->span = dummy_span(depth);
            return node;
        }
    };
}
