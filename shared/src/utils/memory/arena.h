#pragma once

#include <memory_resource>
#include <cstddef>

namespace valuascript::shared
{
    class Arena
    {
    private:
        std::pmr::monotonic_buffer_resource resource_;

    public:
        explicit Arena(size_t initial_size = 256 * 1024)
            : resource_(initial_size, std::pmr::get_default_resource()) {}

        [[nodiscard]] std::pmr::memory_resource* resource() noexcept
        {
            return &resource_;
        }

        void reset() noexcept
        {
            resource_.release();
        }

        [[nodiscard]] static std::pmr::memory_resource* current() noexcept;
        static void set_current(std::pmr::memory_resource* res) noexcept;
    };

    class ArenaScope
    {
    private:
        Arena arena_;
        std::pmr::memory_resource* prev_resource_;

    public:
        explicit ArenaScope(size_t initial_size = 256 * 1024)
            : arena_(initial_size), prev_resource_(Arena::current())
        {
            Arena::set_current(arena_.resource());
        }

        ~ArenaScope()
        {
            Arena::set_current(prev_resource_);
        }

        void reset() noexcept
        {
            arena_.reset();
        }

        [[nodiscard]] std::pmr::memory_resource* resource() noexcept
        {
            return arena_.resource();
        }
    };
}
