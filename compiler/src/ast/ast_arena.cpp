#include "ast_arena.h"

namespace valuascript::compiler
{
    static thread_local std::pmr::memory_resource* g_current_ast_resource = nullptr;

    std::pmr::memory_resource* AstArenaManager::get_current_resource() noexcept
    {
        if (!g_current_ast_resource)
        {
            g_current_ast_resource = std::pmr::get_default_resource();
        }
        return g_current_ast_resource;
    }

    void AstArenaManager::set_current_resource(std::pmr::memory_resource* res) noexcept
    {
        g_current_ast_resource = res ? res : std::pmr::get_default_resource();
    }
}
