#include "ast_node.h"
#include "ast/arena/ast_arena.h"

namespace valuascript::compiler
{
    void* AstNode::operator new(size_t size)
    {
        auto* res = AstArena::current();
        return res->allocate(size, alignof(std::max_align_t));
    }

    void AstNode::operator delete(void* ptr, size_t size) noexcept
    {
        if (!ptr) return;
        auto* res = AstArena::current();
        res->deallocate(ptr, size, alignof(std::max_align_t));
    }

    void AstNode::operator delete(void* ptr) noexcept
    {
        if (!ptr) return;
    }
}
