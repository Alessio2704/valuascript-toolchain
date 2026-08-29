#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <memory_resource>
#include <atomic>
#include <cstdint>
#include <cmath>

#include "utils/memory/arena.h"

namespace valuascript::shared::test
{
    TEST(ArenaTest, ArenaBasicLifecycle)
    {
        Arena arena(64 * 1024);
        auto* resource = arena.resource();
        ASSERT_NE(resource, nullptr);

        void* ptr1 = resource->allocate(128, alignof(std::max_align_t));
        ASSERT_NE(ptr1, nullptr);

        void* ptr2 = resource->allocate(256, alignof(std::max_align_t));
        ASSERT_NE(ptr2, nullptr);
        EXPECT_NE(ptr1, ptr2);

        // Reset the arena and verify we can allocate again
        arena.reset();
        void* ptr3 = resource->allocate(128, alignof(std::max_align_t));
        ASSERT_NE(ptr3, nullptr);
    }

    TEST(ArenaTest, ArenaScopeRaiiSwitching)
    {
        auto* initial_resource = Arena::current();
        ASSERT_NE(initial_resource, nullptr);

        {
            ArenaScope scope(128 * 1024);
            auto* scope_resource = Arena::current();
            ASSERT_NE(scope_resource, nullptr);
            EXPECT_NE(scope_resource, initial_resource);
            EXPECT_EQ(scope_resource, scope.resource());
        }

        EXPECT_EQ(Arena::current(), initial_resource);
    }

    TEST(ArenaTest, ArenaScopeNested)
    {
        auto* base_resource = Arena::current();
        ASSERT_NE(base_resource, nullptr);

        {
            ArenaScope outer_scope(64 * 1024);
            auto* outer_resource = Arena::current();
            ASSERT_NE(outer_resource, nullptr);
            EXPECT_NE(outer_resource, base_resource);

            {
                ArenaScope middle_scope(64 * 1024);
                auto* middle_resource = Arena::current();
                ASSERT_NE(middle_resource, nullptr);
                EXPECT_NE(middle_resource, outer_resource);
                EXPECT_NE(middle_resource, base_resource);

                {
                    ArenaScope inner_scope(64 * 1024);
                    auto* inner_resource = Arena::current();
                    ASSERT_NE(inner_resource, nullptr);
                    EXPECT_NE(inner_resource, middle_resource);
                    EXPECT_NE(inner_resource, outer_resource);
                    EXPECT_NE(inner_resource, base_resource);
                }

                EXPECT_EQ(Arena::current(), middle_resource);
            }

            EXPECT_EQ(Arena::current(), outer_resource);
        }

        EXPECT_EQ(Arena::current(), base_resource);
    }

    TEST(ArenaTest, ArenaScopeManualReset)
    {
        ArenaScope scope(64 * 1024);
        auto* resource = Arena::current();
        ASSERT_NE(resource, nullptr);

        void* ptr1 = resource->allocate(128, alignof(std::max_align_t));
        ASSERT_NE(ptr1, nullptr);

        scope.reset();

        void* ptr2 = resource->allocate(128, alignof(std::max_align_t));
        ASSERT_NE(ptr2, nullptr);
    }

    TEST(ArenaTest, ArenaThreadIsolation)
    {
        constexpr int NUM_THREADS = 4;
        constexpr int ITERATIONS = 100;
        std::atomic<bool> failed{false};
        std::vector<std::thread> threads;
        threads.reserve(NUM_THREADS);

        for (int t = 0; t < NUM_THREADS; ++t)
        {
            threads.emplace_back([&failed]()
            {
                auto* thread_initial = Arena::current();
                if (!thread_initial)
                {
                    failed = true;
                    return;
                }

                for (int i = 0; i < ITERATIONS; ++i)
                {
                    ArenaScope scope(32 * 1024);
                    auto* scope_res = Arena::current();
                    if (!scope_res || scope_res == thread_initial)
                    {
                        failed = true;
                        return;
                    }

                    void* mem = scope_res->allocate(64, alignof(std::max_align_t));
                    if (!mem)
                    {
                        failed = true;
                        return;
                    }
                }

                if (Arena::current() != thread_initial)
                {
                    failed = true;
                }
            });
        }

        for (auto& th : threads)
        {
            th.join();
        }

        EXPECT_FALSE(failed.load());
    }

    TEST(ArenaTest, ArenaVsHeapAllocationBehavior)
    {
        {
            Arena arena(64 * 1024);
            auto* arena_res = arena.resource();

            constexpr size_t BLOCK_SIZE = 64;
            constexpr size_t COUNT = 4;
            std::vector<uint8_t*> arena_ptrs;

            for (size_t i = 0; i < COUNT; ++i)
            {
                auto* p = static_cast<uint8_t*>(arena_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));
                ASSERT_NE(p, nullptr);
                arena_ptrs.push_back(p);
            }

            for (size_t i = 0; i < COUNT - 1; ++i)
            {
                auto diff = std::abs(
                    reinterpret_cast<intptr_t>(arena_ptrs[i + 1]) - reinterpret_cast<intptr_t>(arena_ptrs[i]));
                EXPECT_EQ(static_cast<size_t>(diff), BLOCK_SIZE);
            }
        }

        {
            constexpr size_t BLOCK_SIZE = 64;
            auto* heap_res = std::pmr::get_default_resource();

            Arena arena(64 * 1024);
            auto* arena_res = arena.resource();

            auto* a0 = static_cast<uint8_t*>(arena_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));
            auto* a1 = static_cast<uint8_t*>(arena_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));
            auto* a2 = static_cast<uint8_t*>(arena_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));

            auto* h0 = static_cast<uint8_t*>(heap_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));
            auto* h1 = static_cast<uint8_t*>(heap_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));
            auto* h2 = static_cast<uint8_t*>(heap_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));

            ASSERT_NE(a0, nullptr);
            ASSERT_NE(a1, nullptr);
            ASSERT_NE(a2, nullptr);
            ASSERT_NE(h0, nullptr);
            ASSERT_NE(h1, nullptr);
            ASSERT_NE(h2, nullptr);

            heap_res->deallocate(h1, BLOCK_SIZE, alignof(std::max_align_t));
            arena_res->deallocate(a1, BLOCK_SIZE, alignof(std::max_align_t));

            auto* a_new = static_cast<uint8_t*>(arena_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));
            auto* h_new = static_cast<uint8_t*>(heap_res->allocate(BLOCK_SIZE, alignof(std::max_align_t)));

            ASSERT_NE(a_new, nullptr);
            ASSERT_NE(h_new, nullptr);

            // Arena invariant: Deallocation is a no-op; bump pointer continues in monotonic direction
            EXPECT_NE(a_new, a1);
            auto arena_step = std::abs(reinterpret_cast<intptr_t>(a_new) - reinterpret_cast<intptr_t>(a2));
            EXPECT_EQ(static_cast<size_t>(arena_step), BLOCK_SIZE);

            // Heap invariant: Memory is valid and writable
            *h_new = 0xAA;
            EXPECT_EQ(*h_new, 0xAA);

            // Cleanup heap blocks
            heap_res->deallocate(h0, BLOCK_SIZE, alignof(std::max_align_t));
            heap_res->deallocate(h_new, BLOCK_SIZE, alignof(std::max_align_t));
            heap_res->deallocate(h2, BLOCK_SIZE, alignof(std::max_align_t));
        }
    }
}
