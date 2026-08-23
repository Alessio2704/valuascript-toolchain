#include "ast_node.h"
#include "utils/memory/arena.h"

namespace valuascript::compiler
{
    void* AstNode::operator new(size_t size)
    {
        auto* res = valuascript::shared::Arena::current();
        return res->allocate(size, alignof(std::max_align_t));
    }

    void AstNode::operator delete(void* ptr, size_t size) noexcept
    {
        if (!ptr) return;
        auto* res = valuascript::shared::Arena::current();
        res->deallocate(ptr, size, alignof(std::max_align_t));
    }

    void AstNode::operator delete(void* ptr) noexcept
    {
        if (!ptr) return;
    }
}
