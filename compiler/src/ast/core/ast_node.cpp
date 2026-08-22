#include "ast_node.h"
#include "ast/arena/ast_arena.h"

namespace valuascript::compiler
{
    void* AstNode::operator new(size_t size)
    {
        auto* res = AstArenaManager::get_current_resource();
        return res->allocate(size, alignof(std::max_align_t));
    }

    void AstNode::operator delete(void* ptr, size_t size) noexcept
    {
        if (!ptr) return;
        auto* res = AstArenaManager::get_current_resource();
        res->deallocate(ptr, size, alignof(std::max_align_t));
    }

    void AstNode::operator delete(void* ptr) noexcept
    {
        if (!ptr) return;
    }
}
