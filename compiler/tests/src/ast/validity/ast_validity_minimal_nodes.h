#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <utility>
#include <type_traits>
#include <concepts>

#include "token/source_span.h"
#include "token/token_type.h"
#include "ast/core/ast_core.h"
#include "ast/core/ast_type.h"
#include "ast/core/ast_expr.h"
#include "ast/core/ast_stmt.h"
#include "ast/core/ast_decl.h"
#include "ast/utils/ast_builder.h"
#include "ast/metadata/ast_node_registry.h"
#include "ast/categories/ast_inner_types.h"
#include "utils/traits/tuple_traits.h"

namespace valuascript::compiler::test
{
    inline SourceSpan make_valid_test_span()
    {
        return SourceSpan{
            .line_start = 1,
            .column_start = 1,
            .line_end = 1,
            .column_end = 10,
            .file_path = std::make_shared<const std::string>("validity_test.vs"),
            .start_offset = 0,
            .length = 10
        };
    }

    inline NodeName make_valid_test_name(std::string_view name = "valid_ident")
    {
        return NodeName(std::string(name), make_valid_test_span());
    }

    template <typename T>
    struct MinimalSampleProvider;

    template <>
    struct MinimalSampleProvider<Program>
    {
        static std::unique_ptr<Program> make_valid()
        {
            return AstBuilder::build_with_span<Program>(make_valid_test_span());
        }
    };

    template <>
    struct MinimalSampleProvider<ImportStatement>
    {
        static std::unique_ptr<ImportStatement> make_valid()
        {
            return AstBuilder::build_with_span<ImportStatement>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                make_valid_test_name("module")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<Directive>
    {
        static std::unique_ptr<Directive> make_valid()
        {
            return AstBuilder::build_with_span<Directive>(
                make_valid_test_span(),
                make_valid_test_name("directive"),
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "1")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<FunctionDefinition>
    {
        static std::unique_ptr<FunctionDefinition> make_valid()
        {
            return AstBuilder::build_with_span<FunctionDefinition>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                make_valid_test_name("foo"),
                std::vector<FunctionParameter>{},
                std::vector<TypeAnnPtr>{},
                std::vector<StmtPtr>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<StructDefinition>
    {
        static std::unique_ptr<StructDefinition> make_valid()
        {
            return AstBuilder::build_with_span<StructDefinition>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                make_valid_test_name("MyStruct"),
                std::vector<StructField>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<EnumDefinition>
    {
        static std::unique_ptr<EnumDefinition> make_valid()
        {
            return AstBuilder::build_with_span<EnumDefinition>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                make_valid_test_name("MyEnum"),
                nullptr,
                std::vector<EnumCase>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<TypeAliasDefinition>
    {
        static std::unique_ptr<TypeAliasDefinition> make_valid()
        {
            return AstBuilder::build_with_span<TypeAliasDefinition>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                make_valid_test_name("MyAlias"),
                AstBuilder::build_with_span<TypeAnnotation>(make_valid_test_span(), make_valid_test_name("Target"))
            );
        }
    };

    template <>
    struct MinimalSampleProvider<ExtensionDefinition>
    {
        static std::unique_ptr<ExtensionDefinition> make_valid()
        {
            return AstBuilder::build_with_span<ExtensionDefinition>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                AstBuilder::build_with_span<TypeAnnotation>(make_valid_test_span(), make_valid_test_name("ExtType"))
            );
        }
    };

    template <>
    struct MinimalSampleProvider<Assignment>
    {
        static std::unique_ptr<Assignment> make_valid()
        {
            return AstBuilder::build_with_span<Assignment>(
                make_valid_test_span(),
                std::vector<AssignmentTarget>{},
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "0")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<Reassignment>
    {
        static std::unique_ptr<Reassignment> make_valid()
        {
            return AstBuilder::build_with_span<Reassignment>(
                make_valid_test_span(),
                AstBuilder::build_with_span<IdentifierAccess>(make_valid_test_span(), make_valid_test_name("x")),
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "0")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<ExpressionStatement>
    {
        static std::unique_ptr<ExpressionStatement> make_valid()
        {
            return AstBuilder::build_with_span<ExpressionStatement>(
                make_valid_test_span(),
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "0")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<ReturnStatement>
    {
        static std::unique_ptr<ReturnStatement> make_valid()
        {
            return AstBuilder::build_with_span<ReturnStatement>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                std::vector<ExprPtr>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<NumberLiteral>
    {
        static std::unique_ptr<NumberLiteral> make_valid()
        {
            return AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "42");
        }
    };

    template <>
    struct MinimalSampleProvider<PercentageLiteral>
    {
        static std::unique_ptr<PercentageLiteral> make_valid()
        {
            return AstBuilder::build_with_span<PercentageLiteral>(make_valid_test_span(), "50%");
        }
    };

    template <>
    struct MinimalSampleProvider<StringLiteral>
    {
        static std::unique_ptr<StringLiteral> make_valid()
        {
            return AstBuilder::build_with_span<StringLiteral>(make_valid_test_span(), "hello");
        }
    };

    template <>
    struct MinimalSampleProvider<BooleanLiteral>
    {
        static std::unique_ptr<BooleanLiteral> make_valid()
        {
            return AstBuilder::build_with_span<BooleanLiteral>(make_valid_test_span(), true);
        }
    };

    template <>
    struct MinimalSampleProvider<IdentifierAccess>
    {
        static std::unique_ptr<IdentifierAccess> make_valid()
        {
            return AstBuilder::build_with_span<IdentifierAccess>(make_valid_test_span(), make_valid_test_name("ident"));
        }
    };

    template <>
    struct MinimalSampleProvider<SelfExpression>
    {
        static std::unique_ptr<SelfExpression> make_valid()
        {
            return AstBuilder::build_with_span<SelfExpression>(make_valid_test_span());
        }
    };

    template <>
    struct MinimalSampleProvider<BinaryExpression>
    {
        static std::unique_ptr<BinaryExpression> make_valid()
        {
            return AstBuilder::build_with_span<BinaryExpression>(
                make_valid_test_span(),
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "1"),
                TokenType::Plus,
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "2")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<UnaryExpression>
    {
        static std::unique_ptr<UnaryExpression> make_valid()
        {
            return AstBuilder::build_with_span<UnaryExpression>(
                make_valid_test_span(),
                TokenType::Minus,
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "1")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<GroupingExpression>
    {
        static std::unique_ptr<GroupingExpression> make_valid()
        {
            return AstBuilder::build_with_span<GroupingExpression>(
                make_valid_test_span(),
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "1")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<ConditionalExpression>
    {
        static std::unique_ptr<ConditionalExpression> make_valid()
        {
            return AstBuilder::build_with_span<ConditionalExpression>(
                make_valid_test_span(),
                AstBuilder::build_with_span<BooleanLiteral>(make_valid_test_span(), true),
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "1"),
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "2")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<FunctionCall>
    {
        static std::unique_ptr<FunctionCall> make_valid()
        {
            return AstBuilder::build_with_span<FunctionCall>(
                make_valid_test_span(),
                AstBuilder::build_with_span<IdentifierAccess>(make_valid_test_span(), make_valid_test_name("fn")),
                std::vector<CallArgument>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<DictLiteral>
    {
        static std::unique_ptr<DictLiteral> make_valid()
        {
            return AstBuilder::build_with_span<DictLiteral>(
                make_valid_test_span(),
                std::vector<DictItem>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<TensorLiteral>
    {
        static std::unique_ptr<TensorLiteral> make_valid()
        {
            return AstBuilder::build_with_span<TensorLiteral>(
                make_valid_test_span(),
                std::vector<ExprPtr>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<TupleLiteral>
    {
        static std::unique_ptr<TupleLiteral> make_valid()
        {
            return AstBuilder::build_with_span<TupleLiteral>(
                make_valid_test_span(),
                std::vector<ExprPtr>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<BracketAccess>
    {
        static std::unique_ptr<BracketAccess> make_valid()
        {
            return AstBuilder::build_with_span<BracketAccess>(
                make_valid_test_span(),
                AstBuilder::build_with_span<IdentifierAccess>(make_valid_test_span(), make_valid_test_name("arr")),
                AstBuilder::build_with_span<NumberLiteral>(make_valid_test_span(), "0")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<DotAccess>
    {
        static std::unique_ptr<DotAccess> make_valid()
        {
            return AstBuilder::build_with_span<DotAccess>(
                make_valid_test_span(),
                AstBuilder::build_with_span<IdentifierAccess>(make_valid_test_span(), make_valid_test_name("obj")),
                make_valid_test_name("field")
            );
        }
    };

    template <>
    struct MinimalSampleProvider<SwitchExpression>
    {
        static std::unique_ptr<SwitchExpression> make_valid()
        {
            return AstBuilder::build_with_span<SwitchExpression>(
                make_valid_test_span(),
                AstBuilder::build_with_span<IdentifierAccess>(make_valid_test_span(), make_valid_test_name("s")),
                std::vector<SwitchCase>{},
                std::vector<Modifier>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<TypeAnnotation>
    {
        static std::unique_ptr<TypeAnnotation> make_valid()
        {
            return AstBuilder::build_with_span<TypeAnnotation>(make_valid_test_span(), make_valid_test_name("i32"));
        }
    };

    template <>
    struct MinimalSampleProvider<TupleTypeAnnotation>
    {
        static std::unique_ptr<TupleTypeAnnotation> make_valid()
        {
            return AstBuilder::build_with_span<TupleTypeAnnotation>(
                make_valid_test_span(),
                std::vector<TypeAnnPtr>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<FunctionParameter>
    {
        static FunctionParameter make_valid()
        {
            FunctionParameter p{};
            p.span = make_valid_test_span();
            p.name = make_valid_test_name("param");
            return p;
        }
    };

    template <>
    struct MinimalSampleProvider<StructField>
    {
        static StructField make_valid()
        {
            StructField f{};
            f.span = make_valid_test_span();
            f.name = make_valid_test_name("field");
            return f;
        }
    };

    template <>
    struct MinimalSampleProvider<EnumCase>
    {
        static EnumCase make_valid()
        {
            EnumCase c{};
            c.span = make_valid_test_span();
            c.name = make_valid_test_name("CaseA");
            return c;
        }
    };

    template <>
    struct MinimalSampleProvider<SwitchCase>
    {
        static SwitchCase make_valid()
        {
            return AstBuilder::build_with_span<SwitchCase>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                std::vector<NodeName>{make_valid_test_name("case_a")},
                nullptr
            );
        }
    };

    template <>
    struct MinimalSampleProvider<AssignmentTarget>
    {
        static AssignmentTarget make_valid()
        {
            AssignmentTarget t{};
            t.span = make_valid_test_span();
            t.name = make_valid_test_name("target");
            return t;
        }
    };

    template <>
    struct MinimalSampleProvider<Modifier>
    {
        static Modifier make_valid()
        {
            return AstBuilder::build_with_span<Modifier>(
                make_valid_test_span(),
                make_valid_test_name("pub"),
                std::vector<CallArgument>{}
            );
        }
    };

    template <>
    struct MinimalSampleProvider<CallArgument>
    {
        static CallArgument make_valid()
        {
            return AstBuilder::build_with_span<CallArgument>(
                make_valid_test_span(),
                make_valid_test_name("arg"),
                nullptr
            );
        }
    };

    template <>
    struct MinimalSampleProvider<DictItem>
    {
        static DictItem make_valid()
        {
            return AstBuilder::build_with_span<DictItem>(
                make_valid_test_span(),
                std::vector<Modifier>{},
                make_valid_test_name("k"),
                nullptr
            );
        }
    };

    template <>
    struct MinimalSampleProvider<Comment>
    {
        static Comment make_valid()
        {
            return AstBuilder::build_with_span<Comment>(
                make_valid_test_span(),
                "test comment"
            );
        }
    };

    template <typename T>
    concept HasMinimalSampleProvider = requires {
        { MinimalSampleProvider<T>::make_valid() } -> std::same_as<std::conditional_t<
            valuascript::shared::tuple_contains_type_v<T, AllInnerNodeTypes>,
            T,
            std::unique_ptr<T>
        >>;
    };

    template <typename Tuple>
    struct AllValiditySampleProvidersDefined;

    template <typename... Types>
    struct AllValiditySampleProvidersDefined<std::tuple<Types...>>
    {
        static consteval bool validate()
        {
            return (HasMinimalSampleProvider<Types> && ...);
        }
    };

    static_assert(AllValiditySampleProvidersDefined<AllAstNodeTypes>::validate(),
                  "Every concrete AST node registered in AllAstNodeTypes must define a MinimalSampleProvider!");
}
