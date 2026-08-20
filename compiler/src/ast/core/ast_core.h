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
        TupleTypeAnnotation,
        FunctionParameter,
        StructField,
        EnumCase,
        SwitchCase,
        AssignmentTarget,
        Modifier,
        CallArgument,
        DictItem,
        Comment
    };

    class AstNode
    {
    public:
        SourceSpan span;
        AstKind kind = AstKind::Unknown;

        explicit AstNode(AstKind k = AstKind::Unknown) : kind(k)
        {
        }

        virtual ~AstNode() = default;

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

        [[nodiscard]] virtual bool is_complete() const { return true; }
    };

    class Statement : public AstNode
    {
    public:
        explicit Statement(AstKind k = AstKind::Unknown) : AstNode(k)
        {
        }
    };

    struct NodeName
    {
        std::string value;
        SourceSpan span = {};

        NodeName() = default;

        NodeName(std::string_view val, SourceSpan sp = {}) : value(val), span(std::move(sp))
        {
        }

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

    class Comment : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::Comment;
        std::string text = {};

        Comment() : AstNode(KIND)
        {
        }

        Comment(std::string txt, SourceSpan sp = {})
            : AstNode(KIND), text(std::move(txt))
        {
            span = sp;
        }

        explicit Comment(const valuascript::shared::CommentToken& tok)
            : AstNode(KIND), text(tok.text)
        {
            span = tok.span;
        }
    };

    class CallArgument : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::CallArgument;
        NodeName name;
        ExprPtr value = nullptr;

        CallArgument() : AstNode(KIND)
        {
        }

        CallArgument(NodeName n, ExprPtr val = nullptr, SourceSpan sp = {})
            : AstNode(KIND), name(std::move(n)), value(std::move(val))
        {
            span = sp;
        }
    };

    class Modifier : public AstNode
    {
    public:
        static constexpr AstKind KIND = AstKind::Modifier;
        NodeName name;
        std::vector<CallArgument> arguments;

        Modifier() : AstNode(KIND)
        {
        }

        Modifier(NodeName n, std::vector<CallArgument> args = {}, SourceSpan sp = {})
            : AstNode(KIND), name(std::move(n)), arguments(std::move(args))
        {
            span = sp;
        }
    };

    template <typename T>
    concept AstElement = std::derived_from<std::decay_t<T>, AstNode>;

    template <typename T>
    concept ExpressionNode = std::derived_from<std::decay_t<T>, Expression>;

    template <typename T>
    concept StatementNode = std::derived_from<std::decay_t<T>, Statement>;

    template <typename T>
    concept TypeAnnNode = std::derived_from<std::decay_t<T>, TypeAnnotation>;

    template <typename T>
    concept ConcreteAstNode = AstElement<T> && requires { { T::KIND } -> std::same_as<const AstKind&>; };

    template <AstElement T, typename NodeT>
        requires std::derived_from<std::decay_t<NodeT>, AstNode>
    [[nodiscard]] inline auto* ast_cast(NodeT* node) noexcept
    {
        using ReturnType = std::conditional_t<std::is_const_v<NodeT>, const T, T>;
        if (!node) [[unlikely]] return static_cast<ReturnType*>(nullptr);
        if constexpr (requires { T::KIND; })
        {
            if (node->kind == T::KIND) [[likely]] return static_cast<ReturnType*>(node);
            return static_cast<ReturnType*>(nullptr);
        }
        else
        {
            return dynamic_cast<ReturnType*>(node);
        }
    }

    template <AstElement T, typename NodeT>
        requires std::derived_from<std::decay_t<NodeT>, AstNode>
    [[nodiscard]] inline bool is_a(NodeT* node) noexcept
    {
        return ast_cast<T>(node) != nullptr;
    }

    template <AstElement T, typename NodeT>
        requires std::derived_from<std::decay_t<NodeT>, AstNode>
    [[nodiscard]] inline auto& as(NodeT& node) noexcept
    {
        auto* casted = ast_cast<T>(&node);
        assert(casted != nullptr && "Invalid AST node cast with as<T>()");
        return *casted;
    }

    template <AstElement T, AstElement Base>
    [[nodiscard]] inline std::unique_ptr<T> ast_cast_unique(std::unique_ptr<Base> ptr) noexcept
    {
        if (!ptr) return nullptr;
        if (ast_cast<T>(ptr.get()))
        {
            return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
        }
        return nullptr;
    }

    template <typename T>
    [[nodiscard]] constexpr std::string_view get_node_name(const T& elem) noexcept
    {
        if constexpr (requires { elem.name.value; }) return elem.name.value;
        else if constexpr (requires { elem.property_name.value; }) return elem.property_name.value;
        else if constexpr (requires { elem.key.value; }) return elem.key.value;
        else if constexpr (requires { elem.path.value; }) return elem.path.value;
        else if constexpr (requires { elem.name; }) return elem.name;
        else return "";
    }

    template <typename T>
    [[nodiscard]] constexpr const SourceSpan* get_node_name_span(const T& elem) noexcept
    {
        if constexpr (requires { elem.name.span; }) return &elem.name.span;
        else if constexpr (requires { elem.path.span; }) return &elem.path.span;
        else if constexpr (requires { elem.property_name.span; }) return &elem.property_name.span;
        else return nullptr;
    }
}
