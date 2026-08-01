#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <memory>
#include "token/token.h"
#include "ast_arena.h"

using namespace valuascript::shared;

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

    struct SourceSpan
    {
        size_t line_start = 0;
        size_t column_start = 0;
        size_t line_end = 0;
        size_t column_end = 0;
        std::shared_ptr<const std::string> file_path;

        SourceSpan() = default;

        SourceSpan(size_t ls, size_t cs, size_t le, size_t ce, std::shared_ptr<const std::string> fp)
            : line_start(ls), column_start(cs), line_end(le), column_end(ce), file_path(std::move(fp)) {}

        SourceSpan(size_t ls, size_t cs, size_t le, size_t ce, std::string fp)
            : line_start(ls), column_start(cs), line_end(le), column_end(ce),
              file_path(std::make_shared<const std::string>(std::move(fp))) {}

        SourceSpan(size_t ls, size_t cs, size_t le, size_t ce, const char* fp)
            : line_start(ls), column_start(cs), line_end(le), column_end(ce),
              file_path(std::make_shared<const std::string>(fp ? fp : "")) {}

        SourceSpan& operator=(std::string fp)
        {
            file_path = std::make_shared<const std::string>(std::move(fp));
            return *this;
        }

        SourceSpan& operator=(const char* fp)
        {
            file_path = std::make_shared<const std::string>(fp ? fp : "");
            return *this;
        }

        [[nodiscard]] std::string_view path() const noexcept
        {
            return file_path ? *file_path : std::string_view{};
        }

        [[nodiscard]] bool operator==(const SourceSpan& other) const
        {
            return line_start == other.line_start && column_start == other.column_start &&
                   line_end == other.line_end && column_end == other.column_end &&
                   (file_path == other.file_path || (file_path && other.file_path && *file_path == *other.file_path));
        }
    };

    enum class AstKind : uint8_t
    {
        Unknown,
        NumberLiteral,
        PercentageLiteral,
        StringLiteral,
        BooleanLiteral,
        IdentifierAccess,
        SelfExpression,
        BinaryExpression,
        UnaryExpression,
        GroupingExpression,
        ConditionalExpression,
        FunctionCall,
        DictLiteral,
        TensorLiteral,
        TupleLiteral,
        BracketAccess,
        DotAccess,
        SwitchExpression,
        Assignment,
        Reassignment,
        ExpressionStatement,
        ReturnStatement,
        EnumDefinition,
        Directive,
        ImportStatement,
        FunctionDefinition,
        StructDefinition,
        TypeAliasDefinition,
        ExtensionDefinition,
        Program,
        TypeAnnotation,
        TupleTypeAnnotation
    };

    class AstNode
    {
    public:
        SourceSpan span;
        AstKind kind = AstKind::Unknown;

        explicit AstNode(AstKind k = AstKind::Unknown) : kind(k) {}
        virtual ~AstNode() = default;

        static void* operator new(size_t size);
        static void operator delete(void* ptr, size_t size) noexcept;
        static void operator delete(void* ptr) noexcept;
    };

    class Expression : public AstNode
    {
    public:
        explicit Expression(AstKind k = AstKind::Unknown) : AstNode(k) {}
        [[nodiscard]] virtual bool is_complete() const { return true; }
    };

    class Statement : public AstNode
    {
    public:
        explicit Statement(AstKind k = AstKind::Unknown) : AstNode(k) {}
    };

    struct Modifier
    {
        std::string name;
        std::vector<std::pair<std::string, ExprPtr>> arguments;
        SourceSpan span;
    };

    template <typename T>
    concept AstNodeSubclass = std::derived_from<std::decay_t<T>, AstNode>;

    template <typename T>
        requires AstNodeSubclass<T>
    [[nodiscard]] inline T* ast_cast(AstNode* node) noexcept
    {
        if (!node) return nullptr;
        if constexpr (requires { T::KIND; })
        {
            if (node->kind == T::KIND) return static_cast<T*>(node);
            return nullptr;
        }
        else
        {
            return dynamic_cast<T*>(node);
        }
    }

    template <typename T>
        requires AstNodeSubclass<T>
    [[nodiscard]] inline const T* ast_cast(const AstNode* node) noexcept
    {
        if (!node) return nullptr;
        if constexpr (requires { T::KIND; })
        {
            if (node->kind == T::KIND) return static_cast<const T*>(node);
            return nullptr;
        }
        else
        {
            return dynamic_cast<const T*>(node);
        }
    }
}
