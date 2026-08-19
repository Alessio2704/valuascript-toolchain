#pragma once

#include <gtest/gtest.h>
#include <concepts>
#include <type_traits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <utility>
#include <tuple>
#include <variant>
#include <span>
#include <compare>
#include <source_location>
#include "frontend/parser/ast.h"
#include "utils/demangle_name.h"

namespace valuascript::compiler::test
{
    struct StringStorage
    {
        std::variant<std::string_view, std::string> data_;

        constexpr StringStorage() : data_(std::in_place_index<0>, std::string_view{})
        {
        }

        constexpr StringStorage(const char* s) : data_(std::in_place_index<0>, std::string_view(s ? s : ""))
        {
        }

        constexpr StringStorage(std::string_view sv) : data_(std::in_place_index<0>, sv)
        {
        }

        StringStorage(std::string s) : data_(std::move(s))
        {
        }

        [[nodiscard]] operator std::string_view() const { return get(); }

        [[nodiscard]] std::string_view get() const
        {
            if (auto* sv = std::get_if<std::string_view>(&data_)) [[likely]] return *sv;
            return std::get<std::string>(data_);
        }

        [[nodiscard]] const char* data() const { return get().data(); }
        [[nodiscard]] size_t size() const { return get().size(); }
        [[nodiscard]] bool empty() const { return get().empty(); }

        [[nodiscard]] friend bool operator==(const StringStorage& lhs, std::string_view rhs)
        {
            return lhs.get() == rhs;
        }

        [[nodiscard]] friend bool operator==(const StringStorage& lhs, const NodeName& rhs)
        {
            return lhs.get() == rhs.value;
        }

        [[nodiscard]] friend std::strong_ordering operator<=>(const StringStorage& lhs, std::string_view rhs)
        {
            return lhs.get() <=> rhs;
        }
    };

    inline void AssertSpanMatch(const SourceSpan& actual, const SourceSpan& expected,
                                std::source_location loc = std::source_location::current())
    {
        if (actual.line_start != expected.line_start ||
            actual.column_start != expected.column_start ||
            actual.line_end != expected.line_end ||
            actual.column_end != expected.column_end)
        {
            ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                << "Span line/column mismatch:\n"
                << "  Expected: line " << expected.line_start << ":" << expected.column_start
                << " -> line " << expected.line_end << ":" << expected.column_end << "\n"
                << "  Actual:   line " << actual.line_start << ":" << actual.column_start
                << " -> line " << actual.line_end << ":" << actual.column_end;
        }

        if (expected.start_offset != 0 || expected.length != 0)
        {
            if (actual.start_offset != expected.start_offset || actual.length != expected.length)
            {
                ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                    << "Span offset/length mismatch:\n"
                    << "  Expected: offset " << expected.start_offset << ", length " << expected.length
                    << ", end_offset " << expected.end_offset() << "\n"
                    << "  Actual:   offset " << actual.start_offset << ", length " << actual.length
                    << ", end_offset " << actual.end_offset();
            }
        }

        if (!expected.path().empty())
        {
            if (actual.path() != expected.path())
            {
                ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                    << "Span file_path mismatch:\n"
                    << "  Expected: " << expected.path() << "\n"
                    << "  Actual:   " << actual.path();
            }
        }
    }

    struct AnyMatcher;

    template <typename F, typename NodeT>
    concept HasNodeType = requires
    {
        typename std::decay_t<F>::node_type;
    };

    template <typename F, typename NodeT>
    concept IsCompatibleNodeVerifier =
        (HasNodeType<F, NodeT> && std::derived_from<typename std::decay_t<F>::node_type, NodeT>) ||
        (!HasNodeType<F, NodeT> && std::invocable<F, NodeT*>);

    template <typename T>
    concept ASTNodeConcept = std::derived_from<std::decay_t<T>, AstNode>;

    template <typename M, typename NodeT>
    concept ASTMatcher = requires(const std::decay_t<M>& m, NodeT* node)
    {
        { m(node) };
    };

    template <typename M>
    concept ExprMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, Expression>;

    template <typename M>
    concept TypeNodeMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, TypeAnnotation>;

    template <typename M>
    concept StmtMatcher = std::same_as<std::decay_t<M>, AnyMatcher> || IsCompatibleNodeVerifier<M, Statement>;

    template <typename NodeT, size_t BufferSize = 64>
    class InlineVerifier;

    template <typename M>
    struct FluentNodeMatcher
    {
        using node_type = typename std::decay_t<M>::node_type;

        M matcher;
        std::optional<SourceSpan> expected_span;
        std::optional<SourceSpan> expected_name_span;

        constexpr FluentNodeMatcher(M m) : matcher(std::move(m))
        {
        }

        [[nodiscard]] FluentNodeMatcher with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                                  size_t start_offset, size_t length) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_span(const SourceSpan& span) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_span = span;
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end
            };
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_name_span(size_t line_start, size_t col_start, size_t line_end, size_t col_end,
                                                       size_t start_offset, size_t length) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_name_span = SourceSpan{
                .line_start = line_start, .column_start = col_start, .line_end = line_end, .column_end = col_end,
                .start_offset = start_offset, .length = length
            };
            return copy;
        }

        [[nodiscard]] FluentNodeMatcher with_name_span(const SourceSpan& span) const
        {
            FluentNodeMatcher copy = *this;
            copy.expected_name_span = span;
            return copy;
        }

        void operator()(node_type* node) const
        {
            if (!node) return;
            if (expected_span.has_value())
            {
                AssertSpanMatch(node->span, *expected_span);
            }
            if (expected_name_span.has_value())
            {
                if constexpr (requires { node->name.span; })
                {
                    AssertSpanMatch(node->name.span, *expected_name_span);
                }
                else if constexpr (requires { node->path.span; })
                {
                    AssertSpanMatch(node->path.span, *expected_name_span);
                }
                else if constexpr (requires { node->property_name.span; })
                {
                    AssertSpanMatch(node->property_name.span, *expected_name_span);
                }
            }
            matcher(node);
        }

        explicit operator bool() const
        {
            if constexpr (requires { static_cast<bool>(matcher); })
            {
                return static_cast<bool>(matcher);
            }
            return true;
        }
    };

    template <typename NodeT, size_t BufferSize>
    class InlineVerifier
    {
    private:
        struct VTable
        {
            void (*invoker)(const std::byte*, NodeT*);
            void (*copy_ctor)(std::byte*, const std::byte*);
            void (*move_ctor)(std::byte*, std::byte*);
            void (*dtor)(std::byte*);
        };

        alignas(std::max_align_t) std::byte buffer_[BufferSize];
        const VTable* vtable_ = nullptr;

        template <typename DecayedF>
        static const VTable* get_inline_vtable()
        {
            static constexpr VTable vt{
                [](const std::byte* buf, NodeT* node)
                {
                    (*reinterpret_cast<const DecayedF*>(buf))(node);
                },
                [](std::byte* dst, const std::byte* src)
                {
                    std::construct_at(reinterpret_cast<DecayedF*>(dst), *reinterpret_cast<const DecayedF*>(src));
                },
                [](std::byte* dst, std::byte* src)
                {
                    std::construct_at(reinterpret_cast<DecayedF*>(dst), std::move(*reinterpret_cast<DecayedF*>(src)));
                },
                [](std::byte* buf)
                {
                    std::destroy_at(reinterpret_cast<DecayedF*>(buf));
                }
            };
            return &vt;
        }

        template <typename DecayedF>
        static const VTable* get_heap_vtable()
        {
            using SharedPtr = std::shared_ptr<DecayedF>;
            static constexpr VTable vt{
                [](const std::byte* buf, NodeT* node)
                {
                    (*(*reinterpret_cast<const SharedPtr*>(buf)))(node);
                },
                [](std::byte* dst, const std::byte* src)
                {
                    std::construct_at(reinterpret_cast<SharedPtr*>(dst), *reinterpret_cast<const SharedPtr*>(src));
                },
                [](std::byte* dst, std::byte* src)
                {
                    std::construct_at(reinterpret_cast<SharedPtr*>(dst), std::move(*reinterpret_cast<SharedPtr*>(src)));
                },
                [](std::byte* buf)
                {
                    std::destroy_at(reinterpret_cast<SharedPtr*>(buf));
                }
            };
            return &vt;
        }

    public:
        InlineVerifier() = default;

        InlineVerifier(std::nullptr_t)
        {
        }

        template <typename F>
            requires (!std::same_as<std::decay_t<F>, InlineVerifier> &&
                !std::same_as<std::decay_t<F>, std::nullptr_t> &&
                IsCompatibleNodeVerifier<F, NodeT>)
        InlineVerifier(F&& f)
        {
            using DecayedF = std::decay_t<F>;
            if constexpr (sizeof(DecayedF) <= BufferSize && alignof(DecayedF) <= alignof(std::max_align_t))
            {
                std::construct_at(reinterpret_cast<DecayedF*>(buffer_), std::forward<F>(f));
                vtable_ = get_inline_vtable<DecayedF>();
            }
            else
            {
                using SharedPtr = std::shared_ptr<DecayedF>;
                auto ptr = std::make_shared<DecayedF>(std::forward<F>(f));
                std::construct_at(reinterpret_cast<SharedPtr*>(buffer_), std::move(ptr));
                vtable_ = get_heap_vtable<DecayedF>();
            }
        }

        ~InlineVerifier()
        {
            reset();
        }

        InlineVerifier(const InlineVerifier& other)
        {
            if (other.vtable_) [[likely]]
            {
                other.vtable_->copy_ctor(buffer_, other.buffer_);
                vtable_ = other.vtable_;
            }
        }

        InlineVerifier(InlineVerifier&& other) noexcept
        {
            if (other.vtable_) [[likely]]
            {
                other.vtable_->move_ctor(buffer_, other.buffer_);
                vtable_ = other.vtable_;
                other.vtable_ = nullptr;
            }
        }

        InlineVerifier& operator=(const InlineVerifier& other)
        {
            if (this != &other) [[likely]]
            {
                reset();
                if (other.vtable_) [[likely]]
                {
                    other.vtable_->copy_ctor(buffer_, other.buffer_);
                    vtable_ = other.vtable_;
                }
            }
            return *this;
        }

        InlineVerifier& operator=(InlineVerifier&& other) noexcept
        {
            if (this != &other) [[likely]]
            {
                reset();
                if (other.vtable_) [[likely]]
                {
                    other.vtable_->move_ctor(buffer_, other.buffer_);
                    vtable_ = other.vtable_;
                    other.vtable_ = nullptr;
                }
            }
            return *this;
        }

        void operator()(NodeT* node) const
        {
            if (vtable_) [[likely]] vtable_->invoker(buffer_, node);
        }

        [[nodiscard]] explicit operator bool() const { return vtable_ != nullptr; }

        [[nodiscard]] friend bool operator==(const InlineVerifier& v, std::nullptr_t) { return !static_cast<bool>(v); }
        [[nodiscard]] friend bool operator==(std::nullptr_t, const InlineVerifier& v) { return !static_cast<bool>(v); }
        [[nodiscard]] friend bool operator!=(const InlineVerifier& v, std::nullptr_t) { return static_cast<bool>(v); }
        [[nodiscard]] friend bool operator!=(std::nullptr_t, const InlineVerifier& v) { return static_cast<bool>(v); }

        void reset()
        {
            if (vtable_) [[likely]]
            {
                vtable_->dtor(buffer_);
                vtable_ = nullptr;
            }
        }
    };

    using ExprVerifier = InlineVerifier<Expression>;
    using TypeVerifier = InlineVerifier<TypeAnnotation>;
    using ImportVerifier = InlineVerifier<ImportStatement>;
    using DirectiveVerifier = InlineVerifier<Directive>;
    using FuncVerifier = InlineVerifier<FunctionDefinition>;
    using ExtVerifier = InlineVerifier<ExtensionDefinition>;
    using StructVerifier = InlineVerifier<StructDefinition>;
    using EnumVerifier = InlineVerifier<EnumDefinition>;
    using AliasVerifier = InlineVerifier<TypeAliasDefinition>;
    using AssignmentVerifier = InlineVerifier<Assignment>;
    using ReassignmentVerifier = InlineVerifier<Reassignment>;
    using ReturnVerifier = InlineVerifier<ReturnStatement>;
    using ExprStmtVerifier = InlineVerifier<ExpressionStatement>;

    template <typename T>
        requires std::derived_from<T, AstNode>
    T* ExpectNode(AstNode* node, std::source_location loc = std::source_location::current())
    {
        if (!node) [[unlikely]]
        {
            ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                << "Expected AST node of type [" << get_demangled_name(typeid(T).name())
                << "], but got [nullptr].";
            return nullptr;
        }
        T* casted = ast_cast<T>(node);
        if (!casted) [[unlikely]]
        {
            ADD_FAILURE_AT(loc.file_name(), static_cast<int>(loc.line()))
                << "Expected AST type [" << get_demangled_name(typeid(T).name())
                << "], but got [" << get_demangled_name(typeid(*node).name()) << "].";
            return nullptr;
        }
        return casted;
    }

    inline void ExpectNullNode(AstNode* node)
    {
        EXPECT_EQ(node, nullptr) << "Expected node to be null, but it was populated.";
    }

    struct NullVerifier
    {
        void operator()(AstNode* node) const { ExpectNullNode(node); }
        void operator()(TypeAnnotation* node) const { ExpectNullNode(node); }
    };

    struct AnyMatcher
    {
        void operator()(AstNode*) const
        {
        }

        void operator()(Expression*) const
        {
        }

        void operator()(Statement*) const
        {
        }

        void operator()(TypeAnnotation*) const
        {
        }

        explicit operator bool() const { return false; }
    };

    template <typename F>
    struct MatcherStorage
    {
        F verifier;
        bool has_value = true;

        MatcherStorage() : verifier{}, has_value(false)
        {
        }

        MatcherStorage(F v) : verifier(std::move(v)), has_value(true)
        {
        }

        MatcherStorage(std::nullptr_t) : verifier{}, has_value(false)
        {
        }

        template <typename NodeT>
        void operator()(NodeT* node) const
        {
            if (has_value) verifier(node);
        }

        explicit operator bool() const { return has_value; }
    };

    template <>
    struct MatcherStorage<AnyMatcher>
    {
        AnyMatcher verifier;
        bool has_value = false;

        MatcherStorage() : verifier{}, has_value(false)
        {
        }

        MatcherStorage(AnyMatcher v) : verifier(std::move(v)), has_value(false)
        {
        }

        MatcherStorage(std::nullptr_t) : verifier{}, has_value(false)
        {
        }

        template <typename NodeT>
        void operator()(NodeT*) const
        {
        }

        explicit operator bool() const { return false; }
    };

    inline NullVerifier IsNull() { return NullVerifier{}; }
    inline NullVerifier IsNullType() { return NullVerifier{}; }
}
