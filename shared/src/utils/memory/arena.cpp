#include "utils/memory/arena.h"

namespace valuascript::shared
{
    static thread_local std::pmr::memory_resource* g_current_arena_resource = nullptr;

    std::pmr::memory_resource* Arena::current() noexcept
    {
        if (!g_current_arena_resource)
        {
            g_current_arena_resource = std::pmr::get_default_resource();
        }
        return g_current_arena_resource;
    }

    void Arena::set_current(std::pmr::memory_resource* res) noexcept
    {
        g_current_arena_resource = res ? res : std::pmr::get_default_resource();
    }
}
