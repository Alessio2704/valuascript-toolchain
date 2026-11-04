#pragma once

#include <concepts>
#include <type_traits>
#include <memory>
#include <utility>
#include <cstddef>

namespace valuascript::shared
{
    template <typename Signature, size_t BufferSize = 64>
    class InlineFunction;

    template <typename ReturnType, typename... Args, size_t BufferSize>
    class InlineFunction<ReturnType(Args...), BufferSize>
    {
    private:
        struct VTable
        {
            ReturnType (*invoker)(const std::byte*, Args...);
            void (*copy_ctor)(std::byte*, const std::byte*);
            void (*move_ctor)(std::byte*, std::byte*);
            void (*dtor)(std::byte*);
        };

        alignas(std::max_align_t) std::byte buffer_[BufferSize];
        const VTable* vtable_ = nullptr;

        template <typename DecayedF>
        static const VTable* get_inline_vtable() noexcept
        {
            static constexpr VTable vt{
                [](const std::byte* buf, Args... args) -> ReturnType
                {
                    auto* fn = const_cast<DecayedF*>(reinterpret_cast<const DecayedF*>(buf));
                    if constexpr (std::is_void_v<ReturnType>)
                    {
                        (*fn)(std::forward<Args>(args)...);
                    }
                    else
                    {
                        return (*fn)(std::forward<Args>(args)...);
                    }
                },
                [](std::byte* dst, const std::byte* src)
                {
                    std::construct_at(reinterpret_cast<DecayedF*>(dst), *reinterpret_cast<const DecayedF*>(src));
                },
                [](std::byte* dst, std::byte* src)
                {
                    std::construct_at(reinterpret_cast<DecayedF*>(dst), std::move(*reinterpret_cast<DecayedF*>(src)));
                    std::destroy_at(reinterpret_cast<DecayedF*>(src));
                },
                [](std::byte* buf)
                {
                    std::destroy_at(reinterpret_cast<DecayedF*>(buf));
                }
            };
            return &vt;
        }

        template <typename DecayedF>
        static const VTable* get_heap_vtable() noexcept
        {
            using SharedPtr = std::shared_ptr<DecayedF>;
            static constexpr VTable vt{
                [](const std::byte* buf, Args... args) -> ReturnType
                {
                    auto* ptr = const_cast<SharedPtr*>(reinterpret_cast<const SharedPtr*>(buf));
                    if constexpr (std::is_void_v<ReturnType>)
                    {
                        (*(*ptr))(std::forward<Args>(args)...);
                    }
                    else
                    {
                        return (*(*ptr))(std::forward<Args>(args)...);
                    }
                },
                [](std::byte* dst, const std::byte* src)
                {
                    std::construct_at(reinterpret_cast<SharedPtr*>(dst), *reinterpret_cast<const SharedPtr*>(src));
                },
                [](std::byte* dst, std::byte* src)
                {
                    std::construct_at(reinterpret_cast<SharedPtr*>(dst), std::move(*reinterpret_cast<SharedPtr*>(src)));
                    std::destroy_at(reinterpret_cast<SharedPtr*>(src));
                },
                [](std::byte* buf)
                {
                    std::destroy_at(reinterpret_cast<SharedPtr*>(buf));
                }
            };
            return &vt;
        }

    public:
        constexpr InlineFunction() noexcept = default;

        constexpr InlineFunction(std::nullptr_t) noexcept
        {
        }

        template <typename F>
            requires (!std::same_as<std::decay_t<F>, InlineFunction> &&
                      !std::same_as<std::decay_t<F>, std::nullptr_t> &&
                      std::is_invocable_r_v<ReturnType, F, Args...>)
        InlineFunction(F&& f)
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

        ~InlineFunction()
        {
            reset();
        }

        InlineFunction(const InlineFunction& other)
        {
            if (other.vtable_) [[likely]]
            {
                other.vtable_->copy_ctor(buffer_, other.buffer_);
                vtable_ = other.vtable_;
            }
        }

        InlineFunction(InlineFunction&& other) noexcept
        {
            if (other.vtable_) [[likely]]
            {
                other.vtable_->move_ctor(buffer_, other.buffer_);
                vtable_ = other.vtable_;
                other.vtable_ = nullptr;
            }
        }

        InlineFunction& operator=(const InlineFunction& other)
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

        InlineFunction& operator=(InlineFunction&& other) noexcept
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

        InlineFunction& operator=(std::nullptr_t) noexcept
        {
            reset();
            return *this;
        }

        ReturnType operator()(Args... args) const
        {
            if (vtable_) [[likely]]
            {
                return vtable_->invoker(buffer_, std::forward<Args>(args)...);
            }
            if constexpr (!std::is_void_v<ReturnType>)
            {
                return ReturnType{};
            }
        }

        [[nodiscard]] explicit operator bool() const noexcept { return vtable_ != nullptr; }

        [[nodiscard]] friend bool operator==(const InlineFunction& f, std::nullptr_t) noexcept { return !static_cast<bool>(f); }
        [[nodiscard]] friend bool operator==(std::nullptr_t, const InlineFunction& f) noexcept { return !static_cast<bool>(f); }
        [[nodiscard]] friend bool operator!=(const InlineFunction& f, std::nullptr_t) noexcept { return static_cast<bool>(f); }
        [[nodiscard]] friend bool operator!=(std::nullptr_t, const InlineFunction& f) noexcept { return static_cast<bool>(f); }

        void reset() noexcept
        {
            if (vtable_) [[likely]]
            {
                vtable_->dtor(buffer_);
                vtable_ = nullptr;
            }
        }
    };
}
