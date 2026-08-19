#pragma once

#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <memory>
#include <ostream>
#include <type_traits>
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

    struct NodeName
    {
        std::string value;
        SourceSpan span = {};

        NodeName() = default;
        NodeName(std::string_view val, SourceSpan sp = {}) : value(val), span(std::move(sp)) {}

        [[nodiscard]] const std::string& str() const noexcept { return value; }
        [[nodiscard]] const char* c_str() const noexcept { return value.c_str(); }
        [[nodiscard]] bool empty() const noexcept { return value.empty(); }
        [[nodiscard]] size_t length() const noexcept { return value.length(); }

        operator std::string_view() const noexcept { return value; }
        operator const std::string&() const noexcept { return value; }
        bool operator==(const NodeName& other) const = default;
        bool operator==(std::string_view other) const noexcept { return value == other; }
        bool operator==(const char* other) const noexcept { return value == other; }
        bool operator==(const std::string& other) const noexcept { return value == other; }

        friend std::string operator+(std::string_view lhs, const NodeName& rhs)
        {
            std::string result(lhs);
            result += rhs.value;
            return result;
        }
        friend std::string operator+(const NodeName& lhs, std::string_view rhs)
        {
            std::string result(lhs.value);
            result += rhs;
            return result;
        }
        friend std::string operator+(const std::string& lhs, const NodeName& rhs)
        {
            return lhs + rhs.value;
        }
        friend std::string operator+(const NodeName& lhs, const std::string& rhs)
        {
            return lhs.value + rhs;
        }
        friend std::string operator+(const char* lhs, const NodeName& rhs)
        {
            return std::string(lhs) + rhs.value;
        }
        friend std::string operator+(const NodeName& lhs, const char* rhs)
        {
            return lhs.value + rhs;
        }
        friend std::ostream& operator<<(std::ostream& os, const NodeName& name)
        {
            return os << name.value;
        }
    };

    struct CallArgument
    {
        NodeName name;
        ExprPtr value;
        SourceSpan span = {};
    };

    struct Modifier
    {
        NodeName name;
        std::vector<CallArgument> arguments;
        SourceSpan span = {};
    };

    template <typename T>
    concept AstNodeSubclass = std::derived_from<std::decay_t<T>, AstNode>;

    template <typename T>
        requires AstNodeSubclass<T>
    [[nodiscard]] inline T* ast_cast(AstNode* node) noexcept
    {
        if (!node) [[unlikely]] return nullptr;
        if constexpr (requires { T::KIND; })
        {
            if (node->kind == T::KIND) [[likely]] return static_cast<T*>(node);
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
        if (!node) [[unlikely]] return nullptr;
        if constexpr (requires { T::KIND; })
        {
            if (node->kind == T::KIND) [[likely]] return static_cast<const T*>(node);
            return nullptr;
        }
        else
        {
            return dynamic_cast<const T*>(node);
        }
    }
}
