#pragma once

#include <memory>
#include <optional>
#include <utility>

namespace valuascript::compiler
{
    template <typename T>
    struct OptionalAstField;

    template <typename NodeT>
    struct OptionalAstField<std::unique_ptr<NodeT>>
    {
        std::unique_ptr<NodeT> ptr = nullptr;

        constexpr OptionalAstField() noexcept = default;

        constexpr OptionalAstField(std::nullopt_t) noexcept : ptr(nullptr)
        {
        }

        OptionalAstField(std::unique_ptr<NodeT> p) noexcept : ptr(std::move(p))
        {
        }

        OptionalAstField(std::nullptr_t) noexcept : ptr(nullptr)
        {
        }

        template <typename DerivedNodeT>
            requires (std::is_convertible_v<DerivedNodeT*, NodeT*> && !std::is_same_v<DerivedNodeT, NodeT>)
        OptionalAstField(std::unique_ptr<DerivedNodeT> p) noexcept : ptr(std::move(p))
        {
        }

        OptionalAstField(const OptionalAstField&) = delete;
        OptionalAstField& operator=(const OptionalAstField&) = delete;

        OptionalAstField(OptionalAstField&&) noexcept = default;
        OptionalAstField& operator=(OptionalAstField&&) noexcept = default;

        OptionalAstField& operator=(std::nullopt_t) noexcept
        {
            ptr.reset();
            return *this;
        }

        OptionalAstField& operator=(std::nullptr_t) noexcept
        {
            ptr.reset();
            return *this;
        }

        OptionalAstField& operator=(std::unique_ptr<NodeT> p) noexcept
        {
            ptr = std::move(p);
            return *this;
        }

        template <typename DerivedNodeT>
            requires (std::is_convertible_v<DerivedNodeT*, NodeT*> && !std::is_same_v<DerivedNodeT, NodeT>)
        OptionalAstField& operator=(std::unique_ptr<DerivedNodeT> p) noexcept
        {
            ptr = std::move(p);
            return *this;
        }

        operator std::unique_ptr<NodeT>() && noexcept
        {
            return std::move(ptr);
        }

        [[nodiscard]] bool has_value() const noexcept { return ptr != nullptr; }
        explicit operator bool() const noexcept { return ptr != nullptr; }

        [[nodiscard]] NodeT* get() const noexcept { return ptr.get(); }
        [[nodiscard]] NodeT& operator*() const noexcept { return *ptr; }
        [[nodiscard]] NodeT* operator->() const noexcept { return ptr.get(); }

        [[nodiscard]] std::unique_ptr<NodeT>& value() & noexcept { return ptr; }
        [[nodiscard]] const std::unique_ptr<NodeT>& value() const & noexcept { return ptr; }
        [[nodiscard]] std::unique_ptr<NodeT>&& value() && noexcept { return std::move(ptr); }

        [[nodiscard]] std::unique_ptr<NodeT> release() noexcept { return std::move(ptr); }
        void reset() noexcept { ptr.reset(); }

        bool operator==(std::nullopt_t) const noexcept { return ptr == nullptr; }
        bool operator==(std::nullptr_t) const noexcept { return ptr == nullptr; }
    };

    template <typename ValT>
    struct OptionalAstField
    {
        std::optional<ValT> opt = std::nullopt;

        constexpr OptionalAstField() noexcept = default;

        constexpr OptionalAstField(std::nullopt_t) noexcept : opt(std::nullopt)
        {
        }

        OptionalAstField(ValT val) : opt(std::move(val))
        {
        }

        OptionalAstField(std::optional<ValT> o) : opt(std::move(o))
        {
        }

        OptionalAstField(const OptionalAstField&) = default;
        OptionalAstField& operator=(const OptionalAstField&) = default;

        OptionalAstField(OptionalAstField&&) noexcept = default;
        OptionalAstField& operator=(OptionalAstField&&) noexcept = default;

        OptionalAstField& operator=(std::nullopt_t) noexcept
        {
            opt.reset();
            return *this;
        }

        OptionalAstField& operator=(ValT val)
        {
            opt = std::move(val);
            return *this;
        }

        OptionalAstField& operator=(std::optional<ValT> o)
        {
            opt = std::move(o);
            return *this;
        }

        [[nodiscard]] bool has_value() const noexcept { return opt.has_value(); }
        explicit operator bool() const noexcept { return opt.has_value(); }

        [[nodiscard]] ValT& operator*() & noexcept { return *opt; }
        [[nodiscard]] const ValT& operator*() const & noexcept { return *opt; }
        [[nodiscard]] ValT* operator->() noexcept { return opt.operator->(); }
        [[nodiscard]] const ValT* operator->() const noexcept { return opt.operator->(); }

        [[nodiscard]] ValT& value() & { return opt.value(); }
        [[nodiscard]] const ValT& value() const & { return opt.value(); }
        [[nodiscard]] ValT&& value() && { return std::move(opt).value(); }

        void reset() noexcept { opt.reset(); }

        friend bool operator==(const OptionalAstField& lhs, const OptionalAstField& rhs) noexcept = default;
        friend bool operator==(const OptionalAstField& lhs, const ValT& rhs) noexcept { return lhs.opt == rhs; }

        friend bool operator==(const OptionalAstField& lhs, const std::optional<ValT>& rhs) noexcept
        {
            return lhs.opt == rhs;
        }

        friend bool operator==(const OptionalAstField& lhs, std::nullopt_t) noexcept { return !lhs.opt.has_value(); }
    };
}
