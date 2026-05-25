#pragma once

#include "frontend/parser/helpers/context_infrastructure.h"

namespace valuascript::compiler::test
{
    class ExpansionCalculator
    {
    public:
        static size_t compute_expected_expansions(InjectableType start_type);
    };
}
