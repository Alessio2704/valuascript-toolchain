#include "expansion_policy.h"
#include "utils/test_env_config.h"

namespace valuascript::compiler::test
{
    ExpansionPolicy ExpansionPolicy::current()
    {
        static ExpansionPolicy cached = []()
        {
            constexpr int depth = EXPANSION_DEPTH;
            constexpr int recursion = EXPANSION_RECURSION;

            return ExpansionPolicy{depth, recursion};
        }();
        return cached;
    }
}
