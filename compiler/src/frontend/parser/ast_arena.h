#pragma once

#include <memory_resource>
#include <cstddef>

namespace valuascript::compiler
{
    class AstArena
    {
    private:
        std::pmr::monotonic_buffer_resource resource_;

    public:
        explicit AstArena(size_t initial_size = 256 * 1024)
            : resource_(initial_size, std::pmr::get_default_resource()) {}

        [[nodiscard]] std::pmr::memory_resource* resource() noexcept
        {
            return &resource_;
        }

        void reset() noexcept
        {
            resource_.release();
        }
    };

    class AstArenaManager
    {
    public:
        static std::pmr::memory_resource* get_current_resource() noexcept;
        static void set_current_resource(std::pmr::memory_resource* res) noexcept;
    };

    class AstArenaScope
    {
    private:
        AstArena arena_;
        std::pmr::memory_resource* prev_resource_;

    public:
        explicit AstArenaScope(size_t initial_size = 256 * 1024)
            : arena_(initial_size), prev_resource_(AstArenaManager::get_current_resource())
        {
            AstArenaManager::set_current_resource(arena_.resource());
        }

        ~AstArenaScope()
        {
            AstArenaManager::set_current_resource(prev_resource_);
        }

        void reset() noexcept
        {
            arena_.reset();
        }
    };
}
