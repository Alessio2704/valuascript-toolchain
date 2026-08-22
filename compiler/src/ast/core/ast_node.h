#pragma once

#include <memory>
#include <vector>
#include <cstddef>

#include "token/source_span.h"
#include "ast_kind.h"

namespace valuascript::compiler
{
    class Expression;
    class Statement;
    class TypeAnnotation;
    class ImportStatement;
    class Directive;
    class FunctionDefinition;
    class StructDefinition;
    class EnumDefinition;
    class TypeAliasDefinition;
    class ExtensionDefinition;

    using ExprPtr = std::unique_ptr<Expression>;
    using StmtPtr = std::unique_ptr<Statement>;
    using TypeAnnPtr = std::unique_ptr<TypeAnnotation>;

    using ImportPtr = std::unique_ptr<ImportStatement>;
    using DirectivePtr = std::unique_ptr<Directive>;
    using FuncDefPtr = std::unique_ptr<FunctionDefinition>;
    using StructDefPtr = std::unique_ptr<StructDefinition>;
    using EnumDefPtr = std::unique_ptr<EnumDefinition>;
    using TypeAliasPtr = std::unique_ptr<TypeAliasDefinition>;
    using ExtensionDefPtr = std::unique_ptr<ExtensionDefinition>;

    class AstNode
    {
    public:
        valuascript::shared::SourceSpan span;
        AstKind kind = AstKind::Unknown;

        explicit AstNode(AstKind k = AstKind::Unknown) : kind(k)
        {
        }

        virtual ~AstNode() = default;

        [[nodiscard]] virtual bool is_valid() const noexcept
        {
            return span.is_valid();
        }

        static void* operator new(size_t size);
        static void operator delete(void* ptr, size_t size) noexcept;
        static void operator delete(void* ptr) noexcept;
    };

    class Expression : public AstNode
    {
    public:
        explicit Expression(AstKind k = AstKind::Unknown) : AstNode(k)
        {
        }
    };

    class Statement : public AstNode
    {
    public:
        explicit Statement(AstKind k = AstKind::Unknown) : AstNode(k)
        {
        }
    };
}
