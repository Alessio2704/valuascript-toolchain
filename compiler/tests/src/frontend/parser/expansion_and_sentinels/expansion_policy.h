#pragma once

namespace valuascript::compiler::test
{
    struct ExpansionPolicy
    {
        int max_depth;
        int max_recursion;

        static ExpansionPolicy current();
    };
}
