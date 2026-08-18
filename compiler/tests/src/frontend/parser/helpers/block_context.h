#pragma once

namespace valuascript::compiler::test
{
    enum class BlockContext
    {
        None,
        TopLevel,
        FunctionBody,
        ExtensionBody
    };

    inline constexpr bool is_nested_block_context(BlockContext ctx)
    {
        return ctx != BlockContext::None && ctx != BlockContext::TopLevel;
    }
}
