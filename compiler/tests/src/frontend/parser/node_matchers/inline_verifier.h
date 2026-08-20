#pragma once

#include <concepts>
#include <type_traits>
#include <memory>
#include <utility>
#include <cstddef>
#include "ast/ast.h"
#include "matcher_concepts.h"

namespace valuascript::compiler::test
{
    template <typename NodeT, size_t BufferSize = 64>
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
}
