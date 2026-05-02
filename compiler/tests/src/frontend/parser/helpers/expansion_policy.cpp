#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "expansion_policy.h"
#include <algorithm>
#include <cstdlib>
#include <string>

namespace valuascript::compiler::test
{
    ExpansionPolicy ExpansionPolicy::current()
    {
        static ExpansionPolicy cached = []()
        {
            int depth = 5;
            int recursion = 1;

            if (const char* env_d = std::getenv("EXPANSION_DEPTH")) depth = std::stoi(env_d);
            if (const char* env_r = std::getenv("EXPANSION_RECURSION")) recursion = std::stoi(env_r);

            return ExpansionPolicy{
                std::clamp(depth, 1, 10),
                std::clamp(recursion, 1, 10)
            };
        }();
        return cached;
    }
}
