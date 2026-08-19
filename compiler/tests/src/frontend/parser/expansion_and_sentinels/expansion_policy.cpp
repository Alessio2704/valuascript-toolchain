#include "expansion_policy.h"

namespace valuascript::compiler::test
{
    ExpansionPolicy ExpansionPolicy::current()
    {
        static ExpansionPolicy cached = []()
        {
            constexpr int depth = EXPANSION_DEPTH;
            constexpr int recursion = EXPANSION_RECURSION;

            return ExpansionPolicy{.max_depth = depth, .max_recursion = recursion};
        }();
        return cached;
    }
}
