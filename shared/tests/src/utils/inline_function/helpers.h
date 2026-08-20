#pragma once

#include <gtest/gtest.h>
#include "utils/inline_function.h"
#include <string>
#include <array>
#include <memory>
#include <vector>
#include <functional>

namespace valuascript::shared::test
{
    inline int FreeFunctionAdd(int a, int b)
    {
        return a + b;
    }

    inline std::string FreeFunctionConcat(const std::string& a, const std::string& b)
    {
        return a + b;
    }

    struct FunctorMultiply
    {
        int factor;
        int operator()(int x) const
        {
            return x * factor;
        }
    };

    struct LifecycleTracker
    {
        int* constructions = nullptr;
        int* copies = nullptr;
        int* moves = nullptr;
        int* destructions = nullptr;
        int value = 0;

        LifecycleTracker(int* c, int* cp, int* m, int* d, int val)
            : constructions(c), copies(cp), moves(m), destructions(d), value(val)
        {
            if (constructions) (*constructions)++;
        }

        LifecycleTracker(const LifecycleTracker& other)
            : constructions(other.constructions), copies(other.copies), moves(other.moves),
              destructions(other.destructions), value(other.value)
        {
            if (copies) (*copies)++;
        }

        LifecycleTracker(LifecycleTracker&& other) noexcept
            : constructions(other.constructions), copies(other.copies), moves(other.moves),
              destructions(other.destructions), value(other.value)
        {
            if (moves) (*moves)++;
            other.value = 0;
        }

        ~LifecycleTracker()
        {
            if (destructions) (*destructions)++;
        }

        LifecycleTracker& operator=(const LifecycleTracker&) = default;
        LifecycleTracker& operator=(LifecycleTracker&&) noexcept = default;

        int operator()(int x) const
        {
            return value + x;
        }
    };

    template <size_t Bytes>
    struct SizedCallable
    {
        std::array<std::byte, Bytes> payload{};
        int val = 42;

        int operator()() const
        {
            return val;
        }
    };

    struct TrackedLarge
    {
        std::array<std::byte, 128> payload{};
        int* dtor_count = nullptr;
        bool is_moved_from = false;

        TrackedLarge(int* d) : dtor_count(d) {}
        TrackedLarge(const TrackedLarge& o) : dtor_count(o.dtor_count), is_moved_from(o.is_moved_from) {}
        TrackedLarge(TrackedLarge&& o) noexcept : dtor_count(o.dtor_count), is_moved_from(false)
        {
            o.is_moved_from = true;
        }

        ~TrackedLarge()
        {
            if (dtor_count && !is_moved_from) (*dtor_count)++;
        }

        TrackedLarge& operator=(const TrackedLarge&) = default;
        TrackedLarge& operator=(TrackedLarge&& o) noexcept
        {
            dtor_count = o.dtor_count;
            is_moved_from = false;
            o.is_moved_from = true;
            return *this;
        }

        int operator()() const { return 1; }
    };
}
